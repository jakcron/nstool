#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>
#include <fnd/rsa.h>
#include <nn/ctr/macro.h>

namespace nn
{
namespace ctr
{
	namespace smdh
	{
		static const uint32_t kSmdhStructMagic = _MAKE_STRUCT_MAGIC_U32("SMDH");
		static const uint16_t kDefaultFormatVersion = 0;
		static const size_t kShortTitleLength = 0x40;
		static const size_t kLongTitleLength = 0x80;
		static const size_t kPublisherLength = 0x40;
		static const size_t kApplicationTitleNum = 0x10;
		static const size_t kOrganisationNum = 0x10;
		static const size_t kSmallIconLength = 0x480;
		static const size_t kLargeIconLength = 0x1200;
		
		enum Language
		{
			LANG_Japanese,
			LANG_English,
			LANG_French,
			LANG_German,
			LANG_Italian,
			LANG_Spanish,
			LANG_SimplifiedChinese,
			LANG_Korean,
			LANG_Dutch,
			LANG_Portuguese,
			LANG_Russian,
			LANG_TraditionalChinese
		};

		enum Organisation
		{
			ORGN_CERO = 0,
			ORGN_ESRB = 1,
			ORGN_USK = 3,
			ORGN_PEGI = 4,
			ORGN_PEGIPortugal = 6,
			ORGN_PEGIBBFC = 7,
			ORGN_ACB = 8, // COB previously
			ORGN_GRACGCRB = 9,
			ORGN_GSRMR = 10,
		};

		enum AgeRatingFlag
		{
			RATING_NoRestriction = 5,
			RATING_Pending = 6,
			RATING_Enabled = 7
		};

		enum RegionRestrict
		{
			REGION_Japan,
			REGION_USA,
			REGION_Europe,
			REGION_Australia,
			REGION_China,
			REGION_Korea,
			REGION_Taiwan
		};

		enum Flags
		{
			FLAG_ShowInHomeMenu,
			FLAG_AutoBoot,
			FLAG_UsesStereoDisplay,
			FLAG_RequireAcceptEULA,
			FLAG_WillAutoSave,
			FLAG_UsesExtendedBanner,
			FLAG_UsesAgeRating,
			FLAG_UsesSaveData,
			FLAG_CacheIconMetadata,
			FLAG_DisableSaveDataBackup,
			FLAG_SnakeExclusiveTitle
		};
	}

#pragma pack(push, 1)
	struct sApplicationTitle
	{
		le_uint16_t short_name[smdh::kShortTitleLength];
		le_uint16_t long_name[smdh::kLongTitleLength];
		le_uint16_t publisher[smdh::kPublisherLength];
	};

	struct sSmdh
	{
		le_uint32_t st_magic;
		le_uint16_t format_version;
		byte_t reserved_00[2];
		sApplicationTitle title[smdh::kApplicationTitleNum];
		byte_t age_restriction[smdh::kOrganisationNum];
		le_uint32_t region_restriction;
		le_uint32_t match_maker_id;
		le_uint64_t match_maker_bitmask_id;
		le_uint32_t flags;
		byte_t reserved_01[2];
		le_uint32_t banner_default_animation_frame; // 32bit float
		le_uint32_t cec_id;
		byte_t small_icon[smdh::kSmallIconLength];
		byte_t large_icon[smdh::kLargeIconLength];
	};
	
#pragma pack(pop)
}
}