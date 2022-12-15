#pragma once

namespace rage
{
	struct grcEffectVar;

	struct grcFragmentProgram
	{
		int64_t qword0;
		int64_t qword8;
		uint32_t* FlagsAndIndices;
		uint16_t numUnkVars;
		int8_t gap1A[6];
		int64_t pConstantBuffers;
		int8_t gap28[4];
		unsigned __int8 constantBufferStartSlot;
		unsigned __int8 constantBufferEndSlot;
		int8_t gap2E[120];
		uint32_t numEffectVars;
		rage::grcEffectVar* effectVarArray;
		int8_t gapB8[368];
		ID3D11PixelShader* pPixelShaderD3D11;
		int64_t qword230;
	};
	static_assert(sizeof(grcFragmentProgram) == 0x238);
	static_assert(offsetof(grcFragmentProgram, pPixelShaderD3D11) == 0x228);
	static_assert(offsetof(grcFragmentProgram, effectVarArray) == 0xB0);
}
