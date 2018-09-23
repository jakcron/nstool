#pragma once
#include <string>
#include <cstring>
#include <fnd/types.h>
#include <fnd/aes.h>
#include <fnd/rsa.h>
#include <fnd/ecdsa.h>
#include <nn/hac/nca.h>
#include <nn/pki/SignedData.h>
#include <nn/es/TicketBody_V2.h>

class KeyConfiguration
{
public:
	KeyConfiguration();
	KeyConfiguration(const KeyConfiguration& other);

	void operator=(const KeyConfiguration& other);

	void importHactoolGenericKeyfile(const std::string& path);
	//void importHactoolTitleKeyfile(const std::string& path);
	
	void clearGeneralKeyConfiguration();
	void clearNcaExternalKeys();

	// nca keys
	bool getNcaHeaderKey(fnd::aes::sAesXts128Key& key) const;
	bool getNcaHeader0SignKey(fnd::rsa::sRsa2048Key& key) const;
	bool getAcidSignKey(fnd::rsa::sRsa2048Key& key) const;
	bool getNcaKeyAreaEncryptionKey(byte_t masterkey_index, byte_t keak_type, fnd::aes::sAes128Key& key) const;
	bool getNcaKeyAreaEncryptionKeyHw(byte_t masterkey_index, byte_t keak_type, fnd::aes::sAes128Key& key) const;
	
	// external content keys
	void addNcaExternalContentKey(const byte_t rights_id[nn::hac::nca::kRightsIdLen], const fnd::aes::sAes128Key& key);
	bool getNcaExternalContentKey(const byte_t rights_id[nn::hac::nca::kRightsIdLen], fnd::aes::sAes128Key& key) const;

	// pkg1/pkg2
	bool getPkg1Key(byte_t masterkey_index, fnd::aes::sAes128Key& key) const;
	bool getPkg2Key(byte_t masterkey_index, fnd::aes::sAes128Key& key) const;
	bool getPkg2SignKey(fnd::rsa::sRsa2048Key& key) const;

	// xci keys
	bool getXciHeaderSignKey(fnd::rsa::sRsa2048Key& key) const;
	bool getXciHeaderKey(fnd::aes::sAes128Key& key) const;

	// ticket
	bool getETicketCommonKey(byte_t masterkey_index, fnd::aes::sAes128Key& key) const;

	// pki
	bool getPkiRootSignKey(const std::string& root_name, fnd::rsa::sRsa4096Key& key) const;
	bool getPkiRootSignKey(const std::string& root_name, fnd::rsa::sRsa2048Key& key) const;
	bool getPkiRootSignKey(const std::string& root_name, fnd::ecdsa::sEcdsa240Key& key) const;
private:
	const std::string kModuleName = "KeyConfiguration";
	const fnd::aes::sAes128Key kNullAesKey = {{0}};
	const fnd::aes::sAesXts128Key kNullAesXtsKey = {{{0}}};
	const fnd::rsa::sRsa4096Key kNullRsa4096Key = {{0}, {0}, {0}};
	const fnd::rsa::sRsa2048Key kNullRsa2048Key = {{0}, {0}, {0}};
	static const size_t kMasterKeyNum = 0x20;
	static const size_t kNcaKeakNum = nn::hac::nca::kKeyAreaEncryptionKeyNum;

	// keynames
	enum NameVariantIndex
	{
		NNTOOLS,
		LEGACY_HACTOOL,
		LEGACY_0
	};
	static const size_t kNameVariantNum = 3;
	const std::string kMasterBase[kNameVariantNum] = { "master", "master", "master" };
	const std::string kPkg1Base[kNameVariantNum] = { "package1", "package1", "package1" };
	const std::string kPkg2Base[kNameVariantNum] = { "package2", "package2", "package2" };
	const std::string kXciHeaderBase[kNameVariantNum] = { "xci_header", "xci_header", "xci_header" };
	const std::string kNcaHeaderBase[kNameVariantNum] = { "nca_header", "header", "nca_header" };
	const std::string kAcidBase[kNameVariantNum] = { "acid", "acid", "acid" };
	const std::string kPkiRootBase[kNameVariantNum] = { "pki_root", "pki_root", "pki_root" };
	const std::string kTicketCommonKeyBase[kNameVariantNum] = { "ticket_commonkey", "titlekek", "ticket_commonkey" };
	const std::string kNcaKeyAreaEncKeyBase[kNameVariantNum] = { "nca_key_area_key", "key_area_key", "nca_body_keak" };
	const std::string kNcaKeyAreaEncKeyHwBase[kNameVariantNum] = { "nca_key_area_key_hw", "key_area_hw_key", "nca_key_area_key_hw" };
	const std::string kKekGenBase[kNameVariantNum] = { "aes_kek_generation", "aes_kek_generation", "aes_kek_generation" };
	const std::string kKeyGenBase[kNameVariantNum] = { "aes_key_generation", "aes_key_generation", "aes_key_generation" };

