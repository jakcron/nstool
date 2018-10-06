#include <sstream>
#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include "NacpProcess.h"

NacpProcess::NacpProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void NacpProcess::process()
{
	importNacp();

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayNacp();
}

void NacpProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void NacpProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void NacpProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nn::hac::ApplicationControlPropertyBinary& NacpProcess::getApplicationControlPropertyBinary() const
{
	return mNacp;
}

void NacpProcess::importNacp()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mNacp.fromBytes(scratch.data(), scratch.size());
}

void NacpProcess::displayNacp()
{
	std::cout << "[ApplicationControlProperty]" << std::endl;
	std::cout << "  Menu Description:" << std::endl;
	std::cout << "    DisplayVersion:               " << mNacp.getDisplayVersion() << std::endl;
	if (mNacp.getIsbn().empty() == false || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << "    ISBN:                         " << mNacp.getIsbn() << std::endl;
	for (size_t i = 0; i < mNacp.getTitle().size(); i++)
	{
		std::cout << "    " << getLanguageStr(mNacp.getTitle()[i].language) << " Title:" << std::endl;
		std::cout << "      Name:                       " << mNacp.getTitle()[i].name << std::endl;
		std::cout << "      Publisher:                  " << mNacp.getTitle()[i].publisher << std::endl;
	}
	std::cout << "  Logo:" << std::endl;
	std::cout << "    Type:                         " << getLogoTypeStr(mNacp.getLogoType()) << std::endl;
	std::cout << "    Handling:                     " << getLogoHandlingStr(mNacp.getLogoHandling()) << std::endl;
	std::cout << "  AddOnContent:" << std::endl;
	std::cout << "    BaseId:                       0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getAocBaseId() << std::endl;
	std::cout << "    RegistrationType:             " << getAocRegistrationTypeStr(mNacp.getAocRegistrationType()) << std::endl;
	std::cout << "    RuntimeInstallMode:           " << getRuntimeAocInstallModeStr(mNacp.getRuntimeAocInstallMode()) << std::endl;
	std::cout << "  Play Log:" << std::endl;
	std::cout << "    PlayLogPolicy:                " << getPlayLogPolicyStr(mNacp.getPlayLogPolicy()) << std::endl;
	std::cout << "    PlayLogQueryCapability:       " << getPlayLogQueryCapabilityStr(mNacp.getPlayLogQueryCapability()) << std::endl;
	if (mNacp.getPlayLogQueryableApplicationId().size() > 0)
	{
		std::cout << "    PlayLogQueryableApplicationId:" << std::endl;
		for (size_t i = 0; i < mNacp.getPlayLogQueryableApplicationId().size(); i++)
		{
			std::cout << "      0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getPlayLogQueryableApplicationId()[i] << std::endl;
		}
	}
	std::cout << "  Parental Controls:" << std::endl;
	std::cout << "    ParentalControlFlag:          " << getParentalControlFlagStr(mNacp.getParentalControlFlag()) << std::endl;
	for (size_t i = 0; i < mNacp.getRatingAge().size(); i++)
	{
		std::cout << "    Age Restriction:" << std::endl;
		std::cout << "      Agency:  " << getOrganisationStr(mNacp.getRatingAge()[i].organisation) << std::endl;
		std::cout << "      Age:     " << std::dec << (uint32_t)mNacp.getRatingAge()[i].age << std::endl;
	}
	
	if (mNacp.getBcatPassphase().empty() == false || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  BCAT:" << std::endl;
		std::cout << "    BcatPassphase:                " << mNacp.getBcatPassphase() << std::endl;
		std::cout << "    DeliveryCacheStorageSize:     0x" << std::hex << mNacp.getBcatDeliveryCacheStorageSize() << std::endl;
	}
	if (mNacp.getLocalCommunicationId().size() > 0)
	{
		std::cout << "  Local Area Communication:" << std::endl;
		std::cout << "    LocalCommunicationId:" << std::endl;
		for (size_t i = 0; i < mNacp.getLocalCommunicationId().size(); i++)
		{
			std::cout << "      0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getLocalCommunicationId()[i] << std::endl;
		}
	}
	std::cout << "  SaveData:" << std::endl;
	std::cout << "    SaveDatawOwnerId:             0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getSaveDatawOwnerId() << std::endl;
	if (mNacp.getUserAccountSaveDataSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    UserAccountSaveData:" << std::endl;
		std::cout << "      Size:                       " << getSaveDataSizeStr(mNacp.getUserAccountSaveDataSize().size) << std::endl;
		std::cout << "      JournalSize:                " << getSaveDataSizeStr(mNacp.getUserAccountSaveDataSize().journal_size) << std::endl;
	}
	if (mNacp.getDeviceSaveDataSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    DeviceSaveData:" << std::endl;
		std::cout << "      Size:                       " << getSaveDataSizeStr(mNacp.getDeviceSaveDataSize().size) << std::endl;
		std::cout << "      JournalSize:                " << getSaveDataSizeStr(mNacp.getDeviceSaveDataSize().journal_size) << std::endl;
	}
	if (mNacp.getUserAccountSaveDataMax().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    UserAccountSaveDataMax:" << std::endl;
		std::cout << "      Size:                       " << getSaveDataSizeStr(mNacp.getUserAccountSaveDataMax().size) << std::endl;
		std::cout << "      JournalSize:                " << getSaveDataSizeStr(mNacp.getUserAccountSaveDataMax().journal_size) << std::endl;
	}
	if (mNacp.getDeviceSaveDataMax().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    DeviceSaveDataMax:" << std::endl;
		std::cout << "      Size:                       " << getSaveDataSizeStr(mNacp.getDeviceSaveDataMax().size) << std::endl;
		std::cout << "      JournalSize:                " << getSaveDataSizeStr(mNacp.getDeviceSaveDataMax().journal_size) << std::endl;
	}
	if (mNacp.getTemporaryStorageSize() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    TemporaryStorageSize:         " << getSaveDataSizeStr(mNacp.getTemporaryStorageSize()) << std::endl;
	}
	if (mNacp.getCacheStorageSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    CacheStorage:" << std::endl;
		std::cout << "      Size:                       " << getSaveDataSizeStr(mNacp.getCacheStorageSize().size) << std::endl;
		std::cout << "      JournalSize:                " << getSaveDataSizeStr(mNacp.getCacheStorageSize().journal_size) << std::endl;
		std::cout << "      MaxDataAndJournalSize:      " << getSaveDataSizeStr(mNacp.getCacheStorageDataAndJournalSizeMax()) << std::endl;
		std::cout << "      StorageIndexMax:            0x" << std::hex << mNacp.getCacheStorageIndexMax() << std::endl;
	}
	std::cout << "  Other Flags:" << std::endl;
	std::cout << "    StartupUserAccount:           " << getStartupUserAccountStr(mNacp.getStartupUserAccount()) << std::endl;
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    TouchScreenUsageMode:         " << getTouchScreenUsageModeStr(mNacp.getTouchScreenUsageMode()) << std::endl;
	}
	std::cout << "    AttributeFlag:                " << getAttributeFlagStr(mNacp.getAttributeFlag()) << std::endl;
	std::cout << "    CrashReportMode:              " << getCrashReportModeStr(mNacp.getCrashReportMode()) << std::endl;
	std::cout << "    HDCP:                         " << getHdcpStr(mNacp.getHdcp()) << std::endl;
	std::cout << "    ScreenshotMode:               " << getScreenshotModeStr(mNacp.getScreenshotMode()) << std::endl;
	std::cout << "    VideoCaptureMode:             " << getVideoCaptureModeStr(mNacp.getVideoCaptureMode()) << std::endl;
	std::cout << "    DataLossConfirmation:         " << getDataLossConfirmationStr(mNacp.getDataLossConfirmation()) << std::endl;
	std::cout << "    RepairFlag:                   " << getRepairFlagStr(mNacp.getRepairFlag()) << std::endl;
	std::cout << "    ProgramIndex:                 0x" << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)mNacp.getProgramIndex() << std::endl;
	if (mNacp.getApplicationErrorCodeCategory().empty() == false || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    ApplicationErrorCodeCategory: " << mNacp.getApplicationErrorCodeCategory() << std::endl;
	}
	if (mNacp.getSeedForPsuedoDeviceId() > 0 || mNacp.getPresenceGroupId() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Other Ids:" << std::endl;
		if (mNacp.getSeedForPsuedoDeviceId() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
			std::cout << "    SeedForPsuedoDeviceId:        0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getSeedForPsuedoDeviceId() << std::endl;
		if (mNacp.getPresenceGroupId() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
			std::cout << "    PresenceGroupId:              0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getPresenceGroupId() << std::endl;
	}
}

const char* NacpProcess::getLanguageStr(nn::hac::nacp::Language var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::LANG_AmericanEnglish):
		str = "AmericanEnglish";
		break;
	case (nn::hac::nacp::LANG_BritishEnglish):
		str = "BritishEnglish";
		break;
	case (nn::hac::nacp::LANG_Japanese):
		str = "Japanese";
		break;
	case (nn::hac::nacp::LANG_French):
		str = "French";
		break;
	case (nn::hac::nacp::LANG_German):
		str = "German";
		break;
	case (nn::hac::nacp::LANG_LatinAmericanSpanish):
		str = "LatinAmericanSpanish";
		break;
	case (nn::hac::nacp::LANG_Spanish):
		str = "Spanish";
		break;
	case (nn::hac::nacp::LANG_Italian):
		str = "Italian";
		break;
	case (nn::hac::nacp::LANG_Dutch):
		str = "Dutch";
		break;
	case (nn::hac::nacp::LANG_CanadianFrench):
		str = "CanadianFrench";
		break;
	case (nn::hac::nacp::LANG_Portuguese):
		str = "Portuguese";
		break;
	case (nn::hac::nacp::LANG_Russian):
		str = "Russian";
		break;
	case (nn::hac::nacp::LANG_Korean):
		str = "Korean";
		break;
	case (nn::hac::nacp::LANG_TraditionalChinese):
		str = "TraditionalChinese";
		break;
	case (nn::hac::nacp::LANG_SimplifiedChinese):
		str = "SimplifiedChinese";
		break;
	default:
		str = "Unknown";
	}
	
	return str;
}

