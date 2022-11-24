#pragma once
#include <cstdint>
#include <cstring>

#include "../fwHelpers.h"
#include "../fwTypes.h"

namespace rage
{
	char g_textBuffer[256]{};

	struct fiPackfileEntryHeader
	{
		u16 nameIndex;
		u16 unk0x2;
		u32 flags;
		u32 entriesStartIndex;
		u32 entriesEndIndex;

		bool IsDirectory() const
		{
			return (flags & 0x7FFFFF00) == 0x7FFFFF00;
		}
	};
	static_assert(sizeof(fiPackfileEntryHeader) == 0x10);

	struct fiPackfile_members
	{
		int8_t byte8;
		int8_t gap9[7];
		const char* entryNames;
		int8_t gap18[8];
		fiPackfileEntryHeader* headerEntries;
		int32_t dword28;
		int8_t gap2C[4];
		int64_t qword30;
		int64_t qword38;
		int8_t gap40[16];
		int64_t qword50;
		int8_t gap58[4];
		int8_t byte5C;
		int8_t gap5D[35];
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

	class fiPackfile
	{
		intptr_t vftable;
		fiPackfile_members members;

	public:
		static fiPackfileEntryHeader* vftable_FindEntryHeaderByPath(const fiPackfile* inst, const char* path)
		{
			g_logger->Log(std::format("fiPackFile::FindEntryHeaderByPath({:X}, {})", (intptr_t)inst, path));
			return inst->FindEntryHeaderByPath(path);
		}

		fiPackfileEntryHeader* GetRootEntry() const
		{
			return members.headerEntries;
		}

		fiPackfileEntryHeader* GetEntryHeaderByIndex(u32 index) const
		{
			return &members.headerEntries[index];
		}

		const char* GetEntryNameByIndex(u32 index) const
		{
			return &members.entryNames[GetEntryHeaderByIndex(index)->nameIndex];
		}

		fiPackfileEntryHeader* FindChildEntryIndexByName(const fiPackfileEntryHeader* parent, const char* child) const
		{
			u32 from = parent->entriesStartIndex;
			u32 to = parent->entriesEndIndex + from;

			while (from <= to)
			{
				// Perform binary search on requested entry

				u32 rangeCenterIndex = (from + to) / 2;
				const char* targetEntryName = GetEntryNameByIndex(rangeCenterIndex);

				int cmp = strcmp(child, targetEntryName);

				// No difference, entry found
				if (cmp == 0)
					return GetEntryHeaderByIndex(rangeCenterIndex);

				// If negative, entry is located earlier in array (because entries are sorted by name), otherwise - later
				if (cmp == -1)
					to = rangeCenterIndex - 1;
				else
					from = rangeCenterIndex + 1;
			}

			return nullptr;
		}

		// Virtual Function 0x8
		fiPackfileEntryHeader* FindEntryHeaderByPath(const char* path) const
		{
			char c = path[0];

			if (fwHelpers::IsEndOfString(c))
				return nullptr;

			if (fwHelpers::IsPathSeparator(c))
				path++;

			if (!members.headerEntries || !members.entryNames)
				return nullptr;

			fiPackfileEntryHeader* entry = GetRootEntry();

			// Divide given path (e.g. 'common/ai/handling.meta') into
			// tokens ('common', 'ai', 'handling.meta')
			// and look up each of them in packFile name table
			int i = 0;
			int k = 0;
			do
			{
				c = path[i];

				if (fwHelpers::IsPathSeparator(c) || fwHelpers::IsEndOfString(path[i]))
				{
					// Finalize path token
					g_textBuffer[k] = '\0';

					// Reset buffer iterator for next token
					k = 0;

					entry = FindChildEntryIndexByName(entry, g_textBuffer);

					if (!entry)
						return nullptr;

					// Go deeper in hierarchy if token is directory
					if (entry->IsDirectory())
						continue;

					// We found requested file
					return entry;
				}

				// Path has to be lower case
				c = fwHelpers::ToLower(c);

				g_textBuffer[k++] = c;
			} while (!fwHelpers::IsEndOfString(path[i++]));

			// Entry was not found
			return nullptr;
		}
	};
}
