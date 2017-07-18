#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/memory_blob.h>
#include <fnd/List.h>
#include <nx/ISerialiseableBinary.h>

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
			u64 id;
			u32 version;
			u8 type; // ContentMetaType
			u8 attributes;
			u8 reserved[2];
		};

		struct sContentInfo
		{
			u8 id[16];
			u32 size_low;
			u16 size_high;
			u8 type;
			u8 reserved;
		};

		struct sXciHeader
		{
			u8 signature[4];
			u32 rom_area_start_page;
			u32 backup_area_start_page;
			u8 key_flag; // bit0-3 = KekIndex, bit4-7 = TitleKeyDecIndex
			u8 rom_size; // this is an enum
			u8 flags;
			u8 package_id[8]; // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
			u32 valid_data_end_page;
			u8 reserved_0[100];
			u32 sel_sec;
			u32 sel_t1_key;
			u32 sel_key;
			u32 lim_area;
			u32 fw_version[2]; // [0]=minor, [1]=major
			u32 acc_ctrl_1;
			u8 reserved_1[0x10];
			u32 fw_mode;
			u32 cup_version;
			u8 reserved_2[0x4];
			u8 upp_hash[8]; // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
			u64 cup_id; // cup programID?

		};
#pragma pack (pop)
	};

}
