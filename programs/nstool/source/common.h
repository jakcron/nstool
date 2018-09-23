#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/aes.h>
#include <fnd/rsa.h>
#include <nn/hac/nca.h>

static const size_t kMasterKeyNum = 0x20;
static const size_t kNcaKeakNum = nn::hac::nca::kKeyAreaEncryptionKeyNum;

enum FileType
{
	FILE_XCI,
	FILE_NSP,
	FILE_PARTITIONFS,
	FILE_ROMFS,
	FILE_NCA,
	FILE_NPDM,
	FILE_CNMT,
	FILE_NSO,
	FILE_NRO,
	FILE_NACP,
	FILE_PKI_CERT,
	FILE_ES_TIK,
	FILE_HB_ASSET,
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

const byte_t kDummyRightsIdForUserTitleKey[nn::hac::nca::kRightsIdLen] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const byte_t kDummyRightsIdForUserBodyKey[nn::hac::nca::kRightsIdLen] = {0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe};