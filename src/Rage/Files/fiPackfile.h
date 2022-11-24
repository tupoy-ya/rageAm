#pragma once
#include <cstdint>

#include "../fwTypes.h"

namespace rage
{
	/**
	 * \brief Contains information about entry (file / directory) in .RPF (pack file).
	 */
	struct fiPackfileEntryHeader
	{
		// Offset of the entry name in fiPackfile name array. (which is sorted list).
		u16 nameOffset;
		u16 unk0x2;
		u32 flags;
		// If directory, specifies start index of child entries.
		u32 entriesStartIndex;
		// If directory, specifies end index of child entries.
		u32 entriesEndIndex;

		bool IsDirectory() const;
	};
	static_assert(sizeof(fiPackfileEntryHeader) == 0x10);

	struct fiPackfile_members
	{
		int8_t byte8;
		int8_t gap9[7];
		// A sorted list of every file / directory null-terminated name.
		const char* entryNames;
		int8_t gap18[8];
		// List of packfile entries headers.
		fiPackfileEntryHeader* headerEntries;
		int32_t numEntries;
		int8_t gap2C[4];
		int64_t qword30;
		int64_t qword38;
		int8_t gap40[16];
		int64_t qword50;
		int8_t gap58[4];
		int8_t byte5C;
		int8_t gap5D[35];
		// Path and name of packfile (i.e. 'update/update.rpf').
		const char* filePath;
		int32_t dword88;
		int8_t gap8C[4];
		int8_t byte90;
		int8_t gap91;
		int16_t word92;
		int8_t gap94[4];
		int64_t qword98;
		int64_t qwordA0;
		int32_t A8;
		int8_t byteAC;
		int8_t byteAD;
		int8_t byteAE;
		int8_t byteAF;
		uint8_t byteB0;
		int8_t byteB1;
		int8_t byteB2;
		int8_t byteB3;
		int8_t byteB4;
		int8_t byteB5;
		int16_t wordB6;
	};

	/**
	 * \brief Provides access to .RPF (pack file) entries.
	 */
	class fiPackfile
	{
		intptr_t vftable;
		fiPackfile_members members;

	public:
		const char* GetName() const;

		fiPackfileEntryHeader* GetRootEntry() const;

		fiPackfileEntryHeader* GetEntryHeaderByIndex(u32 index) const;

		const char* GetEntryNameByIndex(u32 index) const;
		const char* GetEntryNameByOffset(u32 offset) const;

		fiPackfileEntryHeader* FindChildEntryIndexByName(const fiPackfileEntryHeader* parent, const char* child) const;

		// Virtual Function 0x1D8
		fiPackfileEntryHeader* FindEntryHeaderByPath(const char* path) const;

		/* Virtual Table Wrappers */

		static fiPackfileEntryHeader* vftable_FindEntryHeaderByPath(const fiPackfile* inst, const char* path);
	};
}
