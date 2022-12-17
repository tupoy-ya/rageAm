#pragma once
#include "FileObserverThreadInterface.h"

namespace fiobs
{
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

	class TextureSwapThreadInterface : public FileObserverThreadInterface<TextureSwapStoreEntry>
	{
	public:
		bool IsGlobalSwapOn;

		using FileObserverThreadInterface<TextureSwapStoreEntry>::FileObserverThreadInterface;

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
}
inline fiobs::TextureSwapThreadInterface g_TextureSwapThreadInterface{ L"rageAm/Textures/global", false };
