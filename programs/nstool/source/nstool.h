#pragma once
#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/rsa.h>
#include <nx/nca.h>

static const size_t kMasterKeyNum = 0x20;
static const size_t kNcaKeakNum = nx::nca::kKeyAreaEncryptionKeyNum;


enum FileType
{
	FILE_XCI,
	FILE_PARTITIONFS,
	FILE_ROMFS,
	FILE_NCA,
	FILE_NPDM,
	FILE_INVALID = -1,
};

enum CliOutputType
{
	OUTPUT_MINIMAL,
	OUTPUT_NORMAL,
	OUTPUT_VERBOSE
};

template <typename T>
struct sOptional
{
	bool isSet;
	T var;
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
	crypto::rsa::sRsa2048Key acid_sign_key;

	crypto::aes::sAes128Key package1_key[kMasterKeyNum];
	crypto::rsa::sRsa2048Key package2_sign_key;
	crypto::aes::sAes128Key package2_key[kMasterKeyNum];

	struct sNcaData
	{
		crypto::rsa::sRsa2048Key header_sign_key;
		crypto::aes::sAesXts128Key header_key;
		crypto::aes::sAes128Key key_area_key[kNcaKeakNum][kMasterKeyNum];

		crypto::aes::sAes128Key manual_title_key_aesctr;
		crypto::aes::sAesXts128Key manual_title_key_aesxts;
		crypto::aes::sAes128Key manual_body_key_aesctr;
		crypto::aes::sAesXts128Key manual_body_key_aesxts;
	} nca;

	struct sXciData
	{
		crypto::rsa::sRsa2048Key header_sign_key;
		crypto::aes::sAes128Key header_key;
	} xci;
	
	struct sTicketData
	{
		crypto::rsa::sRsa2048Key sign_key;
		crypto::aes::sAes128Key titlekey_kek[kMasterKeyNum];
	} ticket;
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
