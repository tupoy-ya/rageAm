#pragma once
#include <unordered_map>

namespace rh
{
	/**
	 * \brief Used for swapping game textures on runtime.
	 */
	class TextureSwapThread
	{
		struct SwapStore
		{
			std::filesystem::file_time_type WriteTime;
			ID3D11Resource* pResource;
			ID3D11ShaderResourceView* pResourceView;

			// Flag to check if file is still exists after iterating directory
			bool Exists;
		};

		static inline HANDLE m_SwapThread;
		static inline std::shared_mutex m_GlobalTexturesMutex;
		static inline std::unordered_map<std::string, SwapStore> m_GlobalTextureStore;

		static constexpr int SWAP_TEXTURE_UPDATE_DELAY_MS = 200;

		static DWORD TextureSwapThreadEntryPoint(LPVOID lpParam)
		{
			g_Log.LogT("Swap Thread -> Start");

			std::string path = "rageAm/Textures/global";
			while (true)
			{
				// Scan through every global texture, read it and add in store
				// if it's not loaded yet or write date was changed
				for (const auto& entry : std::filesystem::directory_iterator(path))
				{
					std::filesystem::file_time_type lastWriteTime;
					_Last_write_time(entry, lastWriteTime);

					std::string fileName = entry.path().stem().string();
					std::wstring filePath = entry.path().wstring();

					bool existsAndValid = m_GlobalTextureStore.contains(fileName);
					if (existsAndValid)
					{
						// TODO: Check if file still being written
						if (m_GlobalTextureStore.at(fileName).WriteTime != lastWriteTime)
						{
							g_Log.LogT("Swap Thread -> File {} changed.", fileName);

							m_GlobalTexturesMutex.lock();

							SwapStore& swap = m_GlobalTextureStore.at(fileName);
							swap.pResource->Release();
							swap.pResourceView->Release();
							m_GlobalTextureStore.erase(fileName);

							m_GlobalTexturesMutex.unlock();

							existsAndValid = false;
						}
					}

					if (!existsAndValid)
					{
						g_Log.LogT("Swap Thread -> Reading: {}", fileName);

						ID3D11Device* pDevice = grcDX11::GetDevice();
						ID3D11Resource* pResource;
						ID3D11ShaderResourceView* pResourceView;

						HRESULT result = DirectX::CreateDDSTextureFromFile(
							pDevice, filePath.c_str(), &pResource, &pResourceView);

						if (result == S_OK)
						{
							m_GlobalTexturesMutex.lock();
							m_GlobalTextureStore.emplace(fileName,
								SwapStore(lastWriteTime, pResource, pResourceView));
							m_GlobalTexturesMutex.unlock();
						}
						else
						{
							g_Log.LogE("Unable to read texture: {}", fileName);
						}
					}

					m_GlobalTextureStore.at(fileName).Exists = true;
				}

				// Find all removed textures from global directory
				std::vector<std::string> toRemove;
				for (auto& entry : m_GlobalTextureStore)
				{
					if (!entry.second.Exists)
					{
						entry.second.pResourceView->Release();
						entry.second.pResource->Release();
						toRemove.push_back(entry.first);
						continue;
					}

					// If entry wouldn't be in folder next loop, flag wont be set to
					// true and we will remove it
					entry.second.Exists = false;
				}

				// Remove all removed textures from global directory
				m_GlobalTexturesMutex.lock();
				for (auto& entry : toRemove)
				{
					m_GlobalTextureStore.erase(entry);
				}
				m_GlobalTexturesMutex.unlock();

				std::this_thread::sleep_for(std::chrono::milliseconds(SWAP_TEXTURE_UPDATE_DELAY_MS));
			}
		}

	public:
		static inline bool IsGlobalSwapOn;

		TextureSwapThread()
		{
			m_SwapThread = CreateThread(
				nullptr, 0, TextureSwapThreadEntryPoint, nullptr, 0, nullptr);
		}

		~TextureSwapThread()
		{
			// Terminate thread because joining it causes game deadlock for some reason
			TerminateThread(m_SwapThread, 0);
			CloseHandle(m_SwapThread);

			g_Log.LogT("SwapThread -> Exited");

			m_GlobalTexturesMutex.lock();
			for (auto& val : m_GlobalTextureStore)
			{
				g_Log.LogT("SwapThread -> Releasing Texture: {}", val.first);
				val.second.pResource->Release();
				val.second.pResourceView->Release();
			}
			m_GlobalTextureStore.clear();
			m_GlobalTexturesMutex.unlock();
		}

		static void GetTextureSwap(const char* name, ID3D11ShaderResourceView** shaderResourceView)
		{
			if (!IsGlobalSwapOn)
				return;

			m_GlobalTexturesMutex.lock_shared();

			auto swap = m_GlobalTextureStore.find(name);
			if (swap != m_GlobalTextureStore.end())
			{
				*shaderResourceView = swap->second.pResourceView;
			}
			m_GlobalTexturesMutex.unlock_shared();
		}
	};

	inline TextureSwapThread g_TextureSwapThread;
}
