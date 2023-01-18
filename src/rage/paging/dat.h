#pragma once
#include "../fwTypes.h"

/*
 * Before exploring code below, make sure to understand page memory management in operating systems.
 * https://stackoverflow.com/questions/14347206/what-are-the-differences-between-virtual-memory-and-physical-memory
 *
 * There's good russian post on this system: https://sannybuilder.com/forums/viewtopic.php?id=433&p=1
 * Note that names in this post are not entirely accurate (because we got them only in 2018 from Companion RDR2 android app).
 *
 * Resources are: #ft, #dr, #td, #cd and so on.
 * Short-term: datResource provides functionality to map resource addresses (or file offsets) to allocated chunks in game heap.
 * All resource addresses are split in two types: virtual and physical (in CodeWalker / Open IV terminology they're called system and graphical),
 * The only difference is that physical addresses point to data that will be allocated GPU (for i.e. textures)
 * and will be de-allocated after resource is constructed.
 *
 * Building resource is done in following steps:
 * - Read resource header (datResourceInfo)
 * - Build map from encoded data (datResourceInfo, datResourceMap)
 * - Read resource data from file
 * - Allocate chunks in process memory at chunk destination address with given chunk size
 * - Iterate through each chunk in datResourceMap, decompress with zlib and copy to allocated chunk memory (destination address)
 * (NOTE: you know size to decompress from chunk size)
 * - Now you have resources in your memory, invoke placement new constructor on that memory for resource structure
 * (Resource structure (for e.g. pgDictionary<grcTexture>) is identified only by file extension).
 * - Placement new constructor (see pgDictionary(const datResource&)) adjusts field offsets using Fixup function
 * (which just look ups for chunk and takes it's destination address)
 * - De-allocate physical chunks (GPU)
 * - Done! Resource is ready to be used.
 */

static constexpr u64 DAT_VIRTUAL_BASE = 0x50000000; // RAM
static constexpr u64 DAT_PHYSICAL_BASE = 0x60000000; // GPU
static constexpr u64 DAT_BASE_SIZE = 0x2000;

static constexpr u8 DAT_NUM_CHUNKS = 128;

static constexpr u64 DAT_CHUNK_SIZE_MASK = 0x00FF'FFFF'FFFF'FFFF;
static constexpr u64 DAT_CHUNK_INDEX_SHIFT = 56; // 8 highest bits
static constexpr u64 DAT_CHUNK_INDEX_MASK = 0xFF;