	// misc str
	const std::string kKeyStr = "key";
	const std::string kKekStr = "kek";
	const std::string kSourceStr = "source";
	const std::string kRsaKeyModulus = "sign_key_modulus";
	const std::string kRsaKeyPrivate = "sign_key_private";
	const std::string kNcaKeyAreaKeyIndexStr[kNcaKeakNum] = { "application", "ocean", "system" };	
	const std::string kKeyIndex[kMasterKeyNum] = {"00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f","10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f"};

	struct sRightsId
	{
		byte_t data[nn::hac::nca::kRightsIdLen];

		void operator=(const sRightsId& other)
		{
			memcpy(this->data, other.data, nn::hac::nca::kRightsIdLen);
		}

		bool operator==(const sRightsId& other) const
		{
			return memcmp(this->data, other.data, nn::hac::nca::kRightsIdLen) == 0;
		}

		bool operator!=(const sRightsId& other) const
		{
			return !(operator==(other));
		}
	};

	struct sNcaExternalContentKey
	{
		sRightsId rights_id;
		fnd::aes::sAes128Key key;

		void operator=(const sNcaExternalContentKey& other)
		{
			rights_id = other.rights_id;
			key = other.key;
		}

		bool operator==(const sNcaExternalContentKey& other) const
		{
			return (rights_id == other.rights_id) \
				&& (key == other.key);
		}

		bool operator!=(const sNcaExternalContentKey& other) const
		{
			return !(operator==(other));
		}
	};

	struct sPkiRootKey
	{
		std::string name;
		nn::pki::sign::SignatureAlgo key_type;
		fnd::rsa::sRsa4096Key rsa4096_key;
		fnd::rsa::sRsa2048Key rsa2048_key;
		fnd::ecdsa::sEcdsa240Key ecdsa240_key;

		void operator=(const sPkiRootKey& other)
		{
			name = other.name;
			key_type = other.key_type;
			rsa4096_key = other.rsa4096_key;
			rsa2048_key = other.rsa2048_key;
			ecdsa240_key = other.ecdsa240_key;
		}

		bool operator==(const sPkiRootKey& other) const
		{
			return (name == other.name) \
				&& (key_type == other.key_type) \
				&& (rsa4096_key == other.rsa4096_key) \
				&& (rsa2048_key == other.rsa2048_key) \
				&& (ecdsa240_key == other.ecdsa240_key);
		}

		bool operator!=(const sPkiRootKey& other) const
		{
			return !(operator==(other));
		}
	};
	

	/* general key config */
	// acid
	fnd::rsa::sRsa2048Key mAcidSignKey;

	// pkg1 and pkg2
	fnd::aes::sAes128Key mPkg1Key[kMasterKeyNum];
	fnd::rsa::sRsa2048Key mPkg2SignKey;
	fnd::aes::sAes128Key mPkg2Key[kMasterKeyNum];

	// nca
	fnd::rsa::sRsa2048Key mNcaHeader0SignKey;
	fnd::aes::sAesXts128Key mNcaHeaderKey;
	fnd::aes::sAes128Key mNcaKeyAreaEncryptionKey[kNcaKeakNum][kMasterKeyNum];
	fnd::aes::sAes128Key mNcaKeyAreaEncryptionKeyHw[kNcaKeakNum][kMasterKeyNum];

	// xci
	fnd::rsa::sRsa2048Key mXciHeaderSignKey;
	fnd::aes::sAes128Key mXciHeaderKey;

	// ticket
	fnd::aes::sAes128Key mETicketCommonKey[kMasterKeyNum];

	// pki
	fnd::List<sPkiRootKey> mPkiRootKeyList;

	/* Nca External Keys */
	fnd::List<sNcaExternalContentKey> mNcaExternalContentKeyList;

	template <class T>
	bool copyOutKeyResourceIfExists(const T& src, T& dst, const T& null_sample) const
	{
		bool resource_exists = false;

		if (src != null_sample)
		{
			resource_exists = true;
			dst = src;
		}	

		return resource_exists;
	}
};