#pragma once
#include <fnd/types.h>

namespace nn
{
namespace hac
{
	namespace fac
	{
		static const uint32_t kFacFormatVersion = 1;
		static const size_t kSectionAlignSize = 4;
		
		enum FsAccessFlag
		{
			FSA_APPLICATION_INFO,
			FSA_BOOT_MODE_CONTROL,
			FSA_CALIBRATION,
			FSA_SYSTEM_SAVE_DATA,
			FSA_GAME_CARD,
			FSA_SAVE_DATA_BACKUP,
			FSA_SAVE_DATA_MANAGEMENT,
			FSA_BIS_ALL_RAW,
			FSA_GAME_CARD_RAW,
			FSA_GAME_CARD_PRIVATE,
			FSA_SET_TIME,
			FSA_CONTENT_MANAGER,
			FSA_IMAGE_MANAGER,
			FSA_CREATE_SAVE_DATA,
			FSA_SYSTEM_SAVE_DATA_MANAGEMENT,
			FSA_BIS_FILE_SYSTEM,
			FSA_SYSTEM_UPDATE,
			FSA_SAVE_DATA_META,
			FSA_DEVICE_SAVE_CONTROL,
			FSA_SETTINGS_CONTROL,
			FSA_DEBUG = 62,
			FSA_FULL_PERMISSION = 63,
		};

		enum SaveDataOwnerIdAccessType
		{
			SDO_READ = 1,
			SDO_WRITE,
			SDO_READWRITE
		};
	}

#pragma pack(push,1)
	struct sFacHeader
	{
		le_uint32_t version; // default 1
		le_uint64_t fac_flags;
		struct sFacSection
		{
			le_uint32_t offset;
			le_uint32_t size;
		} content_owner_ids, save_data_owner_ids; // the data for these follow later in binary. start/end relative to base of FacData instance
	};
#pragma pack(pop)
}
}