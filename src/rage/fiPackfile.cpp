#include "fiPackfile.h"

#include <cstring>

#include "fwHelpers.h"
#ifndef RAGE_STANDALONE
#include "Logger.h"
#endif

char g_textBuffer[256]{};

bool rage::fiPackEntry::IsDirectory() const
{
	return (Data >> 40 & 0x7FFFFF) == 0x7FFFFF;
}

bool rage::fiPackEntry::IsResource() const
{
	return Data >> 63 == 1;
}

u16 rage::fiPackEntry::GetOnDiskSize() const
{
	return (u16)(Data >> 16 & 0xFFFFFF);
}

u32 rage::fiPackEntry::GetOffset() const
{
	return (u32)(Data >> 40 << 9);
}

u16 rage::fiPackEntry::GetNameOffset() const
{
	return Data & 0xFFFF;
}

rage::datResourceInfo rage::fiPackEntry::GetResourceInfo() const
{
	return { Resource.VirtualData, Resource.PhysicalData };
}

const char* rage::fiPackfile::GetName() const
{
	if (m_Path == nullptr)
		return "UNDEFINED";
	return m_Path;
}

rage::fiPackEntry* rage::fiPackfile::GetRootEntry() const
{
	return m_Entries;
}

rage::fiPackEntry* rage::fiPackfile::GetEntry(u32 index) const
{
	return &m_Entries[index];
}

const char* rage::fiPackfile::GetEntryName(const rage::fiPackEntry* entry) const
{
	return GetEntryNameByOffset(entry->GetNameOffset());
}

const char* rage::fiPackfile::GetEntryNameByIndex(u32 index) const
{
	return GetEntryName(GetEntry(index));
}

const char* rage::fiPackfile::GetEntryNameByOffset(u32 offset) const
{
	// Root
	if (offset == 0)
		return GetName();

	// Child .rpf entries (rpf inside rpf) name entries are not loaded for some reason.
	if (!m_NameTable)
		return "Not loaded.";

	return &m_NameTable[offset];
}

rage::fiPackEntry* rage::fiPackfile::FindChildEntry(const fiPackEntry* parent, const char* child) const
{
	u32 from = parent->Directory.ChildStartIndex;
	u32 to = parent->Directory.ChildCount + from;

	//g_Log.Log("Scanning directory [{}]: {} from: {} to: {}", parent->NameOffset, GetEntryNameByOffset(parent->NameOffset), from, to);
	//for (u32 i = from; i < to; i++)
	//{
	//	g_Log.Log("[{}] - {}", i, GetEntryNameByIndex(i));
	//}

	while (from <= to)
	{
		// Perform binary search on requested entry

		u32 rangeCenterIndex = (from + to) / 2;
		const char* targetEntryName = GetEntryNameByIndex(rangeCenterIndex);

		int cmp = strcmp(child, targetEntryName);

		//g_Log.Log("cmp {} with {} : {}", child, targetEntryName, cmp);

		// No difference, entry found
		if (cmp == 0)
			return GetEntry(rangeCenterIndex);

		// If negative, entry is located earlier in array (because entries are sorted by name), otherwise - later
		if (cmp == -1)
			to = rangeCenterIndex - 1;
		else
			from = rangeCenterIndex + 1;
	}

	return nullptr;
}

rage::fiPackEntry* rage::fiPackfile::GetEntry(const char* path) const
{
	char c = path[0];

	if (fwHelpers::IsPathSeparator(c))
		path++;

	if (!m_Entries || !m_NameTable)
		return nullptr;

	fiPackEntry* entry = GetRootEntry();

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

			entry = FindChildEntry(entry, g_textBuffer);

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

rage::fiPackEntry* rage::fiPackfile::vftable_FindEntryHeaderByPath(const fiPackfile* inst, const char* path)
{
	// g_Log.Log("fiPackFile::GetEntry({:X} ({}), {})", reinterpret_cast<intptr_t>(inst), inst->GetName(), path);

	return inst->GetEntry(path);
}
