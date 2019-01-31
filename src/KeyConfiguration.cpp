#include "KeyConfiguration.h"
#include <fnd/ResourceFileReader.h>
#include <fnd/SimpleTextOutput.h>
#include <nn/hac/AesKeygen.h>
#include <nn/hac/NcaUtils.h>

KeyConfiguration::KeyConfiguration()
{
	clearGeneralKeyConfiguration();
	clearNcaExternalKeys();
}

KeyConfiguration::KeyConfiguration(const KeyConfiguration& other)
{
	*this = other;
}

void KeyConfiguration::operator=(const KeyConfiguration& other)
{
	mAcidSignKey = other.mAcidSignKey;	
	mPkg2SignKey = other.mPkg2SignKey;
	mNcaHeader0SignKey = other.mNcaHeader0SignKey;
	mXciHeaderSignKey = other.mXciHeaderSignKey;

	mNcaHeaderKey = other.mNcaHeaderKey;
	mXciHeaderKey = other.mXciHeaderKey;

	for (size_t i = 0; i < kMasterKeyNum; i++)
	{
		mPkg2Key[i] = other.mPkg2Key[i];
		mPkg1Key[i] = other.mPkg1Key[i];
		mNcaKeyAreaEncryptionKey[0][i] = other.mNcaKeyAreaEncryptionKey[0][i];
		mNcaKeyAreaEncryptionKey[1][i] = other.mNcaKeyAreaEncryptionKey[1][i];
		mNcaKeyAreaEncryptionKey[2][i] = other.mNcaKeyAreaEncryptionKey[2][i];
		mNcaKeyAreaEncryptionKeyHw[0][i] = other.mNcaKeyAreaEncryptionKeyHw[0][i];
		mNcaKeyAreaEncryptionKeyHw[1][i] = other.mNcaKeyAreaEncryptionKeyHw[1][i];
		mNcaKeyAreaEncryptionKeyHw[2][i] = other.mNcaKeyAreaEncryptionKeyHw[2][i];
		mETicketCommonKey[i] = other.mETicketCommonKey[i];
	}

	mPkiRootKeyList = other.mPkiRootKeyList;
	
	mNcaExternalContentKeyList = other.mNcaExternalContentKeyList;
}

