/*
BitMath.h
(c) 2018 Jakcron

This is a 0x40 byte header to prepend to raw EXEFS .code binaries that provide enough data to be equivalent to an ELF.
*/
#pragma once

// to be deprecated
#define BIT(n) (1ULL << (n))

// Bit math macros
#define _BIT(n) BIT(n)
#define _HAS_BIT(val, bit) ((val) & _BIT(bit) != 0)
