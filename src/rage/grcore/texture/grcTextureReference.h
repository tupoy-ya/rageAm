#pragma once
#include "grcTexture.h"

namespace rage
{
	class grcTextureReferenceBase : public grcTexture
	{
		grcTexture* m_TextureRef;
	};
	static_assert(sizeof(grcTextureReferenceBase) == 0x50);

	class grcTextureReference : public grcTextureReferenceBase
	{

	};
}
