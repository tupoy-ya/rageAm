#include "fiPackfile.h"
#include <cstring>

#include "../../Logger.h"
#include "../fwHelpers.h"

char g_textBuffer[256]{};

bool rage::fiPackfileEntryHeader::IsDirectory() const
{
	return (flags & 0x7FFFFF00) == 0x7FFFFF00;
}

const char* rage::fiPackfile::GetName() const
{
	return members.filePath;
}

rage::fiPackfileEntryHeader* rage::fiPackfile::GetRootEntry() const
{
	return members.headerEntries;
}

rage::fiPackfileEntryHeader* rage::fiPackfile::GetEntryHeaderByIndex(u32 index) const
{
	return &members.headerEntries[index];
}

const char* rage::fiPackfile::GetEntryNameByIndex(u32 index) const
{
	if (index == 0)
		return "root";

	return GetEntryNameByOffset(GetEntryHeaderByIndex(index)->nameOffset);
}

const char* rage::fiPackfile::GetEntryNameByOffset(u32 offset) const
{
	if (offset == 0)
		return "root";

	return &members.entryNames[offset];
}

rage::fiPackfileEntryHeader* rage::fiPackfile::FindChildEntryIndexByName(const fiPackfileEntryHeader* parent,
	const char* child) const
{
	u32 from = parent->entriesStartIndex;
	u32 to = parent->numEntries + from;

	//g_logger->Log("Scanning directory [{}]: {} from: {} to: {}", parent->nameOffset, GetEntryNameByOffset(parent->nameOffset), from, to);
	//for (u32 i = from; i < to; i++)
	//{
	//	g_logger->Log("[{}] - {}", i, GetEntryNameByIndex(i));
	//}

	while (from <= to)
	{
		// Perform binary search on requested entry

		u32 rangeCenterIndex = (from + to) / 2;
		const char* targetEntryName = GetEntryNameByIndex(rangeCenterIndex);

		int cmp = strcmp(child, targetEntryName);

		//g_logger->Log("cmp {} with {} : {}", child, targetEntryName, cmp);

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

rage::fiPackfileEntryHeader* rage::fiPackfile::FindEntryHeaderByPath(const char* path) const
{
	char c = path[0];

	if (fwHelpers::IsPathSeparator(c))
		path++;

	if (!members.headerEntries || !members.entryNames)
		return nullptr;

	fiPackfileEntryHeader* entry = GetRootEntry();

	// Sometimes game requests root directory by literally '\0' path
	if (fwHelpers::IsEndOfString(c))
		return entry;

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
			if (k == 0)
			{
				// Return last found entry (happens in when request directory e.g. 'dlcPacks/'
				return entry;
			}

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

	// Return last found entry (happens in when request directory e.g. 'x64/movies'
	return entry;
}

/* Virtual Table Wrappers */

rage::fiPackfileEntryHeader* rage::fiPackfile::vftable_FindEntryHeaderByPath(const fiPackfile* inst, const char* path)
{
	// g_logger->Log("fiPackFile::FindEntryHeaderByPath({:X} ({}), {})", reinterpret_cast<intptr_t>(inst), inst->GetName(), path);

	return inst->FindEntryHeaderByPath(path);
}
