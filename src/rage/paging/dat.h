#pragma once
#include "../fwTypes.h"

/*
 * Before exploring code below, make sure to understand virtual & physical memory management in operating systems.
 * https://stackoverflow.com/questions/14347206/what-are-the-differences-between-virtual-memory-and-physical-memory
 *
 * Resource: #ft, #dr, #td, #cd etc.
 * Short-term: datResource provides functionality to map resource addresses (or file offsets) to allocated chunks in game heap.
 * All resource addresses are split in two types: virtual and physical (in CodeWalker they're called system and graphical),
 * The only difference is that physical addresses point to data that will be allocated GPU (for i.e. textures)
 * and will be removed from game memory as soon D3D object was created.
 */

static constexpr u64 DAT_VIRTUAL_BASE = 0x50000000;
static constexpr u64 DAT_PHYSICAL_BASE = 0x60000000;
static constexpr u64 DAT_BASE_SIZE = 0x2000;

static constexpr u8 DAT_NUM_CHUNKS = 128;

static constexpr u64 DAT_CHUNK_SIZE_MASK = 0x00FF'FFFF'FFFF'FFFF;
static constexpr u64 DAT_CHUNK_INDEX_SHIFT = 56; // 8 highest bits
static constexpr u64 DAT_CHUNK_INDEX_MASK = 0xFF;
