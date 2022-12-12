#pragma once

namespace rage
{
	struct fwModelId
	{
		static constexpr uint32_t MI_INVALID = 0; // Not sure about the value

		uint32_t ModelIdAndFlags;

		fwModelId(uint16_t id)
		{
			ModelIdAndFlags = id | 0x0FFF0000; // | 0x10000000;
		}

		uint16_t GetModelIndex() const
		{
			return ModelIdAndFlags & 0x0000FFFF;
		}

		uint16_t GetFlags() const
		{
			return ModelIdAndFlags >> 16;
		}
	};
	static_assert(sizeof(fwModelId) == 0x4);
}
