/*
BitMath.h
(c) 2018 Jakcron
*/
#pragma once

// to be deprecated
#define BIT(n) (1ULL << (n))

// Bit math macros
#define _BIT(n) BIT(n)
#define _HAS_BIT(val, bit) (((val) & _BIT(bit)) != 0)
#define _SET_BIT(val, bit) ((val) |= _BIT(bit))
#define _BITMASK(width) (_BIT(width)-1)