#pragma once

#include "fwTypes.h"
#include "grcTextureInfo.h"

/*
 * vftable grcTexture
 *		2802 Adds 6 methods with static variable in beginning
 *		0x0		~grcTextureDX11();
 *		0x8		uint32_t Get0x44();
 *		0x10	Set0x44(uint32_t);
 *		0x18	// ret 0
 *		0x20	// ret 0
 *		0x28	GetWidth();
 *		0x30	GetHeight();
 *		0x38	GetDepth();
 *		0x40	GetMipLevels();
 *		0x48
 *		0x50	GetBitsPerPixel();
 *		0x58	// ret 1
 *		0x60
 *		0x68	// ret 0
 *		0x70	// ret 0
 *		0x78	// ret 0
 *		0x80	// ret 0
 *		0x88	// ret this
 *		0x90	// ret this
 *		0x98	// ret byte5F & 1
 *		0xA0	// ret 0x38
 *		0xA8	// ret 0x38
 *		0xB0	GetShaderResourceView();
 */

namespace rage
{
	// This is a pure virtual base of rage::grcTexture.
	class grcTextureVFBase
	{
	public:
		virtual ~grcTextureVFBase() = default;

	protected:
		virtual int32_t Get44() const = 0;
		virtual void Set44(int32_t value) = 0;

	private:
		virtual bool Function3() { return false; }
		virtual bool Function4() { return false; }

	public:
		virtual u16 GetWidth() const = 0;
		virtual u16 GetHeight() const = 0;
		virtual u16 GetDepth() const = 0;
		virtual u8 GetMipLevels() const = 0;

	private:
		virtual void Function9() {}

	public:
		virtual u8 GetBitsPerPixel() const = 0;

	private:
		virtual bool Function11() { return true; }
		virtual u32 Function12(u32& a1) { a1 = 0; return a1; }
		virtual bool Function13() { return false; }
		virtual bool Function14() { return false; }
		virtual bool Function15() { return false; }
		virtual bool Function16() { return false; }
		virtual grcTextureVFBase* Function17() { return this; }
		virtual grcTextureVFBase* Function18() { return this; }

	protected:
		virtual bool GetUnk19() = 0;
		virtual void* GetResource1() = 0;
		virtual void* GetResource2() = 0;

	public:
		virtual void* GetResourceView() const = 0;

	private:
		virtual bool Function23() { return false; }

	protected:
		virtual u32 ConvertUnkFormat() { return 0; }

	public:
		virtual grcTextureInfo GetTextureInfo() = 0;

	private:
		virtual bool Function26() { return false; }
		virtual bool Function27() { return false; }
		virtual bool Function28() { return false; }
		virtual bool Function29() { return false; }
		virtual bool Function30() { return false; }
		virtual bool Function31() { return false; }
		virtual bool Function32() { return false; }
		virtual bool Function33() { return false; }
		virtual bool Function34() { return false; }
		virtual bool Function35() { return false; }
		virtual bool Function36() { return false; }
		virtual bool Function37() { return false; }
		virtual bool Function38() { return false; }

	protected:
		virtual u32 GetUnkFlags(u32& flags) = 0;

	private:
		virtual bool Function40() { return false; }
		virtual bool Function41() { return false; }
	};
}
