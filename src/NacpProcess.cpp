#include "NacpProcess.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>

#include <nn/hac/ApplicationControlPropertyUtil.h>

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

const nn::hac::ApplicationControlProperty& NacpProcess::getApplicationControlProperty() const
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
		std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getLanguageAsString(mNacp.getTitle()[i].language) << " Title:" << std::endl;
		std::cout << "      Name:                       " << mNacp.getTitle()[i].name << std::endl;
		std::cout << "      Publisher:                  " << mNacp.getTitle()[i].publisher << std::endl;
	}
	std::cout << "  Logo:" << std::endl;
	std::cout << "    Type:                         " << nn::hac::ApplicationControlPropertyUtil::getLogoTypeAsString(mNacp.getLogoType()) << std::endl;
	std::cout << "    Handling:                     " << nn::hac::ApplicationControlPropertyUtil::getLogoHandlingAsString(mNacp.getLogoHandling()) << std::endl;
	std::cout << "  AddOnContent:" << std::endl;
	std::cout << "    BaseId:                       0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getAocBaseId() << std::endl;
	std::cout << "    RegistrationType:             " << nn::hac::ApplicationControlPropertyUtil::getAocRegistrationTypeAsString(mNacp.getAocRegistrationType()) << std::endl;
	std::cout << "    RuntimeInstallMode:           " << nn::hac::ApplicationControlPropertyUtil::getRuntimeAocInstallModeAsString(mNacp.getRuntimeAocInstallMode()) << std::endl;
	std::cout << "  Play Log:" << std::endl;
	std::cout << "    PlayLogPolicy:                " << nn::hac::ApplicationControlPropertyUtil::getPlayLogPolicyAsString(mNacp.getPlayLogPolicy()) << std::endl;
	std::cout << "    PlayLogQueryCapability:       " << nn::hac::ApplicationControlPropertyUtil::getPlayLogQueryCapabilityAsString(mNacp.getPlayLogQueryCapability()) << std::endl;
	if (mNacp.getPlayLogQueryableApplicationId().size() > 0)
	{
		std::cout << "    PlayLogQueryableApplicationId:" << std::endl;
		for (size_t i = 0; i < mNacp.getPlayLogQueryableApplicationId().size(); i++)
		{
			std::cout << "      0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getPlayLogQueryableApplicationId()[i] << std::endl;
		}
	}
	std::cout << "  Parental Controls:" << std::endl;
	std::cout << "    ParentalControlFlag:          " << nn::hac::ApplicationControlPropertyUtil::getParentalControlFlagAsString(mNacp.getParentalControlFlag()) << std::endl;
	for (size_t i = 0; i < mNacp.getRatingAge().size(); i++)
	{
		std::cout << "    Age Restriction:" << std::endl;
		std::cout << "      Agency:  " << nn::hac::ApplicationControlPropertyUtil::getOrganisationAsString(mNacp.getRatingAge()[i].organisation) << std::endl;
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
		std::cout << "      Size:                       " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataSize().size) << std::endl;
		std::cout << "      JournalSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataSize().journal_size) << std::endl;
	}
	if (mNacp.getDeviceSaveDataSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    DeviceSaveData:" << std::endl;
		std::cout << "      Size:                       " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataSize().size) << std::endl;
		std::cout << "      JournalSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataSize().journal_size) << std::endl;
	}
	if (mNacp.getUserAccountSaveDataMax().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    UserAccountSaveDataMax:" << std::endl;
		std::cout << "      Size:                       " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataMax().size) << std::endl;
		std::cout << "      JournalSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataMax().journal_size) << std::endl;
	}
	if (mNacp.getDeviceSaveDataMax().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    DeviceSaveDataMax:" << std::endl;
		std::cout << "      Size:                       " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataMax().size) << std::endl;
		std::cout << "      JournalSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataMax().journal_size) << std::endl;
	}
	if (mNacp.getTemporaryStorageSize() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    TemporaryStorageSize:         " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getTemporaryStorageSize()) << std::endl;
	}
	if (mNacp.getCacheStorageSize().journal_size > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "    CacheStorage:" << std::endl;
		std::cout << "      Size:                       " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageSize().size) << std::endl;
		std::cout << "      JournalSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageSize().journal_size) << std::endl;
		std::cout << "      MaxDataAndJournalSize:      " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageDataAndJournalSizeMax()) << std::endl;
		std::cout << "      StorageIndexMax:            0x" << std::hex << mNacp.getCacheStorageIndexMax() << std::endl;
	}
	std::cout << "  Other Flags:" << std::endl;
	std::cout << "    StartupUserAccount:           " << nn::hac::ApplicationControlPropertyUtil::getStartupUserAccountAsString(mNacp.getStartupUserAccount()) << std::endl;
	std::cout << "    UserAccountSwitchLock:        " << nn::hac::ApplicationControlPropertyUtil::getUserAccountSwitchLockValueAsString(mNacp.getUserAccountSwitchLockValue()) << std::endl;
	std::cout << "    AttributeFlag:                " << nn::hac::ApplicationControlPropertyUtil::getAttributeFlagAsString(mNacp.getAttributeFlag()) << std::endl;
	std::cout << "    CrashReportMode:              " << nn::hac::ApplicationControlPropertyUtil::getCrashReportModeAsString(mNacp.getCrashReportMode()) << std::endl;
	std::cout << "    HDCP:                         " << nn::hac::ApplicationControlPropertyUtil::getHdcpAsString(mNacp.getHdcp()) << std::endl;
	std::cout << "    ScreenshotMode:               " << nn::hac::ApplicationControlPropertyUtil::getScreenshotModeAsString(mNacp.getScreenshotMode()) << std::endl;
	std::cout << "    VideoCaptureMode:             " << nn::hac::ApplicationControlPropertyUtil::getVideoCaptureModeAsString(mNacp.getVideoCaptureMode()) << std::endl;
	std::cout << "    DataLossConfirmation:         " << nn::hac::ApplicationControlPropertyUtil::getDataLossConfirmationAsString(mNacp.getDataLossConfirmation()) << std::endl;
	std::cout << "    RepairFlag:                   " << nn::hac::ApplicationControlPropertyUtil::getRepairFlagAsString(mNacp.getRepairFlag()) << std::endl;
	std::cout << "    ProgramIndex:                 0x" << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)mNacp.getProgramIndex() << std::endl;
	std::cout << "    Req NetworkLicenseOnLaunch:   " << nn::hac::ApplicationControlPropertyUtil::getRequiredNetworkServiceLicenseOnLaunchValueAsString(mNacp.getRequiredNetworkServiceLicenseOnLaunchValue()) << std::endl;
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