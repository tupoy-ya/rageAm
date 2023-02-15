#pragma once

#define MACRO_END \
	static_assert(true, "" /* To require semicolon after macro */)

#ifdef _DEBUG
#define DEBUG_ONLY(action) action;
#else
#define DEBUG_ONLY(action) ;
#endif
