#pragma once
#include "ProgramTypes.h"
#include "../vendor/directxtex/include/DirectXTex.h"

namespace D3DHelper
{
	inline grc::eProgramType GetProgramTypeFromFile(const std::wstring& fileName)
	{
		if (fileName.find(L"PS_") != -1) return grc::PROGRAM_FRAGMENT;
		if (fileName.find(L"VS_") != -1) return grc::PROGRAM_VERTEX;
		if (fileName.find(L"CS_") != -1) return grc::PROGRAM_COMPUTE;
		if (fileName.find(L"DS_") != -1) return grc::PROGRAM_DOMAIN;
		if (fileName.find(L"GS_") != -1) return grc::PROGRAM_GEOMETRY;
		if (fileName.find(L"HS_") != -1) return grc::PROGRAM_HULL;
		return grc::PROGRAM_UNKNOWN;
	}

	inline const char* GetTargetFromType(grc::eProgramType type)
	{
		switch (type)
		{
		case grc::PROGRAM_FRAGMENT: return "ps_5_0";
		case grc::PROGRAM_VERTEX: return "vs_5_0";
		case grc::PROGRAM_COMPUTE: return "cs_5_0";
		case grc::PROGRAM_DOMAIN: return "ds_5_0";
		case grc::PROGRAM_GEOMETRY: return "gs_5_0";
		case grc::PROGRAM_HULL: return "hs_5_0";
		default: return "";
		}
	}

	inline HRESULT CreateShaderFromTypeAndBytecode(grc::eProgramType type, ID3D11Device* pDevice, ID3DBlob* pBytecode, IUnknown** lpShader)
	{
		*lpShader = nullptr;

		// We can't just give it pointer to IUnknown, otherwise E_INVALIDARG will be returned.
		ID3D11PixelShader* ps;
		ID3D11VertexShader* vs;
		ID3D11ComputeShader* cs;
		ID3D11DomainShader* ds;
		ID3D11GeometryShader* gs;
		ID3D11HullShader* hs;

		HRESULT result;

		void* bytecode = pBytecode->GetBufferPointer();
		SIZE_T bytecodeLength = pBytecode->GetBufferSize();
		switch (type)
		{
		case grc::PROGRAM_FRAGMENT:
			result = pDevice->CreatePixelShader(bytecode, bytecodeLength, nullptr, &ps);
			*lpShader = ps; break;
		case grc::PROGRAM_VERTEX:
			result = pDevice->CreateVertexShader(bytecode, bytecodeLength, nullptr, &vs);
			*lpShader = vs; break;
		case grc::PROGRAM_COMPUTE:
			result = pDevice->CreateComputeShader(bytecode, bytecodeLength, nullptr, &cs);
			*lpShader = cs; break;
		case grc::PROGRAM_DOMAIN:
			result = pDevice->CreateDomainShader(bytecode, bytecodeLength, nullptr, &ds);
			*lpShader = ds; break;
		case grc::PROGRAM_GEOMETRY:
			result = pDevice->CreateGeometryShader(bytecode, bytecodeLength, nullptr, &gs);
			*lpShader = gs; break;
		case grc::PROGRAM_HULL:
			result = pDevice->CreateHullShader(bytecode, bytecodeLength, nullptr, &hs);
			*lpShader = hs; break;
		default: result = E_INVALIDARG;
		}

		return result;
	}

	inline HRESULT CompileProgramFromFile(const std::wstring& fileName, std::string& error, grc::eProgramType type, ID3DBlob** lpBytecode)
	{
		const char* target = GetTargetFromType(type);

		ID3DBlob* errorMsg;
		HRESULT result = D3DCompileFromFile(
			fileName.c_str(), nullptr, nullptr, "main", target, 0, 0, lpBytecode, &errorMsg);

		if (errorMsg)
		{
			error = std::string((const char*)errorMsg->GetBufferPointer(), errorMsg->GetBufferSize() - 1);
			errorMsg->Release();
		}

		return result;
	}

	inline HRESULT CreateProgramFromFile(const std::wstring& fileName, std::string& error, grc::eProgramType& programType, ID3D11Device* pDevice, IUnknown** pShader)
	{
		programType = GetProgramTypeFromFile(fileName);

		if (programType == grc::PROGRAM_UNKNOWN)
			return E_INVALIDARG;

		ID3DBlob* pBytecode;
		HRESULT result = CompileProgramFromFile(fileName, error, programType, &pBytecode);

		if (result != S_OK)
			return result;

		return CreateShaderFromTypeAndBytecode(programType, pDevice, pBytecode, pShader);
	}

	// Required by DirectXTex library.
	inline bool InitCom()
	{
		static thread_local bool isInitialized = false;
		if (isInitialized)
			return true;

		HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(result))
		{
			AM_ERR("D3DHelper::InitCom() -> Failed to initialize.");
			return false;
		}

		isInitialized = true;
		return true;
	}

	inline HRESULT ConvertBitmapToDDS(const wchar_t* fileName, DXGI_FORMAT fmt, ID3D11Device* pDevice, ID3D11ShaderResourceView** ppResourceView)
	{
		InitCom();

		HRESULT result;
		*ppResourceView = nullptr;

		DirectX::ScratchImage image;
		result = LoadFromWICFile(fileName, DirectX::WIC_FLAGS_NONE, nullptr, image);

		// TODO: Pixel transparency detect

		if (result != S_OK)
			return result;

		const DirectX::TexMetadata& imageMeta = image.GetMetadata();
		size_t w = imageMeta.width;
		size_t h = imageMeta.height;
		if ((w & -w) != w || (h & -h) != h)
		{
			AM_ERR("Texture with dimension that is not power of 2 is not supported.");
			return E_INVALIDARG;
		}

		DirectX::ScratchImage mipChain;
		result = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
			DirectX::TEX_FILTER_DEFAULT, 0, mipChain);

		if (result != S_OK)
			return result;

		DirectX::ScratchImage compressed;

		if (fmt == DXGI_FORMAT_BC7_UNORM_SRGB || fmt == DXGI_FORMAT_BC7_UNORM || fmt == DXGI_FORMAT_BC7_TYPELESS)
		{
			// TODO: Crashes game
			// GPU Compression for BC7
			result = Compress(pDevice, mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(),
				fmt, DirectX::TEX_COMPRESS_DITHER | DirectX::TEX_COMPRESS_PARALLEL, // | DirectX::TEX_COMPRESS_BC7_USE_3SUBSETS,
				1.0f, compressed);
		}
		else
		{
			result = Compress(mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(),
				fmt,
				DirectX::TEX_COMPRESS_DITHER | DirectX::TEX_COMPRESS_PARALLEL,
				DirectX::TEX_THRESHOLD_DEFAULT, compressed);
		}

		DirectX::ScratchImage* pImage = &compressed;
		if (result != S_OK)
		{
			pImage = &mipChain;
			AM_TRACE("Unable to compress texture. Asumming non-compressive format was passed.");
		}

		result = CreateShaderResourceViewEx(
			pDevice, pImage->GetImages(), pImage->GetImageCount(), pImage->GetMetadata(),
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::CREATETEX_DEFAULT,
			ppResourceView);

		return result;
	}
}
