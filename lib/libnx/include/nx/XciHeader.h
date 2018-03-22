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

		struct sXciHeader
		{
			byte_t signature[4];
			uint32_t rom_area_start_page;
			uint32_t backup_area_start_page;
			byte_t key_flag; // bit0-3 = KekIndex, bit4-7 = TitleKeyDecIndex
			byte_t rom_size; // this is an enum
			byte_t flags;
			byte_t package_id[8]; // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
			uint32_t valid_data_end_page;
			byte_t reserved_0[100];
			uint32_t sel_sec;
			uint32_t sel_t1_key;
			uint32_t sel_key;
			uint32_t lim_area;
			uint32_t fw_version[2]; // [0]=minor, [1]=major
			uint32_t acc_ctrl_1;
			byte_t reserved_1[0x10];
			uint32_t fw_mode;
			uint32_t cup_version;
			byte_t reserved_2[0x4];
			byte_t upp_hash[8]; // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
			uint64_t cup_id; // cup programID?

		};
#pragma pack (pop)
	};

}
