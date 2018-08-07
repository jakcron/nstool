#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/aes.h>
#include <fnd/sha.h>
#include <fnd/rsa.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace xci
	{
		static const uint32_t kXciStructMagic = _MAKE_STRUCT_MAGIC_U32("HEAD");
		static const uint32_t kHeaderEncOffset = 0x90;
		static const uint32_t kHeaderEncSize = 0x70;
		static const uint32_t kPageSize = 0x200;
		static const uint32_t kUppHashLen = 8;
		/*
		static const uint32_t kCardKeyAreaPageCount = 8;
		static const uint32_t kCardHeaderPageCount = 1;
		static const uint32_t kReservedAreaPageCount = 55;
		static const uint32_t kCertAreaStartPageAddress = kCardHeaderPageCount + kReservedAreaPageCount + kCardKeyAreaPageCount;
		static const uint32_t kCertAreaPageCount = 64;
		static const uint32_t kNormalAreaStartPageAddress = kReservedAreaPageCount + kCertAreaPageCount + kCardHeaderPageCount + kCardKeyAreaPageCount;
		*/

		const std::string kUpdatePartitionStr = "update"; 
		const std::string kLogoPartitionStr = "logo"; 
		const std::string kNormalPartitionStr = "normal"; 
		const std::string kSecurePartitionStr = "secure"; 

		enum KekIndex
		{
			KEK_XCIE,
			KEK_XCIR
		};

		enum RomSize
		{
			ROM_SIZE_1GB = 0xFA,
			ROM_SIZE_2GB = 0xF8,
			ROM_SIZE_4GB = 0xF0,
			ROM_SIZE_8GB = 0xE0,
			ROM_SIZE_16GB = 0xE1,
			ROM_SIZE_32GB = 0xE2
		};

		enum HeaderFlags
		{
			FLAG_AUTOBOOT,
			FLAG_HISTORY_ERASE,
			FLAG_REPAIR_TOOL
		};

		enum FwVersionIndex
		{
			FWVER_MINOR,
			FWVER_MAJOR
		};

		enum CardClockRate
		{
			CLOCK_RATE_25 = 10551312,
			CLOCK_RATE_50 = 10551313,
		};
	}
	
#pragma pack(push,1)
	struct sXciHeader
	{
		le_uint32_t st_magic;
		le_uint32_t rom_area_start_page;
		le_uint32_t backup_area_start_page;
		byte_t key_flag;
		byte_t rom_size;
		byte_t card_header_version;
		byte_t flags;
		le_uint64_t package_id;
		le_uint32_t valid_data_end_page;
		byte_t reserved_00[4];
		fnd::aes::sAesIvCtr aescbc_iv;
		le_uint64_t partition_fs_header_address;
		le_uint64_t partition_fs_header_size;
		fnd::sha::sSha256Hash partition_fs_header_hash;
		fnd::sha::sSha256Hash initial_data_hash;
		le_uint32_t sel_sec;
		le_uint32_t sel_t1_key;
		le_uint32_t sel_key;
		le_uint32_t lim_area;
		// START ENCRYPTION
		le_uint32_t fw_version[2];
		le_uint32_t acc_ctrl_1;
		le_uint32_t wait_1_time_read;
		le_uint32_t wait_2_time_read;
		le_uint32_t wait_1_time_write;
		le_uint32_t wait_2_time_write;
		le_uint32_t fw_mode;
		le_uint32_t upp_version;
		byte_t reserved_01[0x4];
		byte_t upp_hash[xci::kUppHashLen];
		le_uint64_t upp_id;
		byte_t reserved_02[0x38];
		// END ENCRYPTION
	};

	struct sXciHeaderPage
	{
		byte_t signature[fnd::rsa::kRsa2048Size];
		sXciHeader header;
	}; // sizeof() = 512 (1 page)

	struct sInitialData
	{
		byte_t key_source[16]; // { package_id[8], zeros[8]}
		byte_t title_key_enc[16];
		byte_t ccm_mac[16];
		byte_t ccm_nonce[12];
		byte_t reserved[0x1c4];
	}; // sizeof() = 512 (1 page)

	struct sKeyDataArea
	{
		sInitialData initial_data; // AES128-CCM encrypted {titlekey[16]}
		byte_t encrypted_00[xci::kPageSize * 6]; // AES128-CTR encrypted {titlekey[16]}
		byte_t encrypted_00_aesctr_data[fnd::rsa::kRsa2048Size]; // RSA2048-OAEP-SHA256 encrypted AES-CTR data used for encrypted_00 {key[16],iv[16]}
		byte_t reserved[xci::kPageSize - fnd::rsa::kRsa2048Size];
	}; // sizeof() = 512*8 (8 pages)

#pragma pack(pop)
}
}