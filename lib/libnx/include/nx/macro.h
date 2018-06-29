#pragma once
#include <cstdint>

#define _MAKE_STRUCT_MAGIC_U32(x) ((uint32_t)(x[3]) << 24 | (uint32_t)(x[2]) << 16 | (uint32_t)(x[1]) << 8 | (uint32_t)(x[0]))
#define _MAKE_STRUCT_MAGIC_U64(x) ((uint64_t)(x[7]) << 56 | (uint64_t)(x[6]) << 48 | (uint64_t)(x[5]) << 40 | (uint64_t)(x[4]) << 32 | (uint64_t)(x[3]) << 24 | (uint64_t)(x[2]) << 16 | (uint64_t)(x[1]) << 8 | (uint64_t)(x[0]))
