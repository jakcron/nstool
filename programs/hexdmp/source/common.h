#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/aes.h>
#include <fnd/rsa.h>

enum IFileOwnershipMode
{
	SHARED_IFILE = false,
	OWN_IFILE = true
};

enum FileType
{
	FILE_RAW,
	FILE_INVALID = -1,
};

enum CliOutputModeFlag
{
	OUTPUT_BASIC,
	OUTPUT_LAYOUT,
	OUTPUT_KEY_DATA,
	OUTPUT_EXTENDED
};

typedef byte_t CliOutputMode;

template <typename T>
struct sOptional
{
	bool isSet;
	T var;
	inline sOptional() : isSet(false) {}
	inline sOptional(const T& other) : isSet(true), var(other) {}
	inline sOptional(const sOptional& other) : isSet(other.isSet), var(other.var) {}
	inline const T& operator=(const T& other) { isSet = true; var = other; return var; }
	inline const sOptional<T>& operator=(const sOptional<T>& other) 
	{
		isSet = other.isSet;
		if (isSet) {
			var = other.var;
		}
		return *this; 
	}
	inline T& operator*() { return var; }
};