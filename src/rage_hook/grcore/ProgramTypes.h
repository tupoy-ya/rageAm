#pragma once

namespace grc
{
	enum eProgramType
	{
		PROGRAM_UNKNOWN = 0,
		PROGRAM_FRAGMENT = 1,
		PROGRAM_VERTEX = 2,
		PROGRAM_COMPUTE = 3,
		PROGRAM_DOMAIN = 4,
		PROGRAM_GEOMETRY = 5,
		PROGRAM_HULL = 6,
	};

	inline const char* ProgramTypeToStr(eProgramType e)
	{
		switch (e)
		{
		case PROGRAM_UNKNOWN: return "PROGRAM_UNKNOWN";
		case PROGRAM_FRAGMENT: return "PROGRAM_FRAGMENT";
		case PROGRAM_VERTEX: return "PROGRAM_VERTEX";
		case PROGRAM_COMPUTE: return "PROGRAM_COMPUTE";
		case PROGRAM_DOMAIN: return "PROGRAM_DOMAIN";
		case PROGRAM_GEOMETRY: return "PROGRAM_GEOMETRY";
		case PROGRAM_HULL: return "PROGRAM_HULL";
		default: return "unknown";
		}
	}
}
