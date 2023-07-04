//
// File: xmlutils.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "helpers/cstr.h"

namespace rageam::xml
{
	constexpr const char* GetFieldName(const char* var)
	{
		// Struct.Field
		{
			const char* str = cstr::strrchr(var, '.');
			if (str) return str + 1;
		}
		// Struct->Field
		{
			const char* str = cstr::strrchr(var, '>');
			if (str) return str + 1;
		}
		// Struct::Field
		{
			const char* str = cstr::strrchr(var, ':');
			if (str) return str + 1;
		}
		// Field
		return var;
	}

	// Compile-time variable name
#define GET_FIELD_NAME(var) rageam::xml::GetFieldName(#var)
}
