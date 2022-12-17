#pragma once

namespace rage
{
	struct grcEffectVar;

	struct grcProgram
	{
		int64_t vftable;
		const char* shaderName;
		uint32_t* FlagsAndIndices;
		uint16_t numUnkVars;
		int8_t gap1A[6];
		int64_t pConstantBuffers;
		int8_t gap28[4];
		unsigned __int8 constantBufferStartSlot;
		unsigned __int8 constantBufferEndSlot;
		int8_t gap2E[120];
		uint32_t numEffectVars;
		grcEffectVar* effectVarArray;
		int8_t gapB8[368];
		IUnknown* pShaderD3D;
		int64_t qword230;

		/**
		 * \brief Gets name & effect name of program in format: 'EFFECT:PROGRAM',
		 * for i.e. 'vehicle_paint1:PS_DeferredVehicleTextured'.
		 * \remark NOTE: Name can contain relative mount point, 'common:/shaders/im:PS_Clear'.
		 * \return A string containing program name.
		 */
		const char* GetPath() const
		{
			return shaderName;
		}
	};
	static_assert(sizeof(grcProgram) == 0x238);
	static_assert(offsetof(grcProgram, pShaderD3D) == 0x228);

	struct grcFragmentProgram : grcProgram {};
	struct grcVertexProgram : grcProgram
	{
		int64_t qword238;
		int64_t qword240;
	};
	struct grcComputeProgram : grcProgram {};
	struct grcDomainProgram : grcProgram {};
	struct grcHullProgram : grcProgram {};
	struct grcGeometryProgram : grcProgram
	{
		int64_t qword238;
		int64_t qword240;
	};

	static_assert(sizeof(grcFragmentProgram) == 0x238);
	static_assert(sizeof(grcVertexProgram) == 0x248);
	static_assert(sizeof(grcComputeProgram) == 0x238);
	static_assert(sizeof(grcDomainProgram) == 0x238);
	static_assert(sizeof(grcHullProgram) == 0x238);
	static_assert(sizeof(grcGeometryProgram) == 0x248);
}
