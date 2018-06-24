#include <sstream>
#include <fnd/SimpleTextOutput.h>
#include "OffsetAdjustedIFile.h"
#include "NacpProcess.h"

const char* getLanguageStr(nx::nacp::Language var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::LANG_AmericanEnglish):
		str = "AmericanEnglish";
		break;
	case (nx::nacp::LANG_BritishEnglish):
		str = "BritishEnglish";
		break;
	case (nx::nacp::LANG_Japanese):
		str = "Japanese";
		break;
	case (nx::nacp::LANG_French):
		str = "French";
		break;
	case (nx::nacp::LANG_German):
		str = "German";
		break;
	case (nx::nacp::LANG_LatinAmericanSpanish):
		str = "LatinAmericanSpanish";
		break;
	case (nx::nacp::LANG_Spanish):
		str = "Spanish";
		break;
	case (nx::nacp::LANG_Italian):
		str = "Italian";
		break;
	case (nx::nacp::LANG_Dutch):
		str = "Dutch";
		break;
	case (nx::nacp::LANG_CanadianFrench):
		str = "CanadianFrench";
		break;
	case (nx::nacp::LANG_Portuguese):
		str = "Portuguese";
		break;
	case (nx::nacp::LANG_Russian):
		str = "Russian";
		break;
	case (nx::nacp::LANG_Korean):
		str = "Korean";
		break;
	case (nx::nacp::LANG_TraditionalChinese):
		str = "TraditionalChinese";
		break;
	case (nx::nacp::LANG_SimplifiedChinese):
		str = "SimplifiedChinese";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getStartupUserAccountStr(nx::nacp::StartupUserAccount var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::USER_None):
		str = "None";
		break;
	case (nx::nacp::USER_Required):
		str = "Required";
		break;
	case (nx::nacp::USER_RequiredWithNetworkServiceAccountAvailable):
		str = "RequiredWithNetworkServiceAccountAvailable";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getTouchScreenUsageModeStr(nx::nacp::TouchScreenUsageMode var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::TOUCH_None):
		str = "None";
		break;
	case (nx::nacp::TOUCH_Supported):
		str = "Supported";
		break;
	case (nx::nacp::TOUCH_Required):
		str = "Required";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getAocRegistrationTypeStr(nx::nacp::AocRegistrationType var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::AOC_AllOnLaunch):
		str = "AllOnLaunch";
		break;
	case (nx::nacp::AOC_OnDemand):
		str = "OnDemand";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getAttributeFlagStr(nx::nacp::AttributeFlag var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::ATTR_None):
		str = "None";
		break;
	case (nx::nacp::ATTR_Demo):
		str = "Demo";
		break;
	case (nx::nacp::ATTR_RetailInteractiveDisplay):
		str = "RetailInteractiveDisplay";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getParentalControlFlagStr(nx::nacp::ParentalControlFlag var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::PC_None):
		str = "None";
		break;
	case (nx::nacp::PC_FreeCommunication):
		str = "FreeCommunication";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getScreenshotModeStr(nx::nacp::ScreenshotMode var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::SCRN_Allow):
		str = "Allow";
		break;
	case (nx::nacp::SCRN_Deny):
		str = "Deny";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getVideoCaptureModeStr(nx::nacp::VideoCaptureMode var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::VCAP_Disable):
		str = "Disable";
		break;
	case (nx::nacp::VCAP_Manual):
		str = "Manual";
		break;
	case (nx::nacp::VCAP_Enable):
		str = "Enable";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getDataLossConfirmationStr(nx::nacp::DataLossConfirmation var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::DLOSS_None):
		str = "None";
		break;
	case (nx::nacp::DLOSS_Required):
		str = "Required";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getPlayLogPolicyStr(nx::nacp::PlayLogPolicy var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::PLP_All):
		str = "All";
		break;
	case (nx::nacp::PLP_LogOnly):
		str = "LogOnly";
		break;
	case (nx::nacp::PLP_None):
		str = "None";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getOrganisationStr(nx::nacp::Organisation var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::ORGN_CERO):
		str = "CERO";
		break;
	case (nx::nacp::ORGN_GRACGCRB):
		str = "GRACGCRB";
		break;
	case (nx::nacp::ORGN_GSRMR):
		str = "GSRMR";
		break;
	case (nx::nacp::ORGN_ESRB):
		str = "ESRB";
		break;
	case (nx::nacp::ORGN_ClassInd):
		str = "ClassInd";
		break;
	case (nx::nacp::ORGN_USK):
		str = "USK";
		break;
	case (nx::nacp::ORGN_PEGI):
		str = "PEGI";
		break;
	case (nx::nacp::ORGN_PEGIPortugal):
		str = "PEGIPortugal";
		break;
	case (nx::nacp::ORGN_PEGIBBFC):
		str = "PEGIBBFC";
		break;
	case (nx::nacp::ORGN_Russian):
		str = "Russian";
		break;
	case (nx::nacp::ORGN_ACB):
		str = "ACB";
		break;
	case (nx::nacp::ORGN_OFLC):
		str = "OFLC";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getLogoTypeStr(nx::nacp::LogoType var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::LOGO_LicensedByNintendo):
		str = "LicensedByNintendo";
		break;
	case (nx::nacp::LOGO_DistributedByNintendo):
		str = "DistributedByNintendo";
		break;
	case (nx::nacp::LOGO_Nintendo):
		str = "Nintendo";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getLogoHandlingStr(nx::nacp::LogoHandling var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::LHND_Auto):
		str = "Auto";
		break;
	case (nx::nacp::LHND_None):
		str = "None";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getRuntimeAocInstallModeStr(nx::nacp::RuntimeAocInstallMode var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::RTAOC_Deny):
		str = "Deny";
		break;
	case (nx::nacp::RTAOC_AllowAppend):
		str = "AllowAppend";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getCrashReportModeStr(nx::nacp::CrashReportMode var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::CREP_Deny):
		str = "Deny";
		break;
	case (nx::nacp::CREP_Allow):
		str = "Allow";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getHdcpStr(nx::nacp::Hdcp var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::HDCP_None):
		str = "None";
		break;
	case (nx::nacp::HDCP_Required):
		str = "Required";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getPlayLogQueryCapabilityStr(nx::nacp::PlayLogQueryCapability var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::PLQC_None):
		str = "None";
		break;
	case (nx::nacp::PLQC_Whitelist):
		str = "Whitelist";
		break;
	case (nx::nacp::PLQC_All):
		str = "All";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

