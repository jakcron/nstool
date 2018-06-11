#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	namespace nacp
	{
		enum AddOnContentRegistrationTypeValue
		{
			AOC_AllOnLaunch,
			AOC_OnDemand
		};

		enum AttributeFlagValue
		{
			ATTR_None,
			ATTR_Demo,
			ATTR_RetailInteractiveDisplay
		};

		enum CrashReportValue
		{
			CREP_Deny,
			CREP_Allow
		};

		enum DataLossConfirmationValue
		{
			DLOSS_None,
			DLOSS_Required
		};

		enum HdcpValue
		{
			HDCP_None,
			HDCP_Required
		}

		enum Langauge
		{
			AmericanEnglish,
			BritishEnglish,
			Japanese,
			French,
			German,
			LatinAmericanSpanish,
			Spanish,
			Italian,
			Dutch,
			CanadianFrench,
			Portuguese,
			Russian,
			Korean,
			TraditionalChinese,
			SimplifiedChinese
		};

		enum LogoHandlingValue
		{
			LHND_Auto,
			LHND_None
		};

		enum LogoTypeValue
		{
			LOGO_LicensedByNintendo,
			LOGO_Nintendo = 2
		};

		enum Organisation
		{
			CERO,
			GRACGCRB,
			GSRMR,
			ESRB,
			ClassInd,
			USK,
			PEGI,
			PEGIPortugal,
			PEGIBBFC,
			Russian,
			ACB,
			OFLC
		};

		enum ParentalControlFlagValue
		{
			PC_None,
			PC_FreeCommunication
		};

		enum PlayLogPolicyValue
		{
			PLP_All,
			PLP_LogOnly,
			PLP_None
		};

		enum PlayLogQueryCapabilityValue
		{
			PLQC_None,
			PLQC_Whitelist,
			PLQC_All
		};

		enum RepairFlagValue
		{
			REPF_None,
			REPF_SuppressGameCardAccess
		};

		enum RuntimeAddOnContentInstallValue
		{
			RTAOC_Deny,
			RTAOC_AllowAppend
		};

		enum ScreenshotValue
		{
			SCRN_Allow,
			SCRN_Deny
		};

		enum StartupUserAccountValue
		{
			USER_None,
			USER_Required,
			USER_RequiredWithNetworkServiceAccountAvailable
		};

		enum VideoCaptureValue
		{
			VCAP_Disable,
			VCAP_Manual,
			VCAP_Enable
		};

		static const size_t kNameLength = 0x200;
		static const size_t kPublisherLength = 0x100;
		static const size_t kMaxLanguageCount = 16;
		static const size_t kIsbnLength = 37;
		static const size_t kRatingAgeCount = 32;
		static const size_t kDisplayVersionLength = 16;
		static const size_t kApplicationErrorCodeCategoryLength = 8;
		static const size_t kLocalCommunicationIdCount = 8;
		static const size_t kBcatPassphraseLength = 65;
		static const size_t kReservedForUserAccountSaveDataOperationLength = 6;
		static const size_t kPlayLogQueryableApplicationIdCount = 16;
	}


#pragma pack(push,1)
	struct sApplicationTitle
	{
		char name[nacp::kNameLength];
		char publisher[nacp::kPublisherLength];
	}

	struct sApplicationControlProperty
	{
		sApplicationTitle title[nacp::kMaxLanguageCount];
		byte_t isbn[nacp::kIsbnLength]
		byte_t startup_user_account;
		byte_t reserved_00;
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
		byte_t application_error_code_category[nacp::kApplicationErrorCodeCategoryLength];
		le_uint64_t local_communication_id[nacp::kLocalCommunicationIdCount];
		byte_t logo_type;
		byte_t logo_handling;
		byte_t runtime_add_on_content_install;
		byte_t reserved_01[3];
		byte_t crash_report;
		byte_t hdcp;
		le_uint64_t seed_for_pseudo_device_id;
		byte_t bcat_passphrase[nacp::kBcatPassphraseLength];
		byte_t reserved_02;
		byte_t reserved_for_user_account_save_data_operation[nacp::kReservedForUserAccountSaveDataOperationLength];
		le_uint64_t user_account_save_data_size_max;
		le_uint64_t user_account_save_data_journal_size_max;
		le_uint64_t device_save_data_size_max;
		le_uint64_t device_save_data_journal_size_max;
		le_uint64_t temporary_storage_size;
		le_uint64_t cache_storage_size;
		le_uint64_t cache_storage_journal_size;
		le_uint64_t cache_storage_data_and_journal_size_max;
		le_uint16_t cache_storage_index;
		le_uint64_t play_log_queryable_application_id[kPlayLogQueryableApplicationIdCount];
		byte_t play_log_query_capability;
		byte_t repair_flag;
		byte_t program_index;
		byte_t reserved_03[0xDED];
	};
#pragma pack(pop)
}