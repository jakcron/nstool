#pragma once
#include <fnd/types.h>
#include <fnd/aes.h>
#include <fnd/sha.h>
#include <fnd/rsa.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace nca
	{
		static const uint32_t kNca2StructMagic = _MAKE_STRUCT_MAGIC_U32("NCA2");
		static const uint32_t kNca3StructMagic = _MAKE_STRUCT_MAGIC_U32("NCA3");
		static const size_t kSectorSize = 0x200;
		static const size_t kPartitionNum = 4;
		static const size_t kHeaderSectorNum = 6;
		static const size_t kHeaderSize = kSectorSize * kHeaderSectorNum;
		static const size_t kAesKeyNum = 16;
		static const size_t kRightsIdLen = 0x10;
		static const size_t kKeyAreaEncryptionKeyNum = 3;
		static const size_t kFsHeaderHashSuperblockLen = 0x138;
		static const uint16_t kDefaultFsHeaderVersion = 2;

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
			TYPE_PUBLIC_DATA
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
			HASH_NONE,
			HASH_HIERARCHICAL_SHA256,
			HASH_HIERARCHICAL_INTERGRITY // IVFC
		};

		enum EncryptionType
		{
			CRYPT_AUTO,
			CRYPT_NONE,
			CRYPT_AESXTS,
			CRYPT_AESCTR,
			CRYPT_AESCTREX
		};	
	}
	
#pragma pack(push,1)
	struct sNcaHeader
	{
		le_uint32_t st_magic;
		byte_t distribution_type;
		byte_t content_type;
		byte_t key_generation;
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
		fnd::sha::sSha256Hash partition_hash[nca::kPartitionNum];
		fnd::aes::sAes128Key enc_aes_key[nca::kAesKeyNum];
	};

	struct sNcaFsHeader
	{
		le_uint16_t version;
		byte_t format_type;
		byte_t hash_type;
		byte_t encryption_type;
		byte_t reserved_0[3];
		byte_t hash_superblock[nca::kFsHeaderHashSuperblockLen];
		byte_t aes_ctr_upper[8];
		byte_t reserved_1[0xB8];
	};

	struct sNcaHeaderBlock
	{
		byte_t signature_main[fnd::rsa::kRsa2048Size];
		byte_t signature_acid[fnd::rsa::kRsa2048Size];
		sNcaHeader header;
		sNcaFsHeader fs_header[nn::hac::nca::kPartitionNum];
	};

#pragma pack(pop)
}
}