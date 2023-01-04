#pragma once
#include <unordered_map>

namespace fiobs
{
	struct FileStoreEntry
	{
		/**
		 * \brief File name without extension.
		 */
		std::string Name;

		/**
		 * \brief Last time file was modified. Used to detect if file has to be reloaded.
		 */
		std::filesystem::file_time_type WriteTime;

		/**
		 * \brief Flag to check if file is still exists after iterating directory
		 */
		bool Exists;

		/**
		 * \brief Flag to check if there was error loading file to stop loading it over and over again.
		 */
		bool Error;

		/**
		 * \brief Joaat hash of parent directory name.
		 */
		u32 ParentHash;

		FileStoreEntry(std::string name, std::filesystem::file_time_type writeTime, u32 parentHash)
		{
			Name = name;
			WriteTime = writeTime;
			ParentHash = parentHash;
			Exists = true;
			Error = false;
		}

		virtual ~FileStoreEntry() = default;
		virtual HRESULT Load(const std::wstring& path) = 0;
		virtual void Release()
		{
			RequestReload(); // Simply reset time
		}
		void RequestReload()
		{
			WriteTime = std::filesystem::file_time_type{};
		}
	};

	/**
	 * \brief Stores and dynamically manages file resources.
	 * This is base class for shader / texture swap.
	 * \tparam TEntry Data format to store derived from FileStoreEntry.
	 */
	template<typename TEntry>
	class FileObserverThreadInterface
	{
		HANDLE m_ObserveThread;
		const wchar_t* m_Path;
		bool m_IncludeDirInKey;
		bool m_IsShutdown = false;

		static constexpr int OBSERVER_DELAY_MS = 200;

		static DWORD ObserverThreadEntryPoint(LPVOID lpParam)
		{
			g_Log.LogT("Swap Thread -> Start");

			FileObserverThreadInterface* inst = (FileObserverThreadInterface*)lpParam;

			const wchar_t* path = inst->m_Path;
			while (true)
			{
				// Scan through every global texture, read it and add in store
				// if it's not loaded yet or write date was changed
				for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
				{
					if (entry.is_directory())
						continue;

					// Is this a place for it?
					if (g_CrashHandler.GetExceptionOccured())
					{
						inst->Shutdown();
						return 0;
					}

					std::filesystem::file_time_type lastWriteTime;
					_Last_write_time(entry, lastWriteTime);

					// Full path
					std::wstring filePath = entry.path().wstring();

					// Full path without file name
					std::string dir = entry.path().parent_path().filename().string();

					// Just file name
					std::string fileName = entry.path().stem().string();

					// Either file name or file name with dir, used as hash key
					std::string fileNameKey;
					if (inst->m_IncludeDirInKey)
					{
						fileNameKey = dir;
						fileNameKey.append("\\");
						fileNameKey += fileName;
					}
					else
					{
						fileNameKey = fileName;
					}

					auto slotIt = inst->m_FileStore.find(fileNameKey);
					bool exists = slotIt != inst->m_FileStore.end();
					bool valid = true;
					FileStoreEntry* slot;
					if (exists) // Reset state
					{
						slot = slotIt->second.get();
						slot->Exists = true;
						if (slot->WriteTime != lastWriteTime)
						{
							g_Log.LogT("Observer Thread -> File {} changed.", fileNameKey);

							inst->m_Mutex.lock();
							if (!slot->Error)
								slot->Release();
							inst->m_Mutex.unlock();

							valid = false;
						}
					}

					if (!exists || !valid)
					{
						g_Log.LogT("Observer Thread -> Reading: {}", fileNameKey);

						inst->m_Mutex.lock();
						if (exists) // Update existing entry
						{
							slot->WriteTime = lastWriteTime;
						}
						else
						{
							u32 patentHash = fwHelpers::joaat(dir.c_str());
							slot = inst->m_FileStore.emplace(fileNameKey, std::make_unique<TEntry>(fileName, lastWriteTime, patentHash))
								.first->second.get();
						}
						inst->m_Mutex.unlock();

						HRESULT result = slot->Load(filePath);

						if (result != S_OK)
						{
							slot->Error = true;
							g_Log.LogE("Observer Thread -> Unable to load file: {}", fileNameKey);
						}
						else
						{
							inst->OnEntryUpdated(dir, fileName, slot);
						}
					}
				}

				// Find all removed textures from global directory
				std::vector<std::string> toRemove;
				for (auto& entry : inst->m_FileStore)
				{
					if (!entry.second->Exists)
					{
						entry.second.release();
						toRemove.push_back(entry.first);
						continue;
					}

					// If entry wouldn't be in folder next loop, flag wont be set to
					// true and we will remove it
					entry.second->Exists = false;
				}

				// Remove all removed textures from global directory
				inst->m_Mutex.lock();
				for (auto& entry : toRemove)
				{
					inst->m_FileStore.erase(entry);
				}
				inst->m_Mutex.unlock();

				std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_DELAY_MS));
			}
		}
	protected:
		std::unordered_map<std::string, std::unique_ptr<FileStoreEntry>> m_FileStore;
		std::shared_mutex m_Mutex;

		virtual void OnEntryUpdated(std::string dir, std::string name, FileStoreEntry* entry) const {};
	public:
		FileObserverThreadInterface(const wchar_t* path, bool includeDirInKey)
		{
			CreateDefaultFolders();

			// Create directory if it doesn't exists
			CreateDirectoryW(path, nullptr);

			m_Path = path;
			m_IncludeDirInKey = includeDirInKey;
			m_ObserveThread = CreateThread(
				nullptr, 0, ObserverThreadEntryPoint, (LPVOID)this, 0, nullptr);
		}

		virtual ~FileObserverThreadInterface()
		{
			Shutdown();
		}

		void Shutdown()
		{
			if (m_IsShutdown)
				return;

			m_IsShutdown = true;

			// Terminate thread because joining it causes game deadlock for some reason
			TerminateThread(m_ObserveThread, 0);
			CloseHandle(m_ObserveThread);

			g_Log.LogT("SwapThread -> Exited");

			m_Mutex.lock();
			for (auto& val : m_FileStore)
			{
				g_Log.LogT("SwapThread -> Releasing File: {}", val.first);
				val.second.release();
			}
			m_FileStore.clear();
			m_Mutex.unlock();
		}

		TEntry* Find(const char* name)
		{
			TEntry* result = nullptr;

			m_Mutex.lock_shared();
			auto it = m_FileStore.find(name);
			if (it != m_FileStore.end())
			{
				// Don't give files with that wasn't loaded properly
				if (!it->second->Error)
				{
					result = (TEntry*)it->second.get();
				}
			}
			m_Mutex.unlock_shared();

			return result;
		}
	};
}
