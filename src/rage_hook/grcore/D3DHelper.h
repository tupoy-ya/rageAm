#pragma once
#include "ProgramTypes.h"

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
		int bytecodeLength = pBytecode->GetBufferSize();
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
}
