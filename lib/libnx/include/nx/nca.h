#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>
#include <nx/ivfc.h>

namespace nx
{
	namespace nca
	{
		const std::string kNca2Sig = "NCA2";
		const std::string kNca3Sig = "NCA3";
		static const size_t kSectorSize = 0x200;
		static const size_t kPartitionNum = 4;
		static const size_t kHeaderSectorNum = 6;
		static const size_t kHeaderSize = kSectorSize * kHeaderSectorNum;
		static const size_t kAesKeyNum = 16;
		static const size_t kRightsIdLen = 0x10;
		static const size_t kKeyAreaEncryptionKeyNum = 3;

		enum ProgramPartitionId
		{
			PARTITION_CODE,
			PARTITION_DATA,
			PARTITION_LOGO,
		};

		enum DistributionType
		{
			DIST_DOWNLOAD,
			DIST_GAME_CARD
		};

		enum ContentType
		{
			TYPE_PROGRAM,
			TYPE_META,
			TYPE_CONTROL,
			TYPE_MANUAL,
			TYPE_DATA,
		};

		enum KeyBankIndex
		{
			KEY_AESXTS_0,
			KEY_AESXTS_1,
			KEY_AESCTR,
			KEY_UNUSED_3,
			KEY_AESCTR_HW
		};

		enum KeyAreaEncryptionKeyIndex
		{
			KAEK_IDX_APPLICATION,
			KAEK_IDX_OCEAN,
			KAEK_IDX_SYSTEM
		};

		enum FormatType	
		{
			FORMAT_ROMFS,
			FORMAT_PFS0
		};

		enum HashType
		{
			HASH_AUTO,
			HASH_UNK1,
			HASH_HIERARCHICAL_SHA256,
			HASH_HIERARCHICAL_INTERGRITY // IVFC
		};

		enum EncryptionType
		{
			CRYPT_AUTO,
			CRYPT_NONE,
			CRYPT_AESXTS,
			CRYPT_AESCTR,
			CRYPT_BKTR
		};	
	}
	
#pragma pack(push,1)
	struct sNcaHeader
	{
		char signature[4];
		byte_t distribution_type;
		byte_t content_type;
		byte_t key_generation; // KeyGeneration
		byte_t key_area_encryption_key_index;
		le_uint64_t content_size;
		le_uint64_t program_id;
		le_uint32_t content_index;
		le_uint32_t sdk_addon_version;
		byte_t key_generation_2;
		byte_t reserved_2[0xf];
		byte_t rights_id[nca::kRightsIdLen];
		struct sNcaSection
		{
			le_uint32_t start; // block units
			le_uint32_t end; // block units
			byte_t enabled;
			byte_t reserved[7];
		} partition[nca::kPartitionNum];
		crypto::sha::sSha256Hash partition_hash[nca::kPartitionNum];
		crypto::aes::sAes128Key enc_aes_key[nca::kAesKeyNum];
	};

	struct sNcaFsHeader
	{
		le_uint16_t version; // usually 0x0002
		byte_t format_type; // RomFs(0x00), PartitionFs(0x01)
		byte_t hash_type; // HashTypeAuto(0x00), HashTypeHierarchicalSha256(0x02), HashTypeHierarchicalIntegrity(0x03).RomFs uses (0x03) this is forced, PartitionFs uses (0x02).
		byte_t encryption_type; // EncryptionTypeAuto(0x00), EncryptionTypeNone(0x01), EncryptionTypeAesCtr(0x03)
		byte_t reserved[3];
	};

	struct sHierarchicalSha256Header
	{
		byte_t master_hash[0x20];
		le_uint32_t hash_block_size;
		le_uint32_t unk_0x02;
		struct sLayout
		{
			le_uint64_t offset;
			le_uint64_t size;
		} hash_data, hash_target;
	};

#pragma pack(pop)
}