void KeyConfiguration::importHactoolGenericKeyfile(const std::string& path)
{
	clearGeneralKeyConfiguration();

	fnd::ResourceFileReader res;
	try
	{
		res.processFile(path);
	}
	catch (const fnd::Exception&)
	{
		throw fnd::Exception(kModuleName, "Failed to open key file: " + path);
	}

	// internally used sources
	fnd::aes::sAes128Key master_key[kMasterKeyNum] = { kNullAesKey };
	fnd::aes::sAes128Key package2_key_source = kNullAesKey;
	fnd::aes::sAes128Key ticket_titlekek_source = kNullAesKey;
	fnd::aes::sAes128Key key_area_key_source[kNcaKeakNum] = { kNullAesKey, kNullAesKey, kNullAesKey };
	fnd::aes::sAes128Key aes_kek_generation_source = kNullAesKey;
	fnd::aes::sAes128Key aes_key_generation_source = kNullAesKey;
	fnd::aes::sAes128Key nca_header_kek_source = kNullAesKey;
	fnd::aes::sAesXts128Key nca_header_key_source = kNullAesXtsKey;
	fnd::rsa::sRsa4096Key pki_root_sign_key = kNullRsa4096Key;

#define _CONCAT_2_STRINGS(str1, str2) ((str1) + "_" + (str2))
#define _CONCAT_3_STRINGS(str1, str2, str3) _CONCAT_2_STRINGS(_CONCAT_2_STRINGS(str1, str2), str3)

	std::string key,val;
	fnd::Vec<byte_t> dec_array;

#define _SAVE_KEYDATA(key_name, array, len) \
	key = (key_name); \
	val = res[key]; \
	if (val.empty() == false) { \
		fnd::SimpleTextOutput::stringToArray(val, dec_array); \
		if (dec_array.size() != len) \
			throw fnd::Exception(kModuleName, "Key: \"" + key_name + "\" has incorrect length"); \
		memcpy(array, dec_array.data(), len); \
	}

	for (size_t nameidx = 0; nameidx < kNameVariantNum; nameidx++)
	{
		// import sources
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kPkg2Base[nameidx], kKeyStr, kSourceStr), package2_key_source.key, 0x10);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kTicketCommonKeyBase[nameidx], kSourceStr), ticket_titlekek_source.key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyBase[nameidx], kNcaKeyAreaKeyIndexStr[0], kSourceStr), key_area_key_source[0].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyBase[nameidx], kNcaKeyAreaKeyIndexStr[1], kSourceStr), key_area_key_source[1].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyBase[nameidx], kNcaKeyAreaKeyIndexStr[2], kSourceStr), key_area_key_source[2].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kKekGenBase[nameidx], kSourceStr), aes_kek_generation_source.key, 0x10);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kKeyGenBase[nameidx], kSourceStr), aes_key_generation_source.key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kXciHeaderBase[nameidx], kKekStr, kSourceStr), nca_header_kek_source.key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kXciHeaderBase[nameidx], kKeyStr, kSourceStr), nca_header_key_source.key, 0x20);

		// Store Key Variants/Derivatives
		for (size_t mkeyidx = 0; mkeyidx < kMasterKeyNum; mkeyidx++)
		{
			
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kMasterBase[nameidx], kKeyStr, kKeyIndex[mkeyidx]), master_key[mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kPkg1Base[nameidx], kKeyStr, kKeyIndex[mkeyidx]), mPkg1Key[mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kPkg2Base[nameidx], kKeyStr, kKeyIndex[mkeyidx]), mPkg2Key[mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_2_STRINGS(kTicketCommonKeyBase[nameidx], kKeyIndex[mkeyidx]), mETicketCommonKey[mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyBase[nameidx], kNcaKeyAreaKeyIndexStr[0], kKeyIndex[mkeyidx]), mNcaKeyAreaEncryptionKey[0][mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyBase[nameidx], kNcaKeyAreaKeyIndexStr[1], kKeyIndex[mkeyidx]), mNcaKeyAreaEncryptionKey[1][mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyBase[nameidx], kNcaKeyAreaKeyIndexStr[2], kKeyIndex[mkeyidx]), mNcaKeyAreaEncryptionKey[2][mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyHwBase[nameidx], kNcaKeyAreaKeyIndexStr[0], kKeyIndex[mkeyidx]), mNcaKeyAreaEncryptionKeyHw[0][mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyHwBase[nameidx], kNcaKeyAreaKeyIndexStr[1], kKeyIndex[mkeyidx]), mNcaKeyAreaEncryptionKeyHw[1][mkeyidx].key, 0x10);
			_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaKeyAreaEncKeyHwBase[nameidx], kNcaKeyAreaKeyIndexStr[2], kKeyIndex[mkeyidx]), mNcaKeyAreaEncryptionKeyHw[2][mkeyidx].key, 0x10);
		}
		
		// store nca header key
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[nameidx], kKeyStr), mNcaHeaderKey.key[0], 0x20);

		// store xci header key
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase[nameidx], kKeyStr), mXciHeaderKey.key, 0x10);

		// store rsa keys
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[nameidx], kRsaKeyPrivate), mNcaHeader0SignKey.priv_exponent, fnd::rsa::kRsa2048Size);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[nameidx], kRsaKeyModulus), mNcaHeader0SignKey.modulus, fnd::rsa::kRsa2048Size);
		
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase[nameidx], kRsaKeyPrivate), mXciHeaderSignKey.priv_exponent, fnd::rsa::kRsa2048Size);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase[nameidx], kRsaKeyModulus), mXciHeaderSignKey.modulus, fnd::rsa::kRsa2048Size);

		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kAcidBase[nameidx], kRsaKeyPrivate), mAcidSignKey.priv_exponent, fnd::rsa::kRsa2048Size);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kAcidBase[nameidx], kRsaKeyModulus), mAcidSignKey.modulus, fnd::rsa::kRsa2048Size);

		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPkg2Base[nameidx], kRsaKeyPrivate), mPkg2SignKey.priv_exponent, fnd::rsa::kRsa2048Size);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPkg2Base[nameidx], kRsaKeyModulus), mPkg2SignKey.modulus, fnd::rsa::kRsa2048Size);

		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPkiRootBase[nameidx], kRsaKeyPrivate), pki_root_sign_key.priv_exponent, fnd::rsa::kRsa4096Size);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPkiRootBase[nameidx], kRsaKeyModulus), pki_root_sign_key.modulus, fnd::rsa::kRsa4096Size);
	}

