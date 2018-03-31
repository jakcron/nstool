#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class XciHeader// :
		//public ISerialiseableBinary
	{
	public:
		XciHeader();

	private:
#pragma pack (push, 1)


		enum RomSize
		{
			ROM_SIZE_1GB = 0xFA,
			ROM_SIZE_2GB = 0xF8,
			ROM_SIZE_4GB = 0xF0,
			ROM_SIZE_8GB = 0xE0,
			ROM_SIZE_16GB = 0xE1,
			ROM_SIZE_32GB = 0xE2
		};

		enum ContentMetaType
		{
			SYSTEM_PROGRAM = 1,
			SYSTEM_DATA = 2,
			SYSTEM_UPDATE = 3,
			BOOT_IMAGE_PACKAGE = 4,
			BOOT_IMAGE_PACKAGE_SAFE = 5,
			APPLICATION = 128,
			PATCH = 129,
			ADD_ON_CONTENT = 130
		};

		enum ContentType
		{
			META,
			PROGRAM,
			DATA,
			CONTROL,
			HTML_DOCUMENT,
			LEGAL_INFORMATION
		};

		enum ContentMetaAttribute
		{
			None = 0,
			IncludesExFatDriver = 1
		};

		struct sContentMetaInfo
		{
			uint64_t id;
			uint32_t version;
			byte_t type; // ContentMetaType
			byte_t attributes;
			byte_t reserved[2];
		};

		struct sContentInfo
		{
			byte_t id[16];
			uint32_t size_low;
			uint16_t size_high;
			byte_t type;
			byte_t reserved;
		};

		/*
		
	public const int InitialDataSize = 512;

	public const int InitialDataASize = 16;

	public const int InitialDataMacSize = 16;

	public const int InitialDataNonceSize = 12;
	*/
		struct sInitialData
		{
			byte_t key_source[16]; // { package_id[8], zeros[8]}
			byte_t title_key_enc[16];
			byte_t ccm_mac[16];
			byte_t ccm_nonce[12];
		};

		/*
			decrypt key_source
		*/

		struct sXciHeader
		{
			char signature[4]; // 0x00 // "HEAD"
			le_uint32_t rom_area_start_page; // 0x04
			le_uint32_t backup_area_start_page; // 0x08
			byte_t key_flag; // 0x0C // bit0-3 = KekIndex, bit4-7 = TitleKeyDecIndex
			byte_t rom_size; // 0x0D // this is an enum
			byte_t card_header_version; // 0x0E // CardHeaderVersion
			byte_t flags; // 0x0F
			byte_t package_id[8]; // 0x10 // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
			le_uint32_t valid_data_end_page; // 0x18
			byte_t reserved_01[4]; // 0x1C
			byte_t encryption_iv[16]; // 0x20
			le_uint64_t partition_fs_header_address; // 0x30
			le_uint64_t partition_fs_header_size; // 0x38
			byte_t partition_fs_header_hash[0x20]; // 0x40
			byte_t initial_data_hash[0x20]; // 0x60
			le_uint32_t sel_sec; // 0x80
			le_uint32_t sel_t1_key; // 0x84 // SelT1Key
			le_uint32_t sel_key; // 0x88 // SelKey
			le_uint32_t lim_area; // 0x8C
			// START ENCRYPTION
			le_uint32_t fw_version[2]; // 0x90 // [0]=minor, [1]=major
			le_uint32_t acc_ctrl_1; // 0x98
			le_uint32_t wait_1_time_read; // 0x9C // Wait1TimeRead
			le_uint32_t wait_2_time_read; // 0xA0 // Wait2TimeRead
			le_uint32_t wait_1_time_write; // 0xA4 // Wait1TimeWrite
			le_uint32_t wait_2_time_write; // 0xA8 // Wait2TimeWrite
			le_uint32_t fw_mode; // 0xAC
			le_uint32_t cup_version; // 0xB0
			byte_t reserved_03[0x4]; // 0xB4
			byte_t upp_hash[8]; // 0xB8 // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
			le_uint64_t cup_id; // 0xC0 // cup programID?
			byte_t reserved_04[0x38];
			// END ENCRYPTION
		};
#pragma pack (pop)
	};

}
