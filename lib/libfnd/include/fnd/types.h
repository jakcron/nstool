#pragma once
#include <cinttypes>
#include <fnd/Exception.h>
#include <fnd/BitMath.h>
#include <fnd/Endian.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef uint8_t byte_t;

#define MIN(x,y) ((x) <= (y)? (x) : (y))
#define MAX(x,y) ((x) >= (y)? (x) : (y))

static inline uint64_t align(uint64_t size, uint64_t align)
{
	return (size % align) == 0? size : (size - (size % align) + align); 
}

