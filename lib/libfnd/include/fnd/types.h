#pragma once
#include <cinttypes>
#include <fnd/Exception.h>
#include <fnd/BitMath.h>
#include <fnd/Endian.h>

typedef uint8_t byte_t;

#define _MIN(x,y) ((x) <= (y)? (x) : (y))
#define _MAX(x,y) ((x) >= (y)? (x) : (y))

static inline uint64_t align(uint64_t size, uint64_t align)
{
	return (size % align) == 0? size : (size - (size % align) + align); 
}

