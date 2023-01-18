#pragma once
#include "fwTypes.h"

#ifndef RAGE_STANDALONE
#include "gmFunc.h"
#endif

/**
 * \brief File handle used by fiDevice and fiStream.
 */
typedef int64 fiHandle_t; // Appears in companion symbols as 'long long', not a structure.

static constexpr fiHandle_t FI_INVALID_HANDLE = -1;

namespace rage
{
	enum eFiSeekWhence
	{
		SEEK_FILE_BEGIN = 0,
		SEEK_FILE_CURRENT = 1,
		SEEK_FILE_END = 2,
	};

	struct fiDevice;

	struct fiFindData
	{
		char fileName[256];
		int64_t fileSize;
		uint64_t lastWriteTime;
		uint64_t fileAttributes;
	};
	static_assert(sizeof(fiFindData) == 0x118);

	struct fiDevice_vftable
	{
		fiDevice(*Destruct)(fiDevice*, bool); // 0x0
		// Opens file with read / read & write access
		// Called by fiStream::Open()
		// Packfile doesn't support editing and returns invalid handle if isReadOnly set to false
		fiHandle_t(*Open)(fiDevice*, const char* fileName, bool isReadOnly); // 0x8
		fiHandle_t(*OpenReadCaseSensetive)(fiDevice*, const char* fileName, s64* outOffset); // 0x10
		fiHandle_t(*OpenReadCaseSensetive_Wrapper)(fiDevice*, const char* fileName, s64* outOffset); // 0x18
		fiHandle_t(*OpenWrite)(fiDevice*, const char* fileName); // 0x20
		// Creates a new file. If filed exists, its overwritten.
		// Called by fiStream::Create()
		// Packfile has default implementation that returns -1
		fiHandle_t(*Create)(fiDevice*, const char* fileName); // 0x28
		uint32_t(*Read)(fiDevice*, fiHandle_t, void* buffer, u32 size); // 0x30
		uint64_t* (*ReadOverlapped)(fiDevice*, fiHandle_t, s64 offset, void* buffer, u32 size); // 0x38
		int64_t nullsub_7;			// 0x40
		int64_t(*Write)(fiDevice*, fiHandle_t, void* buffer, u32 size); // 0x48
		int64_t(*Seek)(fiDevice*, fiHandle_t, int32_t offset, eFiSeekWhence whence); // 0x50
		int64_t(*Seek64)(fiDevice*, fiHandle_t, int64_t offset, eFiSeekWhence whence);	// 0x58
		int64_t(*Close)(fiDevice*, fiHandle_t); // 0x60
		int64_t(*CloseBulk)(fiDevice*);	// 0x68
		int32_t(*Size)(fiDevice*, fiHandle_t); // 0x70
		uint64_t(*Size64)(fiDevice*, fiHandle_t); // 0x78
		bool(*Flush)(fiDevice*, fiHandle_t); // 0x80
		bool(*Delete)(fiDevice*, const char* fileName); // 0x88
		bool(*Rename)(fiDevice*, const char* fileName, const char* newName);	// 0x90
		bool(*MakeDirectory)(fiDevice*, const char* dirName); // 0x98
		bool(*DeleteDirectory)(fiDevice*, const char* dirName); // 0xA0
		int64_t nullsub_18;			// 0xA8
		uint64_t(*GetFileSize)(fiDevice*, const char* fileName); // 0xB0
		uint64_t(*GetFileTime)(fiDevice*, const char* fileName); // 0xB8
		bool (*SetFileTime)(fiDevice*, const char* fileName, uint64_t time); // 0xC0
		fiHandle_t(*FindFileBegin)(fiDevice*, const char* search, fiFindData* outFindData); // 0xC8
		bool(*FindFileNext)(fiDevice*, fiHandle_t searchHandle, fiFindData* outFindData); // 0xD0
		bool(*FindFileEnd)(fiDevice*, fiHandle_t searchHandle);	// 0xD8
		int64_t nullsub_24;			// 0xE0
		int64_t sub_7FF71FB7EAB4;	// 0xE8
		bool(*SetEndOfFile)(fiDevice*, fiHandle_t); // 0xF0
		int64_t nullsub_26;			// 0xF8
		u32(*GetAttributes)(fiDevice*, const char* fileName); // 0x100
		bool(*SetAttributes)(fiDevice*, const char* fileName, u32 attributes); // 0x108
		int32_t(*GetRootDeviceId)(fiDevice*); // 0x110
		int64_t sub_7FF71FB8432C;	// 0x118
		void(*WriteBuffer)(fiDevice*, fiHandle_t, const char* buffer, int cursorPos); // 0x120
		int64_t sub_7FF71FB800D8;	// 0x128
		int64_t nullsub_30;			// 0x130
		int64_t nullsub_31;			// 0x138
		int64_t(*GetPhysicalSortKey)(fiDevice*, const char* path); // 0x140
		int64_t nullsub_32;			// 0x148
		int64_t nullsub_33;			// 0x150
		// Returns itself in Packfile
		fiDevice* (*Function34)();	// 0x158
		int64_t nullsub_35;			// 0x160
		int64_t nullsub_36;			// 0x168
		// Gets device type as string. For i.e. fiDeviceLocal returns 'Local'.
		bool(*GetDebugName)(fiDevice*); // 0x170
	};

#ifdef RAGE_STANDALONE
	struct fiDevice
	{
		fiDevice_vftable* vftable;

		static fiDevice* GetDeviceImpl(const char* path, bool isReadOnly)
		{
			throw;
		}
	};
#else
	struct fiDevice;
	namespace hooks
	{
		static inline gm::gmFunc<fiDevice*, const char*, bool> gImpl_fiDevice_GetDeviceImpl(
			"fiDevice::GetDeviceImpl",
			"48 89 5C 24 08 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 83");
	}
	struct fiDevice
	{
		fiDevice_vftable* vftable;

		static fiDevice* GetDeviceImpl(const char* path, bool isReadOnly = true)
		{
			return hooks::gImpl_fiDevice_GetDeviceImpl(path, isReadOnly);
		}
	};
#endif
}