#undef _SAVE_KEYDATA
#undef _CONCAT_3_STRINGS
#undef _CONCAT_2_STRINGS

	// Derive keys 
	for (size_t i = 0; i < kMasterKeyNum; i++)
	{
		if (master_key[i] != kNullAesKey)
		{
			if (aes_kek_generation_source != kNullAesKey && aes_key_generation_source != kNullAesKey)
			{
				if (i == 0 && nca_header_kek_source != kNullAesKey && nca_header_key_source != kNullAesXtsKey)
				{
					if (mNcaHeaderKey == kNullAesXtsKey)
					{
						fnd::aes::sAes128Key nca_header_kek;
						nn::hac::AesKeygen::generateKey(nca_header_kek.key, aes_kek_generation_source.key, nca_header_kek_source.key, aes_key_generation_source.key, master_key[i].key);
						nn::hac::AesKeygen::generateKey(mNcaHeaderKey.key[0], nca_header_key_source.key[0], nca_header_kek.key);
						nn::hac::AesKeygen::generateKey(mNcaHeaderKey.key[1], nca_header_key_source.key[1], nca_header_kek.key);
					}
				}

				for (size_t j = 0; j < nn::hac::nca::kKeyAreaEncryptionKeyNum; j++)
				{
					if (key_area_key_source[j] != kNullAesKey && mNcaKeyAreaEncryptionKey[j][i] == kNullAesKey)
					{
						nn::hac::AesKeygen::generateKey(mNcaKeyAreaEncryptionKey[j][i].key, aes_kek_generation_source.key, key_area_key_source[j].key, aes_key_generation_source.key, master_key[i].key);
					}
				}
			}

			if (ticket_titlekek_source != kNullAesKey && mETicketCommonKey[i] == kNullAesKey)
			{
				nn::hac::AesKeygen::generateKey(mETicketCommonKey[i].key, ticket_titlekek_source.key, master_key[i].key);
			}
			if (package2_key_source != kNullAesKey && mPkg2Key[i] == kNullAesKey)
			{
				nn::hac::AesKeygen::generateKey(mPkg2Key[i].key, package2_key_source.key, master_key[i].key);
			}
		}
	}

	// populate pki root keys
	if (pki_root_sign_key != kNullRsa4096Key)
	{
		sPkiRootKey tmp;

		tmp.name = nn::pki::sign::kRootIssuerStr;
		tmp.key_type = nn::pki::sign::SIGN_ALGO_RSA4096;
		tmp.rsa4096_key = pki_root_sign_key;

		mPkiRootKeyList.addElement(tmp);
	}
}


void KeyConfiguration::clearGeneralKeyConfiguration()
{
	mAcidSignKey = kNullRsa2048Key;
	mPkg2SignKey = kNullRsa2048Key;
	mNcaHeader0SignKey = kNullRsa2048Key;
	mXciHeaderSignKey = kNullRsa2048Key;
	mPkiRootKeyList.clear();

	mNcaHeaderKey = kNullAesXtsKey;
	mXciHeaderKey = kNullAesKey;

	for (size_t i = 0; i < kMasterKeyNum; i++)
	{
		mPkg1Key[i] = kNullAesKey;
		mPkg2Key[i] = kNullAesKey;
		mETicketCommonKey[i] = kNullAesKey;
		for (size_t j = 0; j < kNcaKeakNum; j++)
		{
			mNcaKeyAreaEncryptionKey[j][i] = kNullAesKey;
			mNcaKeyAreaEncryptionKey[j][i] = kNullAesKey;
		}
	}
}

void KeyConfiguration::clearNcaExternalKeys()
{
	mNcaExternalContentKeyList.clear();
}

bool KeyConfiguration::getNcaHeaderKey(fnd::aes::sAesXts128Key& key) const
{
	return copyOutKeyResourceIfExists(mNcaHeaderKey, key, kNullAesXtsKey);
}

bool KeyConfiguration::getNcaHeader0SignKey(fnd::rsa::sRsa2048Key& key) const
{
	return copyOutKeyResourceIfExists(mNcaHeader0SignKey, key, kNullRsa2048Key);
}

bool KeyConfiguration::getAcidSignKey(fnd::rsa::sRsa2048Key& key) const
{
	return copyOutKeyResourceIfExists(mAcidSignKey, key, kNullRsa2048Key);
}

bool KeyConfiguration::getNcaKeyAreaEncryptionKey(byte_t masterkey_index, byte_t keak_type, fnd::aes::sAes128Key& key) const
{
	if (keak_type >= kNcaKeakNum || masterkey_index >= kMasterKeyNum)
	{
		return false;
	}
	return copyOutKeyResourceIfExists(mNcaKeyAreaEncryptionKey[keak_type][masterkey_index], key, kNullAesKey);
}

bool KeyConfiguration::getNcaKeyAreaEncryptionKeyHw(byte_t masterkey_index, byte_t keak_type, fnd::aes::sAes128Key& key) const
{
	if (keak_type >= kNcaKeakNum || masterkey_index >= kMasterKeyNum)
	{
		return false;
	}
	return copyOutKeyResourceIfExists(mNcaKeyAreaEncryptionKeyHw[keak_type][masterkey_index], key, kNullAesKey);
}

