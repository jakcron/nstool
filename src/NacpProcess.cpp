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
	
	// Title
	if (mNacp.getTitle().size() > 0)
	{
		std::cout << "  Title:" << std::endl;
		for (auto itr = mNacp.getTitle().begin(); itr != mNacp.getTitle().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getLanguageAsString(itr->language) << ":" << std::endl;
			std::cout << "      Name:       " << itr->name << std::endl;
			std::cout << "      Publisher:  " << itr->publisher << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Title:                                  None" << std::endl;
	}

	// Isbn
	if (mNacp.getIsbn().empty() == false)
	{
		std::cout << "  ISBN:                                   " << mNacp.getIsbn() << std::endl;
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  ISBN:                                   (NotSet)" << std::endl;
	}
	
	// StartupUserAccount
	if (mNacp.getStartupUserAccount() != nn::hac::nacp::StartupUserAccount::None || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  StartupUserAccount:                     " << nn::hac::ApplicationControlPropertyUtil::getStartupUserAccountAsString(mNacp.getStartupUserAccount()) << std::endl;
	}

	// UserAccountSwitchLock
	if (mNacp.getUserAccountSwitchLock() != nn::hac::nacp::UserAccountSwitchLock::Disable || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  UserAccountSwitchLock:                  " << nn::hac::ApplicationControlPropertyUtil::getUserAccountSwitchLockAsString(mNacp.getUserAccountSwitchLock()) << std::endl;
	}

	// AddOnContentRegistrationType
	if (mNacp.getAddOnContentRegistrationType() != nn::hac::nacp::AddOnContentRegistrationType::AllOnLaunch || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  AddOnContentRegistrationType:           " << nn::hac::ApplicationControlPropertyUtil::getAddOnContentRegistrationTypeAsString(mNacp.getAddOnContentRegistrationType()) << std::endl;
	}

	// Attribute
	if (mNacp.getAttribute().size() > 0)
	{
		std::cout << "  Attribute:" << std::endl;
		for (auto itr = mNacp.getAttribute().begin(); itr != mNacp.getAttribute().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getAttributeFlagAsString(*itr) << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Attribute:                              None" << std::endl;
	}

	// SupportedLanguage
	if (mNacp.getSupportedLanguage().size() > 0)
	{
		std::cout << "  SupportedLanguage:" << std::endl;
		for (auto itr = mNacp.getSupportedLanguage().begin(); itr != mNacp.getSupportedLanguage().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getLanguageAsString(*itr) << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  SupportedLanguage:                      None" << std::endl;
	}

	// ParentalControl
	if (mNacp.getParentalControl().size() > 0)
	{
		std::cout << "  ParentalControl:" << std::endl;
		for (auto itr = mNacp.getParentalControl().begin(); itr != mNacp.getParentalControl().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getParentalControlFlagAsString(*itr) << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  ParentalControl:                        None" << std::endl;
	}

	// Screenshot
	if (mNacp.getScreenshot() != nn::hac::nacp::Screenshot::Allow || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Screenshot:                             " << nn::hac::ApplicationControlPropertyUtil::getScreenshotAsString(mNacp.getScreenshot()) << std::endl;
	}

	// VideoCapture
	if (mNacp.getVideoCapture() != nn::hac::nacp::VideoCapture::Disable || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  VideoCapture:                           " << nn::hac::ApplicationControlPropertyUtil::getVideoCaptureAsString(mNacp.getVideoCapture()) << std::endl;
	}

	// DataLossConfirmation
	if (mNacp.getDataLossConfirmation() != nn::hac::nacp::DataLossConfirmation::None || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  DataLossConfirmation:                   " << nn::hac::ApplicationControlPropertyUtil::getDataLossConfirmationAsString(mNacp.getDataLossConfirmation()) << std::endl;
	}

	// PlayLogPolicy
	if (mNacp.getPlayLogPolicy() != nn::hac::nacp::PlayLogPolicy::All || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  PlayLogPolicy:                          " << nn::hac::ApplicationControlPropertyUtil::getPlayLogPolicyAsString(mNacp.getPlayLogPolicy()) << std::endl;
	}

	// PresenceGroupId
	if (mNacp.getPresenceGroupId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  PresenceGroupId:                        0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getPresenceGroupId() << std::endl;
	}

	// RatingAge
	if (mNacp.getRatingAge().size() > 0)
	{
		std::cout << "  RatingAge:" << std::endl;
		
		for (auto itr = mNacp.getRatingAge().begin(); itr != mNacp.getRatingAge().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getOrganisationAsString(itr->organisation) << ":" << std::endl;
			std::cout << "      Age: " << std::dec << (uint32_t)itr->age << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  RatingAge:                              None" << std::endl;
	}

	// DisplayVersion
	if (mNacp.getDisplayVersion().empty() == false)
	{
		std::cout << "  DisplayVersion:                         " << mNacp.getDisplayVersion() << std::endl;
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  DisplayVersion:                         (NotSet)" << std::endl;
	}

	// AddOnContentBaseId
	if (mNacp.getAddOnContentBaseId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  AddOnContentBaseId:                     0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getAddOnContentBaseId() << std::endl;
	}

	// SaveDataOwnerId
	if (mNacp.getSaveDataOwnerId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  SaveDataOwnerId:                        0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getSaveDataOwnerId() << std::endl;
	}

	// UserAccountSaveDataSize
	if (mNacp.getUserAccountSaveDataSize().size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  UserAccountSaveDataSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataSize().size) << std::endl;
	}

	// UserAccountSaveDataJournalSize
	if (mNacp.getUserAccountSaveDataSize().journal_size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  UserAccountSaveDataJournalSize:         " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataSize().journal_size) << std::endl;
	}

	// DeviceSaveDataSize
	if (mNacp.getDeviceSaveDataSize().size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  DeviceSaveDataSize:                     " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataSize().size) << std::endl;
	}

	// DeviceSaveDataJournalSize
	if (mNacp.getDeviceSaveDataSize().journal_size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  DeviceSaveDataJournalSize:              " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataSize().journal_size) << std::endl;
	}

	// BcatDeliveryCacheStorageSize
	if (mNacp.getBcatDeliveryCacheStorageSize() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  BcatDeliveryCacheStorageSize:           " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getBcatDeliveryCacheStorageSize()) << std::endl;
	}

	// ApplicationErrorCodeCategory
	if (mNacp.getApplicationErrorCodeCategory().empty() == false)
	{
		std::cout << "  ApplicationErrorCodeCategory:           " << mNacp.getApplicationErrorCodeCategory() << std::endl;
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  ApplicationErrorCodeCategory:           (NotSet)" << std::endl;
	}

	// LocalCommunicationId
	if (mNacp.getLocalCommunicationId().size() > 0)
	{
		std::cout << "  LocalCommunicationId:" << std::endl;
		for (auto itr = mNacp.getLocalCommunicationId().begin(); itr != mNacp.getLocalCommunicationId().end(); itr++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << *itr << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  LocalCommunicationId:                   None" << std::endl;
	}

	// LogoType
	//if (mNacp.getLogoType() != nn::hac::nacp::LogoType::Nintendo || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	//{
		std::cout << "  LogoType:                               " << nn::hac::ApplicationControlPropertyUtil::getLogoTypeAsString(mNacp.getLogoType()) << std::endl;
	//}

	// LogoHandling
	if (mNacp.getLogoHandling() != nn::hac::nacp::LogoHandling::Auto || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  LogoHandling:                           " << nn::hac::ApplicationControlPropertyUtil::getLogoHandlingAsString(mNacp.getLogoHandling()) << std::endl;
	}

	// RuntimeAddOnContentInstall
	if (mNacp.getRuntimeAddOnContentInstall() != nn::hac::nacp::RuntimeAddOnContentInstall::Deny || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  RuntimeAddOnContentInstall:             " << nn::hac::ApplicationControlPropertyUtil::getRuntimeAddOnContentInstallAsString(mNacp.getRuntimeAddOnContentInstall()) << std::endl;
	}

	// RuntimeParameterDelivery
	if (mNacp.getRuntimeParameterDelivery() != nn::hac::nacp::RuntimeParameterDelivery::Always || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  RuntimeParameterDelivery:               " << nn::hac::ApplicationControlPropertyUtil::getRuntimeParameterDeliveryAsString(mNacp.getRuntimeParameterDelivery()) << std::endl;
	}

	// CrashReport
	if (mNacp.getCrashReport() != nn::hac::nacp::CrashReport::Deny || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  CrashReport:                            " << nn::hac::ApplicationControlPropertyUtil::getCrashReportAsString(mNacp.getCrashReport()) << std::endl;
	}

	// Hdcp
	if (mNacp.getHdcp() != nn::hac::nacp::Hdcp::None || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Hdcp:                                   " << nn::hac::ApplicationControlPropertyUtil::getHdcpAsString(mNacp.getHdcp()) << std::endl;
	}

	// SeedForPsuedoDeviceId
	if (mNacp.getSeedForPsuedoDeviceId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  SeedForPsuedoDeviceId:                  0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getSeedForPsuedoDeviceId() << std::endl;
	}

	// BcatPassphase
	if (mNacp.getBcatPassphase().empty() == false)
	{
		std::cout << "  BcatPassphase:                          " << mNacp.getBcatPassphase() << std::endl;
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  BcatPassphase:                          (NotSet)" << std::endl;
	}

	// StartupUserAccountOption
	if (mNacp.getStartupUserAccountOption().size() > 0)
	{
		std::cout << "  StartupUserAccountOption:" << std::endl;
		for (auto itr = mNacp.getStartupUserAccountOption().begin(); itr != mNacp.getStartupUserAccountOption().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getStartupUserAccountOptionFlagAsString(*itr) << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  StartupUserAccountOption:               None" << std::endl;
	}

	// UserAccountSaveDataSizeMax
	if (mNacp.getUserAccountSaveDataMax().size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  UserAccountSaveDataSizeMax:             " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataMax().size) << std::endl;
	}

	// UserAccountSaveDataJournalSizeMax
	if (mNacp.getUserAccountSaveDataMax().journal_size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  UserAccountSaveDataJournalSizeMax:      " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataMax().journal_size) << std::endl;
	}

	// DeviceSaveDataSizeMax
	if (mNacp.getDeviceSaveDataMax().size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  DeviceSaveDataSizeMax:                  " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataMax().size) << std::endl;
	}

	// DeviceSaveDataJournalSizeMax
	if (mNacp.getDeviceSaveDataMax().journal_size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  DeviceSaveDataJournalSizeMax:           " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataMax().journal_size) << std::endl;
	}

	// TemporaryStorageSize
	if (mNacp.getTemporaryStorageSize() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  TemporaryStorageSize:                   " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getTemporaryStorageSize()) << std::endl;
	}

	// CacheStorageSize
	if (mNacp.getCacheStorageSize().size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  CacheStorageSize:                       " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageSize().size) << std::endl;
	}

	// CacheStorageJournalSize
	if (mNacp.getCacheStorageSize().journal_size != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  CacheStorageJournalSize:                " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageSize().journal_size) << std::endl;
	}

	// CacheStorageDataAndJournalSizeMax
	if (mNacp.getCacheStorageDataAndJournalSizeMax() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  CacheStorageDataAndJournalSizeMax:      " << nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageDataAndJournalSizeMax()) << std::endl;
	}

	// CacheStorageIndexMax
	if (mNacp.getCacheStorageIndexMax() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  CacheStorageIndexMax:                   0x" << std::hex << std::setw(4) << std::setfill('0') << mNacp.getCacheStorageIndexMax() << std::endl;
	}

	// PlayLogQueryableApplicationId
	if (mNacp.getPlayLogQueryableApplicationId().size() > 0)
	{
		std::cout << "  PlayLogQueryableApplicationId:" << std::endl;
		for (auto itr = mNacp.getPlayLogQueryableApplicationId().begin(); itr != mNacp.getPlayLogQueryableApplicationId().end(); itr++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << *itr << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  PlayLogQueryableApplicationId:          None" << std::endl;
	}

	// PlayLogQueryCapability
	if (mNacp.getPlayLogQueryCapability() != nn::hac::nacp::PlayLogQueryCapability::None || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  PlayLogQueryCapability:                 " << nn::hac::ApplicationControlPropertyUtil::getPlayLogQueryCapabilityAsString(mNacp.getPlayLogQueryCapability()) << std::endl;
	}

	// Repair
	if (mNacp.getRepair().size() > 0)
	{
		std::cout << "  Repair:" << std::endl;
		for (auto itr = mNacp.getRepair().begin(); itr != mNacp.getRepair().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getRepairFlagAsString(*itr) << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Repair:                                 None" << std::endl;
	}

	// ProgramIndex
	if (mNacp.getProgramIndex() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  ProgramIndex:                           0x" << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)mNacp.getProgramIndex() << std::endl;
	}

	// RequiredNetworkServiceLicenseOnLaunch
	if (mNacp.getRequiredNetworkServiceLicenseOnLaunch().size() > 0)
	{
		std::cout << "  RequiredNetworkServiceLicenseOnLaunch:" << std::endl;
		for (auto itr = mNacp.getRequiredNetworkServiceLicenseOnLaunch().begin(); itr != mNacp.getRequiredNetworkServiceLicenseOnLaunch().end(); itr++)
		{
			std::cout << "    " << nn::hac::ApplicationControlPropertyUtil::getRequiredNetworkServiceLicenseOnLaunchFlagAsString(*itr) << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  RequiredNetworkServiceLicenseOnLaunch:  None" << std::endl;
	}

	// NeighborDetectionClientConfiguration
	auto detect_config = mNacp.getNeighborDetectionClientConfiguration();
	if (detect_config.countSendGroupConfig() > 0 || detect_config.countReceivableGroupConfig() > 0)
	{
		std::cout << "  NeighborDetectionClientConfiguration:" << std::endl;
		if (detect_config.countSendGroupConfig() > 0)
		{
			std::cout << "    SendGroupConfig:" << std::endl;
			std::cout << "      GroupId:  0x" << std::hex << std::setw(16) << std::setfill('0') << detect_config.send_data_configuration.group_id << std::endl;
			std::cout << "        Key:    " << fnd::SimpleTextOutput::arrayToString(detect_config.send_data_configuration.key, nn::hac::nacp::kNeighborDetectionGroupConfigurationKeyLength, false, "") << std::endl;
		}
		else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		{
			std::cout << "    SendGroupConfig: None" << std::endl;
		}
		if (detect_config.countReceivableGroupConfig() > 0)
		{
			std::cout << "    ReceivableGroupConfig:" << std::endl;
			for (size_t i = 0; i < nn::hac::nacp::kReceivableGroupConfigurationCount; i++)
			{
				if (detect_config.receivable_data_configuration[i].isNull())
					continue;

				std::cout << "      GroupId:  0x" << std::hex << std::setw(16) << std::setfill('0') << detect_config.receivable_data_configuration[i].group_id << std::endl;
				std::cout << "        Key:    " << fnd::SimpleTextOutput::arrayToString(detect_config.receivable_data_configuration[i].key, nn::hac::nacp::kNeighborDetectionGroupConfigurationKeyLength, false, "") << std::endl;
			}
		}
		else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		{
			std::cout << "    ReceivableGroupConfig: None" << std::endl;
		}
	}
	else if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  NeighborDetectionClientConfiguration:   None" << std::endl;
	}
	
	// JitConfiguration
	if (mNacp.getJitConfiguration().is_enabled || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  JitConfiguration:" << std::endl;
		std::cout << "    IsEnabled:  " << std::boolalpha << mNacp.getJitConfiguration().is_enabled << std::endl;
		std::cout << "    MemorySize: 0x" << std::hex << std::setw(16) << std::setfill('0') << mNacp.getJitConfiguration().memory_size << std::endl;
	}
}