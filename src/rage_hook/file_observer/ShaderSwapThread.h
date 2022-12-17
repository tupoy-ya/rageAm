#pragma once
#include "../grcore/D3DHelper.h"
#include "../../rage/grcore/grcEffectMgr.h"

namespace fiobs
{
	struct ShaderSwapStoreEntry : FileStoreEntry
	{
		IUnknown* pShader;

		/**
		 * \brief Pointer to original ID3D11Shader# from grcEffect.
		 */
		IUnknown* pOriginalShader;

		/**
		 * \brief Points to where ID3D11Shader# was stored in grcEffect.
		 */
		IUnknown** lpOriginalShader;

		/**
		 * \brief Type of the shader that was resolved.
		 */
		grc::eProgramType ProgramType;

		using FileStoreEntry::FileStoreEntry;

		HRESULT Load(const std::wstring& path) override
		{
			std::string error;
			HRESULT result = D3DHelper::CreateProgramFromFile(path, error, ProgramType, rh::D3D::GetDevice(), &pShader);

			const char* typeStr = ProgramTypeToStr(ProgramType);

			// Formatter doesn't work with wchar
			std::string _path(path.begin(), path.end());

			if (result == S_OK)
				g_Log.LogT("Program {} of type: {} was successfully compiled.", _path, typeStr);
			else
				g_Log.LogE("An error occurred while compiling shader {} of type: {}\n{}", _path, typeStr, error);

			return result;
		}

		void Release() override
		{
			// TODO: Not releasing correctly

			g_Log.LogT("Releasing shader: {:X}", (uintptr_t)pShader);
			*lpOriginalShader = pOriginalShader;
			pShader->Release();
		}
	};

	class ShaderSwapThread : public FileObserverThread<ShaderSwapStoreEntry>
	{
	protected:
		void OnEntryUpdated(std::string dir, std::string name, std::unique_ptr<FileStoreEntry>* entry) const override
		{
			rage::grcEffect* effect = rage::grcEffectMgr::FindEffectByHashKey(fwHelpers::joaat(dir.c_str()));
			if (!effect)
			{
				g_Log.LogE("ShaderSwapThread::OnEntryUpdated() -> Unable to find effect {} in game memory.", dir.c_str());
				return;
			}

			g_Log.LogT("ShaderSwapThread::OnEntryUpdated() -> Found grcEffect for {} at {:X}", dir.c_str(), (uintptr_t)effect);

			ShaderSwapStoreEntry* swapSlot = (ShaderSwapStoreEntry*)entry->get();

			rage::grcProgram* program;
			for (int i = 0; ; i++)
			{
				// AtArray doesn't support dynamic types, have to do this.
				switch (swapSlot->ProgramType)
				{
				case grc::PROGRAM_FRAGMENT: program = effect->pFragmentPrograms.GetAt(i);	break;
				case grc::PROGRAM_VERTEX:	program = effect->pVertexPrograms.GetAt(i);		break;
				case grc::PROGRAM_COMPUTE:	program = effect->pComputePrograms.GetAt(i);	break;
				case grc::PROGRAM_DOMAIN:	program = effect->pDomainPrograms.GetAt(i);		break;
				case grc::PROGRAM_GEOMETRY: program = effect->pGeometryPrograms.GetAt(i);	break;
				case grc::PROGRAM_HULL:		program = effect->pHullPrograms.GetAt(i);		break;
				default: program = nullptr;
				}

				if (!program)
					break;

				// Program names are in format 'fxc:hlsl', compare both parts.
				const char* hlslName = strchr(program->shaderName, ':') + 1;

				if (strcmp(name.c_str(), hlslName) != 0)
					continue;

				// TODO: Rewrite this
				std::string fxcName(program->shaderName);
				fxcName = fxcName.substr(0, fxcName.find(':'));
				if (strcmp(dir.c_str(), fxcName.c_str()) != 0)
					continue;

				break;
			}

			IUnknown** lpShader = &program->pShaderD3D;

			g_Log.LogT("Swapping shader: {}", name.c_str());
			g_Log.LogT("	- grcProgram: {:X}", (uint64_t)program);
			g_Log.LogT("	- Original shader: {:X}", (uint64_t)program->pShaderD3D);
			g_Log.LogT("	- Replace on: {:X}", (uint64_t)swapSlot->pShader);

			swapSlot->lpOriginalShader = lpShader;
			swapSlot->pOriginalShader = *lpShader;
			*lpShader = swapSlot->pShader;
		}
	public:
		using FileObserverThread<ShaderSwapStoreEntry>::FileObserverThread;
	};

	inline ShaderSwapThread g_ShaderSwapThread{ L"rageAm/Shaders", true };
}
