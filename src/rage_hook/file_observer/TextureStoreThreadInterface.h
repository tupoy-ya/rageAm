#pragma once
#include <unordered_set>

#include "FileObserverThreadInterface.h"
#include "../grcore/D3DHelper.h"

namespace fiobs
{
	struct TextureStoreEntry : FileStoreEntry
	{
		ID3D11Resource* pResource;
		ID3D11ShaderResourceView* pResourceView;
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		bool IsDDS;

		using FileStoreEntry::FileStoreEntry;

		HRESULT Load(const std::wstring& path) override
		{
			ID3D11Device* pDevice = rh::D3D::GetDevice();

			IsDDS = path.ends_with(L".dds");
			if (IsDDS)
			{
				return DirectX::CreateDDSTextureFromFile(
					pDevice, path.c_str(), &pResource, &pResourceView);
			}

			std::string path_(path.begin(), path.end());
			AM_TRACEF("TextureStoreEntry::Load() -> Converting to DDS: {}", path_.c_str());
			pResource = nullptr;
			return D3DHelper::ConvertBitmapToDDS(path.c_str(), Format, pDevice, &pResourceView);
		}

		void Release() override
		{
			if (pResource)
				pResource->Release();
			pResourceView->Release();
		}
	};

	/**
	 * \brief Provides interface for texture lookup.
	 */
	class TextureStoreThreadInterface : public FileObserverThreadInterface<TextureStoreEntry>
	{
	public:
		using FileObserverThreadInterface<TextureStoreEntry>::FileObserverThreadInterface;

		bool GetTextureView(const char* name, ID3D11ShaderResourceView** shaderResourceView)
		{
			TextureStoreEntry* entry = Find(name);
			if (entry == nullptr)
				return false;

			*shaderResourceView = entry->pResourceView;
			return true;
		}

		bool GetTextureView(u32 parentHash, const char* name, ID3D11ShaderResourceView** shaderResourceView)
		{
			// TODO: This is the worst usage of map, though in debug mode it works faster

			m_Mutex.lock_shared();
			bool result = false;
			for (auto const& slot : m_FileStore)
			{
				if (slot.second->ParentHash == parentHash && strcmp(slot.first.c_str(), name) == 0)
				{
					*shaderResourceView = ((TextureStoreEntry*)slot.second.get())->pResourceView;
					result = true;
					break;
				}
			}
			m_Mutex.unlock_shared();
			return result;
		}
	};
}
inline fiobs::TextureStoreThreadInterface g_GlobalTextureSwapThreadInterface{ L"rageAm\\Textures\\global", false };
inline fiobs::TextureStoreThreadInterface g_LocalTextureSwapThreadInterface{ L"rageAm\\Textures\\local", true };
