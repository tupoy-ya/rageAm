#pragma once
#include "fwTypes.h"

namespace rage
{
	static constexpr RAGE_HANDLE RAGE_INVALID_HANDLE = -1;

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
		RAGE_HANDLE(*Open)(fiDevice*, const char* fileName, bool isReadOnly); // 0x8
		RAGE_HANDLE(*OpenReadCaseSensetive)(fiDevice*, const char* fileName, int64_t& outUnk); // 0x10
		RAGE_HANDLE(*OpenReadCaseSensetive_Wrapper)(fiDevice*, const char* fileName, int64_t& outUnk); // 0x18
		RAGE_HANDLE(*OpenWrite)(fiDevice*, const char* fileName); // 0x20
		// Creates a new file. If filed exists, its overwritten.
		// Called by fiStream::Create()
		// Packfile has default implementation that returns -1
		RAGE_HANDLE(*Create)(fiDevice*, const char* fileName); // 0x28
		int64_t(*Read)(fiDevice*, RAGE_HANDLE, void* buffer, u32 size); // 0x30
		int64_t nullsub_6;			// 0x38
		int64_t nullsub_7;			// 0x40
		int64_t(*Write)(fiDevice*, RAGE_HANDLE, void* buffer, u32 size); // 0x48
		int64_t(*Seek)(fiDevice*, RAGE_HANDLE, int32_t offset, eFiSeekWhence whence); // 0x50
		int64_t(*Seek64)(fiDevice*, RAGE_HANDLE, int64_t offset, eFiSeekWhence whence);	// 0x58
		int64_t(*Close)(fiDevice*); // 0x60
		int64_t(*CloseBulk)(fiDevice*);	// 0x68
		int64_t(*Size)(fiDevice*, RAGE_HANDLE); // 0x70
		int64_t(*Size64)(fiDevice*, RAGE_HANDLE); // 0x78
		int64_t(*Flush)(fiDevice*, RAGE_HANDLE); // 0x80
		bool(*Delete)(fiDevice*, const char* fileName); // 0x88
		bool(*Rename)(fiDevice*, const char* fileName, const char* newName);	// 0x90
		bool(*MakeDirectory)(fiDevice*, const char* dirName); // 0x98
		bool(*DeleteDirectory)(fiDevice*, const char* dirName); // 0xA0
		int64_t nullsub_18;			// 0xA8
		uint64_t(*GetFileSize)(fiDevice*, const char* fileName); // 0xB0
		uint64_t(*GetFileTime)(fiDevice*, const char* fileName); // 0xB8
		bool (*SetFileTime)(fiDevice*, const char* fileName, uint64_t time); // 0xC0
		RAGE_HANDLE(*FindFileBegin)(fiDevice*, const char* search, fiFindData& outFindData); // 0xC8
		bool(*FindFileNext)(fiDevice*, RAGE_HANDLE searchHandle, fiFindData& outFindData); // 0xD0
		bool(*FindFileEnd)(fiDevice*, RAGE_HANDLE searchHandle);	// 0xD8
		int64_t nullsub_24;			// 0xE0
		int64_t sub_7FF71FB7EAB4;	// 0xE8
		bool(*SetEndOfFile)(fiDevice*, RAGE_HANDLE); // 0xF0
		int64_t nullsub_26;			// 0xF8
		u32(*GetAttributes)(fiDevice*, const char* fileName); // 0x100
		bool(*SetAttributes)(fiDevice*, const char* fileName, u32 attributes); // 0x108
		int32_t(*GetRootDeviceId)(fiDevice*); // 0x110
		int64_t sub_7FF71FB8432C;	// 0x118
		int64_t sub_7FF71FB84424;	// 0x120
		int64_t sub_7FF71FB800D8;	// 0x128
		int64_t nullsub_30;			// 0x130
		int64_t nullsub_31;			// 0x138
		int64_t(*GetPhysicalSortKey)(fiDevice*, const char* unkName); // 0x140
		int64_t nullsub_32;			// 0x148
		int64_t nullsub_33;			// 0x150
		int64_t nullsub_34;			// 0x158
		int64_t nullsub_35;			// 0x160
		int64_t nullsub_36;			// 0x168
		// Gets device type as string. For i.e. fiDeviceLocal returns 'Local'.
		bool(*GetDebugName)(fiDevice*); // 0x170
	};

	struct fiDevice
	{
		fiDevice_vftable* vftable;
	};
}