const char* getRepairFlagStr(nx::nacp::RepairFlag var)
{
	const char* str = nullptr;
	switch(var)
	{
	case (nx::nacp::REPF_None):
		str = "None";
		break;
	case (nx::nacp::REPF_SuppressGameCardAccess):
		str = "SuppressGameCardAccess";
		break;
	default:
		str = "Unknown";
	}
	return str;
}

std::string getSaveDataSizeStr(int64_t size)
{
	static const int64_t kKiloByte = 1024;
	static const int64_t kMegaByte = 1024 * 1024;

	std::stringstream sstr;


	if (size < kKiloByte)
	{
		sstr << size << " B";
	}
	else if (size < kMegaByte)
	{
		sstr << (size/kKiloByte) << " KB";
	}
	else
	{
		sstr << (size/kMegaByte) << " MB";
	}

	return sstr.str();
}

NacpProcess::NacpProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

NacpProcess::~NacpProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void NacpProcess::process()
{
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	mNacp.fromBytes(scratch.data(), scratch.size());

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayNacp();
}

void NacpProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void NacpProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void NacpProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nx::ApplicationControlPropertyBinary& NacpProcess::getApplicationControlPropertyBinary() const
{
	return mNacp;
}

void NacpProcess::displayNacp()
{
	printf("[ApplicationControlProperty]\n");
	printf("  Menu Description:\n");
	printf("    DisplayVersion:               %s\n", mNacp.getDisplayVersion().c_str());
	if (mNacp.getIsbn().empty() == false || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		printf("    ISBN:                         %s\n", mNacp.getIsbn().c_str());
	for (size_t i = 0; i < mNacp.getTitle().size(); i++)
	{
		printf("    %s Title:\n", getLanguageStr(mNacp.getTitle()[i].language));
		printf("      Name:                       %s\n", mNacp.getTitle()[i].name.c_str());
		printf("      Publisher:                  %s\n", mNacp.getTitle()[i].publisher.c_str());
	}
	printf("  Logo:\n");
	printf("    Type:                         %s\n", getLogoTypeStr(mNacp.getLogoType()));
	printf("    Handling:                     %s\n", getLogoHandlingStr(mNacp.getLogoHandling()));
	printf("  AddOnContent:\n");
	printf("    BaseId:                       0x%016" PRIx64 "\n", mNacp.getAocBaseId());
	printf("    RegistrationType:             %s\n", getAocRegistrationTypeStr(mNacp.getAocRegistrationType()));
	printf("    RuntimeInstallMode:           %s\n", getRuntimeAocInstallModeStr(mNacp.getRuntimeAocInstallMode()));
	printf("  Play Log:\n");
	printf("    PlayLogPolicy:                %s\n", getPlayLogPolicyStr(mNacp.getPlayLogPolicy()));
	printf("    PlayLogQueryCapability:       %s\n", getPlayLogQueryCapabilityStr(mNacp.getPlayLogQueryCapability()));
	if (mNacp.getPlayLogQueryableApplicationId().size() > 0)
	{
		printf("    PlayLogQueryableApplicationId:\n");
		for (size_t i = 0; i < mNacp.getPlayLogQueryableApplicationId().size(); i++)
		{
			printf("      0x%016" PRIx64 "\n", mNacp.getPlayLogQueryableApplicationId()[i]);
		}
	}
	printf("  Parental Controls:\n");
	printf("    ParentalControlFlag:          %s\n", getParentalControlFlagStr(mNacp.getParentalControlFlag()));
	for (size_t i = 0; i < mNacp.getRatingAge().size(); i++)
	{
		printf("    Age Restriction:\n");
		printf("      Agency:  %s\n", getOrganisationStr(mNacp.getRatingAge()[i].organisation));
		printf("      Age:     %d\n", mNacp.getRatingAge()[i].age);
	}
	
	if (mNacp.getBcatPassphase().empty() == false || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("  BCAT:\n");
		printf("    BcatPassphase:                %s\n", mNacp.getBcatPassphase().c_str());
		printf("    DeliveryCacheStorageSize:     0x%016" PRIx64 "\n", mNacp.getBcatDeliveryCacheStorageSize());
	}
	if (mNacp.getLocalCommunicationId().size() > 0)
	{
		printf("  Local Area Communication:\n");
		printf("    LocalCommunicationId:\n");
		for (size_t i = 0; i < mNacp.getLocalCommunicationId().size(); i++)
		{
			printf("      0x%016" PRIx64 "\n", mNacp.getLocalCommunicationId()[i]);
		}
	}
	printf("  SaveData:\n");
	printf("    SaveDatawOwnerId:             0x%016" PRIx64 "\n", mNacp.getSaveDatawOwnerId());
	if (mNacp.getUserAccountSaveDataSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    UserAccountSaveData:\n");
		printf("      Size:                       %s\n", getSaveDataSizeStr(mNacp.getUserAccountSaveDataSize().size).c_str());
		printf("      JournalSize:                %s\n", getSaveDataSizeStr(mNacp.getUserAccountSaveDataSize().journal_size).c_str());
	}
	if (mNacp.getDeviceSaveDataSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    DeviceSaveData:\n");
		printf("      Size:                       %s\n", getSaveDataSizeStr(mNacp.getDeviceSaveDataSize().size).c_str());
		printf("      JournalSize:                %s\n", getSaveDataSizeStr(mNacp.getDeviceSaveDataSize().journal_size).c_str());
	}
	if (mNacp.getUserAccountSaveDataMax().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    UserAccountSaveDataMax:\n");
		printf("      Size:                       %s\n", getSaveDataSizeStr(mNacp.getUserAccountSaveDataMax().size).c_str());
		printf("      JournalSize:                %s\n", getSaveDataSizeStr(mNacp.getUserAccountSaveDataMax().journal_size).c_str());
	}
	if (mNacp.getDeviceSaveDataMax().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    DeviceSaveDataMax:\n");
		printf("      Size:                       %s\n", getSaveDataSizeStr(mNacp.getDeviceSaveDataMax().size).c_str());
		printf("      JournalSize:                %s\n", getSaveDataSizeStr(mNacp.getDeviceSaveDataMax().journal_size).c_str());
	}
	if (mNacp.getTemporaryStorageSize() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    TemporaryStorageSize:         %s\n", getSaveDataSizeStr(mNacp.getTemporaryStorageSize()).c_str());
	}
	if (mNacp.getCacheStorageSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    CacheStorage:\n");
		printf("      Size:                       %s\n", getSaveDataSizeStr(mNacp.getCacheStorageSize().size).c_str());
		printf("      JournalSize:                %s\n", getSaveDataSizeStr(mNacp.getCacheStorageSize().journal_size).c_str());
		printf("      MaxDataAndJournalSize:      %s\n", getSaveDataSizeStr(mNacp.getCacheStorageDataAndJournalSizeMax()).c_str());
		printf("      StorageIndex:               0x%" PRIx16 "\n", mNacp.getCacheStorageIndex());
	}
	printf("  Other Flags:\n");
	printf("    StartupUserAccount:           %s\n", getStartupUserAccountStr(mNacp.getStartupUserAccount()));
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    TouchScreenUsageMode:         %s\n", getTouchScreenUsageModeStr(mNacp.getTouchScreenUsageMode()));
	}
	printf("    AttributeFlag:                %s\n", getAttributeFlagStr(mNacp.getAttributeFlag()));
	printf("    CrashReportMode:              %s\n", getCrashReportModeStr(mNacp.getCrashReportMode()));
	printf("    HDCP:                         %s\n", getHdcpStr(mNacp.getHdcp()));
	printf("    ScreenshotMode:               %s\n", getScreenshotModeStr(mNacp.getScreenshotMode()));
	printf("    VideoCaptureMode:             %s\n", getVideoCaptureModeStr(mNacp.getVideoCaptureMode()));
	printf("    DataLossConfirmation:         %s\n", getDataLossConfirmationStr(mNacp.getDataLossConfirmation()));
	printf("    RepairFlag:                   %s\n", getRepairFlagStr(mNacp.getRepairFlag()));
	printf("    ProgramIndex:                 0x%02x\n", mNacp.getProgramIndex());
	if (mNacp.getApplicationErrorCodeCategory().empty() == false || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    ApplicationErrorCodeCategory: %s\n", mNacp.getApplicationErrorCodeCategory().c_str());
	}
	if (mNacp.getSeedForPsuedoDeviceId() > 0 || mNacp.getPresenceGroupId() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("  Other Ids:\n");
		if (mNacp.getSeedForPsuedoDeviceId() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
			printf("    SeedForPsuedoDeviceId:        0x%016" PRIx64 "\n", mNacp.getSeedForPsuedoDeviceId());
		if (mNacp.getPresenceGroupId() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
			printf("    PresenceGroupId:              0x%016" PRIx64 "\n", mNacp.getPresenceGroupId());
	}
}
