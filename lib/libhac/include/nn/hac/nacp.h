#pragma once
#include <fnd/types.h>

namespace nn
{
namespace hac
{
	namespace nacp
	{
		static const size_t kNameLength = 0x200;
		static const size_t kPublisherLength = 0x100;
		static const size_t kMaxLanguageCount = 16;
		static const size_t kIsbnLength = 37;
		static const size_t kRatingAgeCount = 32;
		static const size_t kDisplayVersionLength = 16;
		static const size_t kApplicationErrorCodeCategoryLength = 8;
		static const size_t kLocalCommunicationIdCount = 8;
		static const size_t kBcatPassphraseLength = 65;
		static const size_t kPlayLogQueryableApplicationIdCount = 16;
		static const int8_t kUnusedAgeRating = -1;

		enum AocRegistrationType
		{
			AOC_AllOnLaunch,
			AOC_OnDemand
		};

		enum AttributeFlag
		{
			ATTR_None,
			ATTR_Demo,
			ATTR_RetailInteractiveDisplay
		};

		enum CrashReportMode
		{
			CREP_Deny,
			CREP_Allow
		};

		enum DataLossConfirmation
		{
			DLOSS_None,
			DLOSS_Required
		};

		enum Hdcp
		{
			HDCP_None,
			HDCP_Required
		};

		enum Language
		{
			LANG_AmericanEnglish,
			LANG_BritishEnglish,
			LANG_Japanese,
			LANG_French,
			LANG_German,
			LANG_LatinAmericanSpanish,
			LANG_Spanish,
			LANG_Italian,
			LANG_Dutch,
			LANG_CanadianFrench,
			LANG_Portuguese,
			LANG_Russian,
			LANG_Korean,
			LANG_TraditionalChinese,
			LANG_SimplifiedChinese
		};

		enum LogoHandling
		{
			LHND_Auto,
			LHND_None
		};

		enum LogoType
		{
			LOGO_LicensedByNintendo,
			LOGO_DistributedByNintendo,
			LOGO_Nintendo
		};

		enum Organisation
		{
			ORGN_CERO,
			ORGN_GRACGCRB,
			ORGN_GSRMR,
			ORGN_ESRB,
			ORGN_ClassInd,
			ORGN_USK,
			ORGN_PEGI,
			ORGN_PEGIPortugal,
			ORGN_PEGIBBFC,
			ORGN_Russian,
			ORGN_ACB,
			ORGN_OFLC
		};

		enum ParentalControlFlag
		{
			PC_None,
			PC_FreeCommunication
		};

		enum PlayLogPolicy
		{
			PLP_All,
			PLP_LogOnly,
			PLP_None
		};

		enum PlayLogQueryCapability
		{
			PLQC_None,
			PLQC_Whitelist,
			PLQC_All
		};

		enum RepairFlag
		{
			REPF_None,
			REPF_SuppressGameCardAccess
		};

		enum RuntimeAocInstallMode
		{
			RTAOC_Deny,
			RTAOC_AllowAppend
		};

		enum ScreenshotMode
		{
			SCRN_Allow,
			SCRN_Deny
		};

		enum StartupUserAccount
		{
			USER_None,
			USER_Required,
			USER_RequiredWithNetworkServiceAccountAvailable
		};

		enum TouchScreenUsageMode
		{
			TOUCH_None,
			TOUCH_Supported,
			TOUCH_Required,
		};

		enum VideoCaptureMode
		{
			VCAP_Disable,
			VCAP_Manual,
			VCAP_Enable
		};
	}


#pragma pack(push,1)
	struct sApplicationTitle
	{
		char name[nacp::kNameLength];
		char publisher[nacp::kPublisherLength];
	};

	struct sApplicationControlProperty
	{
		sApplicationTitle title[nacp::kMaxLanguageCount];
		char isbn[nacp::kIsbnLength];
		byte_t startup_user_account;
		byte_t touch_screen_usage;
		byte_t add_on_content_registration_type;
		le_uint32_t attribute_flag;
		le_uint32_t supported_language_flag;
		le_uint32_t parental_control_flag;
		byte_t screenshot;
		byte_t video_capture;
		byte_t data_loss_confirmation;
		byte_t play_log_policy;
		le_uint64_t presence_group_id;
		int8_t rating_age[nacp::kRatingAgeCount];
		char display_version[nacp::kDisplayVersionLength];
		le_uint64_t add_on_content_base_id;
		le_uint64_t save_data_owner_id;
		le_uint64_t user_account_save_data_size;
		le_uint64_t user_account_save_data_journal_size;
		le_uint64_t device_save_data_size;
		le_uint64_t device_save_data_journal_size;
		le_uint64_t bcat_delivery_cache_storage_size;
		char application_error_code_category[nacp::kApplicationErrorCodeCategoryLength];
		le_uint64_t local_communication_id[nacp::kLocalCommunicationIdCount];
		byte_t logo_type;
		byte_t logo_handling;
		byte_t runtime_add_on_content_install;
		byte_t reserved_00[3];
		byte_t crash_report;
		byte_t hdcp;
		le_uint64_t seed_for_pseudo_device_id;
		char bcat_passphrase[nacp::kBcatPassphraseLength];
		byte_t reserved_01;
		byte_t reserved_02[6]; //reserved_for_user_account_save_data_operation
		le_uint64_t user_account_save_data_size_max;
		le_uint64_t user_account_save_data_journal_size_max;
		le_uint64_t device_save_data_size_max;
		le_uint64_t device_save_data_journal_size_max;
		le_uint64_t temporary_storage_size;
		le_uint64_t cache_storage_size;
		le_uint64_t cache_storage_journal_size;
		le_uint64_t cache_storage_data_and_journal_size_max;
		le_uint16_t cache_storage_index_max;
		byte_t reserved_03[6];
		le_uint64_t play_log_queryable_application_id[nacp::kPlayLogQueryableApplicationIdCount];
		byte_t play_log_query_capability;
		byte_t repair_flag;
		byte_t program_index;
		byte_t reserved_04[0xDED];
	};
#pragma pack(pop)
}
}