void KeyConfiguration::addNcaExternalContentKey(const byte_t rights_id[nn::hac::nca::kRightsIdLen], const fnd::aes::sAes128Key& key)
{
	sNcaExternalContentKey tmp;
	memcpy(tmp.rights_id.data, rights_id, nn::hac::nca::kRightsIdLen);
	tmp.key = key;

	if (mNcaExternalContentKeyList.hasElement(tmp))
		return;

	mNcaExternalContentKeyList.addElement(tmp);
}

bool KeyConfiguration::getNcaExternalContentKey(const byte_t rights_id[nn::hac::nca::kRightsIdLen], fnd::aes::sAes128Key& key) const
{
	sRightsId id;
	bool res_exists = false;

	memcpy(id.data, rights_id, nn::hac::nca::kRightsIdLen);
	for (size_t i = 0; i < mNcaExternalContentKeyList.size(); i++)
	{
		if (mNcaExternalContentKeyList[i].rights_id == id)
		{
			res_exists = true;
			key = mNcaExternalContentKeyList[i].key;
			break;
		}
	}

	return res_exists;
}

bool KeyConfiguration::getPkg1Key(byte_t masterkey_index, fnd::aes::sAes128Key& key) const
{
	if (masterkey_index >= kMasterKeyNum)
	{
		return false;
	}
	return copyOutKeyResourceIfExists(mPkg1Key[masterkey_index], key, kNullAesKey);
}

bool KeyConfiguration::getPkg2Key(byte_t masterkey_index, fnd::aes::sAes128Key& key) const
{
	if (masterkey_index >= kMasterKeyNum)
	{
		return false;
	}
	return copyOutKeyResourceIfExists(mPkg2Key[masterkey_index], key, kNullAesKey);
}

bool KeyConfiguration::getPkg2SignKey(fnd::rsa::sRsa2048Key& key) const
{
	return copyOutKeyResourceIfExists(mPkg2SignKey, key, kNullRsa2048Key);
}

bool KeyConfiguration::getXciHeaderSignKey(fnd::rsa::sRsa2048Key& key) const
{
	return copyOutKeyResourceIfExists(mXciHeaderSignKey, key, kNullRsa2048Key);
}

bool KeyConfiguration::getXciHeaderKey(fnd::aes::sAes128Key& key) const
{
	return copyOutKeyResourceIfExists(mXciHeaderKey, key, kNullAesKey);
}

bool KeyConfiguration::getETicketCommonKey(byte_t masterkey_index, fnd::aes::sAes128Key& key) const
{
	if (masterkey_index >= kMasterKeyNum)
	{
		return false;
	}
	return copyOutKeyResourceIfExists(mETicketCommonKey[masterkey_index], key, kNullAesKey);
}

bool KeyConfiguration::getPkiRootSignKey(const std::string& root_name, fnd::rsa::sRsa4096Key& key) const
{
	bool res_exists = false;
	for (size_t i = 0; i < mPkiRootKeyList.size(); i++)
	{
		if (root_name == mPkiRootKeyList[i].name && mPkiRootKeyList[i].key_type == nn::pki::sign::SIGN_ALGO_RSA4096)
		{
			res_exists = true;
			key = mPkiRootKeyList[i].rsa4096_key;
			break;
		}
	}

	return res_exists;
}

bool KeyConfiguration::getPkiRootSignKey(const std::string& root_name, fnd::rsa::sRsa2048Key& key) const
{
	bool res_exists = false;
	for (size_t i = 0; i < mPkiRootKeyList.size(); i++)
	{
		if (root_name == mPkiRootKeyList[i].name && mPkiRootKeyList[i].key_type == nn::pki::sign::SIGN_ALGO_RSA2048)
		{
			res_exists = true;
			key = mPkiRootKeyList[i].rsa2048_key;
			break;
		}
	}

	return res_exists;
}

bool KeyConfiguration::getPkiRootSignKey(const std::string& root_name, fnd::ecdsa::sEcdsa240Key& key) const
{
	bool res_exists = false;
	for (size_t i = 0; i < mPkiRootKeyList.size(); i++)
	{
		if (root_name == mPkiRootKeyList[i].name && mPkiRootKeyList[i].key_type == nn::pki::sign::SIGN_ALGO_ECDSA240)
		{
			res_exists = true;
			key = mPkiRootKeyList[i].ecdsa240_key;
			break;
		}
	}

	return res_exists;
}