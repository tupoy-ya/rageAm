#pragma once
#include <cstdint>
#include <Windows.h>

#include "fiDevice.h"
#include "fwTypes.h"
#include "paging/datResourceInfo.h"

namespace rage
{
	class fiPackfile;

	/**
	 * \brief Contains information about packfile entry (file / directory / resource).
	 */
	struct fiPackEntry
	{
		uint64_t Data;
		union
		{
			struct {
				uint32_t Size;
				uint32_t Encryption;
			} Binary;

			struct {
				uint32_t VirtualData;
				uint32_t PhysicalData;
			} Resource;

			struct {
				uint32_t ChildStartIndex;
				uint32_t ChildCount;
			} Directory;
		};

		bool IsDirectory() const;
		bool IsResource() const;
		u16 GetOnDiskSize() const;
		u32 GetOffset() const;
		u16 GetNameOffset() const;
		datResourceInfo GetResourceInfo() const;
	};
	static_assert(sizeof(fiPackEntry) == 0x10);

	/**
	 * \brief Provides access to .RPF (pack file) entries.
	 */
	class fiPackfile
	{
		intptr_t vftable;
		int8_t byte8;
		int8_t gap9[7];
		// A sorted list of every file / directory null-terminated name.
		const char* m_NameTable;
		const char* m_NameTableEnd;
		// List of packfile entries.
		fiPackEntry* m_Entries;
		int32_t m_NumEntries;
		int8_t gap2C[4]; // Once was 'ydr'? what
		fiHandle_t m_Handle;
		FILETIME m_FileTime;
		int8_t gap40[16];
		fiDevice* m_Parent;
		// Used to truncate full path with mounting point like 'dlcMPBeach:/common/data/handling.meta'
		// to 'common/data/handling.meta' when mounting scope is no longer needed
		int32_t m_MountPointLength;
		char m_Name[32];
		// Path and name of packfile (i.e. 'update/update.rpf').
		const char* m_Path;
		int32_t dword88;
		int8_t gap8C[4];
		int8_t byte90;
		int8_t gap91;
		u16 m_CollectionIndex;
		uint32_t gap94;
		int64_t qword98;
		void* FileData;
		uint8_t gapA8;
		uint8_t gapA1;
		uint8_t gapA2;
		uint8_t byteAB;
		uint32_t qwordAC;
		uint32_t entriesSize_Plus16;
		uint8_t nameShift;
		uint8_t byteb1;
		uint8_t byteb2;
		uint8_t byteb3;
		uint8_t byteb4;
		uint8_t byteb5;
		uint16_t wordB9;

		const char* GetEntryNameByIndex(u32 index) const;
		const char* GetEntryNameByOffset(u32 offset) const;
	public:
		const char* GetName() const;
		fiPackEntry* GetRootEntry() const;

		fiPackEntry* GetEntry(u32 index) const;
		fiPackEntry* GetEntry(const char* path) const; // Virtual Function 0x1D8
		fiPackEntry* FindChildEntry(const fiPackEntry* parent, const char* child) const;

		const char* GetEntryName(const fiPackEntry* entry) const;

		u32 GetEntryCount() const { return m_NumEntries; }
		fiHandle_t GetHandle() const { return m_Handle; }
		fiDevice* GetParent() const { return m_Parent; }

		/* Virtual Table Wrappers */

		static fiPackEntry* vftable_FindEntryHeaderByPath(const fiPackfile* inst, const char* path);
	};
	static_assert(sizeof(fiPackfile) == 0xC0);
}
