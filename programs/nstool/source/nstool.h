#pragma once
#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/aes.h>
#include <fnd/rsa.h>
#include <nn/hac/nca.h>

static const size_t kMasterKeyNum = 0x20;
static const size_t kNcaKeakNum = nn::hac::nca::kKeyAreaEncryptionKeyNum;

enum IFileOwnershipMode
{
	SHARED_IFILE = false,
	OWN_IFILE = true
};

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

struct sKeyset
{
	fnd::rsa::sRsa2048Key acid_sign_key;
	fnd::aes::sAes128Key package1_key[kMasterKeyNum];
	fnd::rsa::sRsa2048Key package2_sign_key;
	fnd::aes::sAes128Key package2_key[kMasterKeyNum];

	struct sNcaData
	{
		fnd::rsa::sRsa2048Key header_sign_key;
		fnd::aes::sAesXts128Key header_key;
		fnd::aes::sAes128Key key_area_key[kNcaKeakNum][kMasterKeyNum];

		fnd::aes::sAes128Key manual_title_key_aesctr;
		fnd::aes::sAesXts128Key manual_title_key_aesxts;
		fnd::aes::sAes128Key manual_body_key_aesctr;
		fnd::aes::sAesXts128Key manual_body_key_aesxts;
	} nca;

	struct sXciData
	{
		fnd::rsa::sRsa2048Key header_sign_key;
		fnd::aes::sAes128Key header_key;
	} xci;

	struct sTicketData
	{
		fnd::rsa::sRsa2048Key sign_key;
		fnd::aes::sAes128Key titlekey_kek[kMasterKeyNum];
	} ticket;

	struct sPkiData
	{
		fnd::rsa::sRsa4096Key root_sign_key;
	} pki;
};

inline byte_t charToByte(char chr)
{
	if (chr >= 'a' && chr <= 'f')
		return (chr - 'a') + 0xa;
	else if (chr >= 'A' && chr <= 'F')
		return (chr - 'A') + 0xa; 
	else if (chr >= '0' && chr <= '9')
		return chr - '0';
	return 0;
}
