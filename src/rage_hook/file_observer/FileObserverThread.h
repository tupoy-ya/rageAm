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

		explicit FileStoreEntry(std::string name, std::filesystem::file_time_type writeTime)
		{
			Name = name;
			WriteTime = writeTime;
			Exists = true;
			Error = false;
		}

		virtual ~FileStoreEntry() = default;
		virtual HRESULT Load(const std::wstring& path) = 0;
		virtual void Release() = 0;
	};

	struct TextureSwapStoreEntry : FileStoreEntry
	{
		ID3D11Resource* pResource;
		ID3D11ShaderResourceView* pResourceView;

		using FileStoreEntry::FileStoreEntry;

		HRESULT Load(const std::wstring& path) override
		{
			ID3D11Device* pDevice = rh::D3D::GetDevice();

			return DirectX::CreateDDSTextureFromFile(
				pDevice, path.c_str(), &pResource, &pResourceView);
		}

		void Release() override
		{
			pResource->Release();
			pResourceView->Release();
		}
	};

	/**
	 * \brief Stores and dynamically manages file resources.
	 * This is base class for shader / texture swap.
	 * \tparam TEntry Data format to store derived from FileStoreEntry.
	 */
	template<typename TEntry>
	class FileObserverThread
	{
		HANDLE m_ObserveThread;
		std::unordered_map<std::string, std::unique_ptr<FileStoreEntry>> m_FileStore;
		const wchar_t* m_Path;
		bool m_IncludeDirInKey;

		static constexpr int OBSERVER_DELAY_MS = 200;

		static DWORD ObserverThreadEntryPoint(LPVOID lpParam)
		{
			g_Log.LogT("Swap Thread -> Start");

			FileObserverThread* inst = (FileObserverThread*)lpParam;

			const wchar_t* path = inst->m_Path;
			while (true)
			{
				// Scan through every global texture, read it and add in store
				// if it's not loaded yet or write date was changed
				for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
				{
					if (entry.is_directory())
						continue;

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

					bool existsAndValid = inst->m_FileStore.contains(fileNameKey);
					if (existsAndValid)
					{
						auto& slot = inst->m_FileStore.at(fileNameKey);

						slot->Exists = true;

						// TODO: Check if file still being written
						if (slot->WriteTime != lastWriteTime)
						{
							g_Log.LogT("Observer Thread -> File {} changed.", fileNameKey);

							inst->m_Mutex.lock();
							if (!slot->Error)
							{
								slot->Release();
							}
							inst->m_FileStore.erase(fileNameKey);
							inst->m_Mutex.unlock();

							existsAndValid = false;
						}
					}

					if (!existsAndValid)
					{
						g_Log.LogT("Observer Thread -> Reading: {}", fileNameKey);

						inst->m_Mutex.lock();
						inst->m_FileStore.emplace(fileNameKey, std::make_unique<TEntry>(fileName, lastWriteTime));
						inst->m_Mutex.unlock();

						// Is there better way to do it with unique ptr?
						auto& slot = inst->m_FileStore.at(fileNameKey);
						HRESULT result = slot->Load(filePath);

						if (result != S_OK)
						{
							slot->Error = true;
							g_Log.LogE("Observer Thread -> Unable to load file: {}", fileNameKey);
						}
						else
						{
							inst->OnEntryUpdated(dir, fileName, &slot);
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
		std::shared_mutex m_Mutex;

		virtual void OnEntryUpdated(std::string dir, std::string name, std::unique_ptr<FileStoreEntry>* entry) const {};
	public:
		FileObserverThread(const wchar_t* path, bool includeDirInKey)
		{
			// Create directory if it doesn't exists
			CreateDirectoryW(path, nullptr);

			m_Path = path;
			m_IncludeDirInKey = includeDirInKey;
			m_ObserveThread = CreateThread(
				nullptr, 0, ObserverThreadEntryPoint, (LPVOID)this, 0, nullptr);
		}

		virtual ~FileObserverThread()
		{
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

	class TextureSwapThread : public FileObserverThread<TextureSwapStoreEntry>
	{
	public:
		static inline bool IsGlobalSwapOn;

		using FileObserverThread<TextureSwapStoreEntry>::FileObserverThread;

		void GetTextureSwap(const char* name, ID3D11ShaderResourceView** shaderResourceView)
		{
			if (!IsGlobalSwapOn)
				return;

			TextureSwapStoreEntry* entry = Find(name);
			if (entry == nullptr)
				return;

			*shaderResourceView = entry->pResourceView;
		}
	};

	inline TextureSwapThread g_TextureSwapThread{ L"rageAm/Textures/global", false };
}