const char* NacpProcess::getStartupUserAccountStr(nn::hac::nacp::StartupUserAccount var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::USER_None):
		str = "None";
		break;
	case (nn::hac::nacp::USER_Required):
		str = "Required";
		break;
	case (nn::hac::nacp::USER_RequiredWithNetworkServiceAccountAvailable):
		str = "RequiredWithNetworkServiceAccountAvailable";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getTouchScreenUsageModeStr(nn::hac::nacp::TouchScreenUsageMode var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::TOUCH_None):
		str = "None";
		break;
	case (nn::hac::nacp::TOUCH_Supported):
		str = "Supported";
		break;
	case (nn::hac::nacp::TOUCH_Required):
		str = "Required";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getAocRegistrationTypeStr(nn::hac::nacp::AocRegistrationType var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::AOC_AllOnLaunch):
		str = "AllOnLaunch";
		break;
	case (nn::hac::nacp::AOC_OnDemand):
		str = "OnDemand";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getAttributeFlagStr(nn::hac::nacp::AttributeFlag var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::ATTR_None):
		str = "None";
		break;
	case (nn::hac::nacp::ATTR_Demo):
		str = "Demo";
		break;
	case (nn::hac::nacp::ATTR_RetailInteractiveDisplay):
		str = "RetailInteractiveDisplay";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getParentalControlFlagStr(nn::hac::nacp::ParentalControlFlag var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::PC_None):
		str = "None";
		break;
	case (nn::hac::nacp::PC_FreeCommunication):
		str = "FreeCommunication";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getScreenshotModeStr(nn::hac::nacp::ScreenshotMode var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::SCRN_Allow):
		str = "Allow";
		break;
	case (nn::hac::nacp::SCRN_Deny):
		str = "Deny";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getVideoCaptureModeStr(nn::hac::nacp::VideoCaptureMode var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::VCAP_Disable):
		str = "Disable";
		break;
	case (nn::hac::nacp::VCAP_Manual):
		str = "Manual";
		break;
	case (nn::hac::nacp::VCAP_Enable):
		str = "Enable";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getDataLossConfirmationStr(nn::hac::nacp::DataLossConfirmation var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::DLOSS_None):
		str = "None";
		break;
	case (nn::hac::nacp::DLOSS_Required):
		str = "Required";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getPlayLogPolicyStr(nn::hac::nacp::PlayLogPolicy var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::PLP_All):
		str = "All";
		break;
	case (nn::hac::nacp::PLP_LogOnly):
		str = "LogOnly";
		break;
	case (nn::hac::nacp::PLP_None):
		str = "None";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getOrganisationStr(nn::hac::nacp::Organisation var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::ORGN_CERO):
		str = "CERO";
		break;
	case (nn::hac::nacp::ORGN_GRACGCRB):
		str = "GRACGCRB";
		break;
	case (nn::hac::nacp::ORGN_GSRMR):
		str = "GSRMR";
		break;
	case (nn::hac::nacp::ORGN_ESRB):
		str = "ESRB";
		break;
	case (nn::hac::nacp::ORGN_ClassInd):
		str = "ClassInd";
		break;
	case (nn::hac::nacp::ORGN_USK):
		str = "USK";
		break;
	case (nn::hac::nacp::ORGN_PEGI):
		str = "PEGI";
		break;
	case (nn::hac::nacp::ORGN_PEGIPortugal):
		str = "PEGIPortugal";
		break;
	case (nn::hac::nacp::ORGN_PEGIBBFC):
		str = "PEGIBBFC";
		break;
	case (nn::hac::nacp::ORGN_Russian):
		str = "Russian";
		break;
	case (nn::hac::nacp::ORGN_ACB):
		str = "ACB";
		break;
	case (nn::hac::nacp::ORGN_OFLC):
		str = "OFLC";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getLogoTypeStr(nn::hac::nacp::LogoType var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::LOGO_LicensedByNintendo):
		str = "LicensedByNintendo";
		break;
	case (nn::hac::nacp::LOGO_DistributedByNintendo):
		str = "DistributedByNintendo";
		break;
	case (nn::hac::nacp::LOGO_Nintendo):
		str = "Nintendo";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getLogoHandlingStr(nn::hac::nacp::LogoHandling var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::LHND_Auto):
		str = "Auto";
		break;
	case (nn::hac::nacp::LHND_None):
		str = "None";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getRuntimeAocInstallModeStr(nn::hac::nacp::RuntimeAocInstallMode var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::RTAOC_Deny):
		str = "Deny";
		break;
	case (nn::hac::nacp::RTAOC_AllowAppend):
		str = "AllowAppend";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getCrashReportModeStr(nn::hac::nacp::CrashReportMode var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::CREP_Deny):
		str = "Deny";
		break;
	case (nn::hac::nacp::CREP_Allow):
		str = "Allow";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getHdcpStr(nn::hac::nacp::Hdcp var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::HDCP_None):
		str = "None";
		break;
	case (nn::hac::nacp::HDCP_Required):
		str = "Required";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getPlayLogQueryCapabilityStr(nn::hac::nacp::PlayLogQueryCapability var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::PLQC_None):
		str = "None";
		break;
	case (nn::hac::nacp::PLQC_Whitelist):
		str = "Whitelist";
		break;
	case (nn::hac::nacp::PLQC_All):
		str = "All";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

const char* NacpProcess::getRepairFlagStr(nn::hac::nacp::RepairFlag var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::nacp::REPF_None):
		str = "None";
		break;
	case (nn::hac::nacp::REPF_SuppressGameCardAccess):
		str = "SuppressGameCardAccess";
		break;
	default:
		str = "Unknown";
	}

	return str;
}

std::string NacpProcess::getSaveDataSizeStr(int64_t size) const
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