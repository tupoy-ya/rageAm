//
// File: texturedict.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "texturedx11.h"
#include "rage/paging/template/dictionary.h"

namespace rage
{
	using pgTextureDictionary = pgDictionary<grcTextureDX11>;

	using pTxd = pgRef<pgTextureDictionary>;
}
