#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>
#include <fnd/rsa.h>
#include <nn/ctr/macro.h>

namespace nn
{
namespace ctr
{
	namespace ncch
	{
		static const uint32_t kNcchStructMagic = _MAKE_STRUCT_MAGIC_U32("NCCH");
		static const size_t kCompanyCodeLen = 2;
		static const size_t kProductCodeLen = 0x10;
		static const byte_t kFormTypeBitWidth = 2;
		static const byte_t kContentTypeBitWidth = 6;

		enum NcchType
		{
			TYPE_CFA = 0,
			TYPE_CXI = 2
		};

		enum NewCryptoFlag
		{
			CRYPTO_DEFAULT = 0x00,
			CRYPTO_25 = 0x01,
			CRYPTO_18 = 0x0A,
			CRYPTO_1B = 0x0B
		};

		enum Platform
		{
			PLATFORM_UNDEFINED,
			PLATFORM_CTR,
			PLATFORM_SNAKE,
		};

		enum FormType
		{
			UNASSIGNED,
			SIMPLE_CONTENT,
			EXECUTABLE_WITHOUT_ROMFS,
			EXECUTABLE
		};

		enum ContentType
		{
			APPLICATION,
			SYSTEM_UPDATE,
			MANUAL,
			CHILD,
			TRIAL,
			EXTENDED_SYSTEM_UPDATE
		};

		enum BlockSizeFlag
		{
			BLOCK_SIZE_512
		};

		enum OtherFlag
		{
			FIXED_AES_KEY = 0,
			NO_MOUNT_ROMFS = 1,
			NO_AES = 2,
			SEED_KEY = 5,
			MANUAL_DISCLOSURE = 6,
		};

		enum FlagIndex
		{
			CRYPTO_FLAG_INDEX = 3,
			PLATFORM_INDEX = 4,
			CONTENT_TYPE_INDEX = 5,
			BLOCK_SIZE_INDEX = 6,
			OTHER_FLAG_INDEX = 7
		};
	}

#pragma pack(push, 1)
	struct sLayout
	{
		le_uint32_t block_offset;
		le_uint32_t block_num;
	};

	struct sLayoutHashed : public sLayout
	{
		le_uint32_t block_num_hash_protected;
	};

	struct sNcchHeader
	{
		le_uint32_t st_magic;
		le_uint32_t size;
		le_uint64_t title_id;
		char company_code[ncch::kCompanyCodeLen];
		le_uint16_t ncch_type;
		le_uint32_t seed_checksum;
		le_uint64_t program_id;
		byte_t reserved1[0x10];
		fnd::sha::sSha256Hash logo_hash;
		char product_code[ncch::kProductCodeLen];
		fnd::sha::sSha256Hash exheader_hash;
		le_uint32_t exheader_size;
		byte_t reserved2[0x4];
		byte_t flags[8];
		sLayout plain_region;
		sLayout logo;
		sLayoutHashed exefs;
		byte_t reserved3[4];
		sLayoutHashed romfs;
		byte_t reserved4[4];
		fnd::sha::sSha256Hash exefs_hash;
		fnd::sha::sSha256Hash romfs_hash;
	};

	struct sNcchHeaderBlock
	{
		byte_t rsa_signature[fnd::rsa::kRsa2048Size];
		sNcchHeader header;
	};
#pragma pack(pop)
}
}