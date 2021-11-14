#include "NacpProcess.h"

#include <nn/hac/ApplicationControlPropertyUtil.h>

nstool::NacpProcess::NacpProcess() :
	mModuleName("nstool::NacpProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::NacpProcess::process()
{
	importNacp();

	if (mCliOutputMode.show_basic_info)
		displayNacp();
}

void nstool::NacpProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::NacpProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::NacpProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nn::hac::ApplicationControlProperty& nstool::NacpProcess::getApplicationControlProperty() const
{
	return mNacp;
}

void nstool::NacpProcess::importNacp()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size does matches expected size
	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());
	if (file_size != sizeof(nn::hac::sApplicationControlProperty))
	{
		throw tc::Exception(mModuleName, "File was incorrect size.");
	}

	// read cnmt
	tc::ByteData scratch = tc::ByteData(file_size);
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	mNacp.fromBytes(scratch.data(), scratch.size());
}

void nstool::NacpProcess::displayNacp()
{
	fmt::print("[ApplicationControlProperty]\n");
	
	// Title
	if (mNacp.getTitle().size() > 0)
	{
		fmt::print("  Title:\n");
		for (auto itr = mNacp.getTitle().begin(); itr != mNacp.getTitle().end(); itr++)
		{
			fmt::print("    {:s}:\n", nn::hac::ApplicationControlPropertyUtil::getLanguageAsString(itr->language));
			fmt::print("      Name:       {:s}\n", itr->name);
			fmt::print("      Publisher:  {:s}\n", itr->publisher);
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  Title:                                  None\n");
	}

	// Isbn
	if (mNacp.getIsbn().empty() == false)
	{
		fmt::print("  ISBN:                                   {:s}\n", mNacp.getIsbn());
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  ISBN:                                   (NotSet)\n");
	}
	
	// StartupUserAccount
	if (mNacp.getStartupUserAccount() != nn::hac::nacp::StartupUserAccount::None || mCliOutputMode.show_extended_info)
	{
		fmt::print("  StartupUserAccount:                     {:s}\n", nn::hac::ApplicationControlPropertyUtil::getStartupUserAccountAsString(mNacp.getStartupUserAccount()));
	}

	// UserAccountSwitchLock
	if (mNacp.getUserAccountSwitchLock() != nn::hac::nacp::UserAccountSwitchLock::Disable || mCliOutputMode.show_extended_info)
	{
		fmt::print("  UserAccountSwitchLock:                  {:s}\n", nn::hac::ApplicationControlPropertyUtil::getUserAccountSwitchLockAsString(mNacp.getUserAccountSwitchLock()));
	}

	// AddOnContentRegistrationType
	if (mNacp.getAddOnContentRegistrationType() != nn::hac::nacp::AddOnContentRegistrationType::AllOnLaunch || mCliOutputMode.show_extended_info)
	{
		fmt::print("  AddOnContentRegistrationType:           {:s}\n", nn::hac::ApplicationControlPropertyUtil::getAddOnContentRegistrationTypeAsString(mNacp.getAddOnContentRegistrationType()));
	}

	// Attribute
	if (mNacp.getAttribute().size() > 0)
	{
		fmt::print("  Attribute:\n");
		for (auto itr = mNacp.getAttribute().begin(); itr != mNacp.getAttribute().end(); itr++)
		{
			fmt::print("    {:s}\n", nn::hac::ApplicationControlPropertyUtil::getAttributeFlagAsString(*itr));
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  Attribute:                              None\n");
	}

	// SupportedLanguage
	if (mNacp.getSupportedLanguage().size() > 0)
	{
		fmt::print("  SupportedLanguage:\n");
		for (auto itr = mNacp.getSupportedLanguage().begin(); itr != mNacp.getSupportedLanguage().end(); itr++)
		{
			fmt::print("    {:s}\n", nn::hac::ApplicationControlPropertyUtil::getLanguageAsString(*itr));
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  SupportedLanguage:                      None\n");
	}

	// ParentalControl
	if (mNacp.getParentalControl().size() > 0)
	{
		fmt::print("  ParentalControl:\n");
		for (auto itr = mNacp.getParentalControl().begin(); itr != mNacp.getParentalControl().end(); itr++)
		{
			fmt::print("    {:s}\n", nn::hac::ApplicationControlPropertyUtil::getParentalControlFlagAsString(*itr));
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  ParentalControl:                        None\n");
	}

	// Screenshot
	if (mNacp.getScreenshot() != nn::hac::nacp::Screenshot::Allow || mCliOutputMode.show_extended_info)
	{
		fmt::print("  Screenshot:                             {:s}\n", nn::hac::ApplicationControlPropertyUtil::getScreenshotAsString(mNacp.getScreenshot()));
	}

	// VideoCapture
	if (mNacp.getVideoCapture() != nn::hac::nacp::VideoCapture::Disable || mCliOutputMode.show_extended_info)
	{
		fmt::print("  VideoCapture:                           {:s}\n", nn::hac::ApplicationControlPropertyUtil::getVideoCaptureAsString(mNacp.getVideoCapture()));
	}

	// DataLossConfirmation
	if (mNacp.getDataLossConfirmation() != nn::hac::nacp::DataLossConfirmation::None || mCliOutputMode.show_extended_info)
	{
		fmt::print("  DataLossConfirmation:                   {:s}\n", nn::hac::ApplicationControlPropertyUtil::getDataLossConfirmationAsString(mNacp.getDataLossConfirmation()));
	}

	// PlayLogPolicy
	if (mNacp.getPlayLogPolicy() != nn::hac::nacp::PlayLogPolicy::All || mCliOutputMode.show_extended_info)
	{
		fmt::print("  PlayLogPolicy:                          {:s}\n", nn::hac::ApplicationControlPropertyUtil::getPlayLogPolicyAsString(mNacp.getPlayLogPolicy()));
	}

	// PresenceGroupId
	if (mNacp.getPresenceGroupId() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  PresenceGroupId:                        0x{:016x}\n", mNacp.getPresenceGroupId());
	}

	// RatingAge
	if (mNacp.getRatingAge().size() > 0)
	{
		fmt::print("  RatingAge:\n");
		
		for (auto itr = mNacp.getRatingAge().begin(); itr != mNacp.getRatingAge().end(); itr++)
		{
			fmt::print("    {:s}:\n", nn::hac::ApplicationControlPropertyUtil::getOrganisationAsString(itr->organisation));
			fmt::print("      Age: {:d}\n", itr->age);
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  RatingAge:                              None\n");
	}

	// DisplayVersion
	if (mNacp.getDisplayVersion().empty() == false)
	{
		fmt::print("  DisplayVersion:                         {:s}\n", mNacp.getDisplayVersion());
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  DisplayVersion:                         (NotSet)\n");
	}

	// AddOnContentBaseId
	if (mNacp.getAddOnContentBaseId() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  AddOnContentBaseId:                     0x{:016x}\n", mNacp.getAddOnContentBaseId());
	}

	// SaveDataOwnerId
	if (mNacp.getSaveDataOwnerId() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  SaveDataOwnerId:                        0x{:016x}\n", mNacp.getSaveDataOwnerId());
	}

	// UserAccountSaveDataSize
	if (mNacp.getUserAccountSaveDataSize().size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  UserAccountSaveDataSize:                {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataSize().size));
	}

	// UserAccountSaveDataJournalSize
	if (mNacp.getUserAccountSaveDataSize().journal_size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  UserAccountSaveDataJournalSize:         {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataSize().journal_size));
	}

	// DeviceSaveDataSize
	if (mNacp.getDeviceSaveDataSize().size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  DeviceSaveDataSize:                     {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataSize().size));
	}

	// DeviceSaveDataJournalSize
	if (mNacp.getDeviceSaveDataSize().journal_size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  DeviceSaveDataJournalSize:              {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataSize().journal_size));
	}

	// BcatDeliveryCacheStorageSize
	if (mNacp.getBcatDeliveryCacheStorageSize() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  BcatDeliveryCacheStorageSize:           {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getBcatDeliveryCacheStorageSize()));
	}

	// ApplicationErrorCodeCategory
	if (mNacp.getApplicationErrorCodeCategory().empty() == false)
	{
		fmt::print("  ApplicationErrorCodeCategory:           {:s}\n", mNacp.getApplicationErrorCodeCategory());
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  ApplicationErrorCodeCategory:           (NotSet)\n");
	}

	// LocalCommunicationId
	if (mNacp.getLocalCommunicationId().size() > 0)
	{
		fmt::print("  LocalCommunicationId:\n");
		for (auto itr = mNacp.getLocalCommunicationId().begin(); itr != mNacp.getLocalCommunicationId().end(); itr++)
		{
			fmt::print("    0x{:016x}\n", *itr);
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  LocalCommunicationId:                   None\n");
	}

	// LogoType
	//if (mNacp.getLogoType() != nn::hac::nacp::LogoType::Nintendo || mCliOutputMode.show_extended_info)
	//{
		fmt::print("  LogoType:                               {:s}\n", nn::hac::ApplicationControlPropertyUtil::getLogoTypeAsString(mNacp.getLogoType()));
	//}

	// LogoHandling
	if (mNacp.getLogoHandling() != nn::hac::nacp::LogoHandling::Auto || mCliOutputMode.show_extended_info)
	{
		fmt::print("  LogoHandling:                           {:s}\n", nn::hac::ApplicationControlPropertyUtil::getLogoHandlingAsString(mNacp.getLogoHandling()));
	}

	// RuntimeAddOnContentInstall
	if (mNacp.getRuntimeAddOnContentInstall() != nn::hac::nacp::RuntimeAddOnContentInstall::Deny || mCliOutputMode.show_extended_info)
	{
		fmt::print("  RuntimeAddOnContentInstall:             {:s}\n", nn::hac::ApplicationControlPropertyUtil::getRuntimeAddOnContentInstallAsString(mNacp.getRuntimeAddOnContentInstall()));
	}

	// RuntimeParameterDelivery
	if (mNacp.getRuntimeParameterDelivery() != nn::hac::nacp::RuntimeParameterDelivery::Always || mCliOutputMode.show_extended_info)
	{
		fmt::print("  RuntimeParameterDelivery:               {:s}\n", nn::hac::ApplicationControlPropertyUtil::getRuntimeParameterDeliveryAsString(mNacp.getRuntimeParameterDelivery()));
	}

	// CrashReport
	if (mNacp.getCrashReport() != nn::hac::nacp::CrashReport::Deny || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CrashReport:                            {:s}\n", nn::hac::ApplicationControlPropertyUtil::getCrashReportAsString(mNacp.getCrashReport()));
	}

	// Hdcp
	if (mNacp.getHdcp() != nn::hac::nacp::Hdcp::None || mCliOutputMode.show_extended_info)
	{
		fmt::print("  Hdcp:                                   {:s}\n", nn::hac::ApplicationControlPropertyUtil::getHdcpAsString(mNacp.getHdcp()));
	}

	// SeedForPsuedoDeviceId
	if (mNacp.getSeedForPsuedoDeviceId() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  SeedForPsuedoDeviceId:                  0x{:016x}\n", mNacp.getSeedForPsuedoDeviceId());
	}

	// BcatPassphase
	if (mNacp.getBcatPassphase().empty() == false)
	{
		fmt::print("  BcatPassphase:                          {:s}\n", mNacp.getBcatPassphase());
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  BcatPassphase:                          (NotSet)\n");
	}

	// StartupUserAccountOption
	if (mNacp.getStartupUserAccountOption().size() > 0)
	{
		fmt::print("  StartupUserAccountOption:\n");
		for (auto itr = mNacp.getStartupUserAccountOption().begin(); itr != mNacp.getStartupUserAccountOption().end(); itr++)
		{
			fmt::print("    {:s}\n", nn::hac::ApplicationControlPropertyUtil::getStartupUserAccountOptionFlagAsString(*itr));
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  StartupUserAccountOption:               None\n");
	}

	// UserAccountSaveDataSizeMax
	if (mNacp.getUserAccountSaveDataMax().size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  UserAccountSaveDataSizeMax:             {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataMax().size));
	}

	// UserAccountSaveDataJournalSizeMax
	if (mNacp.getUserAccountSaveDataMax().journal_size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  UserAccountSaveDataJournalSizeMax:      {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getUserAccountSaveDataMax().journal_size));
	}

	// DeviceSaveDataSizeMax
	if (mNacp.getDeviceSaveDataMax().size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  DeviceSaveDataSizeMax:                  {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataMax().size));
	}

	// DeviceSaveDataJournalSizeMax
	if (mNacp.getDeviceSaveDataMax().journal_size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  DeviceSaveDataJournalSizeMax:           {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getDeviceSaveDataMax().journal_size));
	}

	// TemporaryStorageSize
	if (mNacp.getTemporaryStorageSize() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  TemporaryStorageSize:                   {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getTemporaryStorageSize()));
	}

	// CacheStorageSize
	if (mNacp.getCacheStorageSize().size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CacheStorageSize:                       {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageSize().size));
	}

	// CacheStorageJournalSize
	if (mNacp.getCacheStorageSize().journal_size != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CacheStorageJournalSize:                {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageSize().journal_size));
	}

	// CacheStorageDataAndJournalSizeMax
	if (mNacp.getCacheStorageDataAndJournalSizeMax() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CacheStorageDataAndJournalSizeMax:      {:s}\n", nn::hac::ApplicationControlPropertyUtil::getSaveDataSizeAsString(mNacp.getCacheStorageDataAndJournalSizeMax()));
	}

	// CacheStorageIndexMax
	if (mNacp.getCacheStorageIndexMax() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CacheStorageIndexMax:                   0x{:04x}\n", mNacp.getCacheStorageIndexMax());
	}

	// PlayLogQueryableApplicationId
	if (mNacp.getPlayLogQueryableApplicationId().size() > 0)
	{
		fmt::print("  PlayLogQueryableApplicationId:\n");
		for (auto itr = mNacp.getPlayLogQueryableApplicationId().begin(); itr != mNacp.getPlayLogQueryableApplicationId().end(); itr++)
		{
			fmt::print("    0x{:016x}\n", *itr);
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  PlayLogQueryableApplicationId:          None\n");
	}

	// PlayLogQueryCapability
	if (mNacp.getPlayLogQueryCapability() != nn::hac::nacp::PlayLogQueryCapability::None || mCliOutputMode.show_extended_info)
	{
		fmt::print("  PlayLogQueryCapability:                 {:s}\n", nn::hac::ApplicationControlPropertyUtil::getPlayLogQueryCapabilityAsString(mNacp.getPlayLogQueryCapability()));
	}

	// Repair
	if (mNacp.getRepair().size() > 0)
	{
		fmt::print("  Repair:\n");
		for (auto itr = mNacp.getRepair().begin(); itr != mNacp.getRepair().end(); itr++)
		{
			fmt::print("    {:s}\n", nn::hac::ApplicationControlPropertyUtil::getRepairFlagAsString(*itr));
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  Repair:                                 None\n");
	}

	// ProgramIndex
	if (mNacp.getProgramIndex() != 0 || mCliOutputMode.show_extended_info)
	{
		fmt::print("  ProgramIndex:                           0x{:02x}\n", mNacp.getProgramIndex());
	}

	// RequiredNetworkServiceLicenseOnLaunch
	if (mNacp.getRequiredNetworkServiceLicenseOnLaunch().size() > 0)
	{
		fmt::print("  RequiredNetworkServiceLicenseOnLaunch:\n");
		for (auto itr = mNacp.getRequiredNetworkServiceLicenseOnLaunch().begin(); itr != mNacp.getRequiredNetworkServiceLicenseOnLaunch().end(); itr++)
		{
			fmt::print("    {:s}\n", nn::hac::ApplicationControlPropertyUtil::getRequiredNetworkServiceLicenseOnLaunchFlagAsString(*itr));
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  RequiredNetworkServiceLicenseOnLaunch:  None\n");
	}

	// NeighborDetectionClientConfiguration
	auto detect_config = mNacp.getNeighborDetectionClientConfiguration();
	if (detect_config.countSendGroupConfig() > 0 || detect_config.countReceivableGroupConfig() > 0)
	{
		fmt::print("  NeighborDetectionClientConfiguration:\n");
		if (detect_config.countSendGroupConfig() > 0)
		{
			fmt::print("    SendGroupConfig:\n");
			fmt::print("      GroupId:  0x{:016x}\n", detect_config.send_data_configuration.group_id);
			fmt::print("        Key:    {:s}\n", tc::cli::FormatUtil::formatBytesAsString(detect_config.send_data_configuration.key.data(), detect_config.send_data_configuration.key.size(), false, ""));
		}
		else if (mCliOutputMode.show_extended_info)
		{
			fmt::print("    SendGroupConfig: None\n");
		}
		if (detect_config.countReceivableGroupConfig() > 0)
		{
			fmt::print("    ReceivableGroupConfig:\n");
			for (size_t i = 0; i < nn::hac::nacp::kReceivableGroupConfigurationCount; i++)
			{
				if (detect_config.receivable_data_configuration[i].isNull())
					continue;

				fmt::print("      GroupId:  0x{:016x}\n", detect_config.receivable_data_configuration[i].group_id);
				fmt::print("        Key:    {:s}\n", tc::cli::FormatUtil::formatBytesAsString(detect_config.receivable_data_configuration[i].key.data(), detect_config.receivable_data_configuration[i].key.size(), false, ""));
			}
		}
		else if (mCliOutputMode.show_extended_info)
		{
			fmt::print("    ReceivableGroupConfig: None\n");
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  NeighborDetectionClientConfiguration:   None\n");
	}
	
	// JitConfiguration
	if (mNacp.getJitConfiguration().is_enabled || mCliOutputMode.show_extended_info)
	{
		fmt::print("  JitConfiguration:\n");
		fmt::print("    IsEnabled:  {}\n", mNacp.getJitConfiguration().is_enabled);
		fmt::print("    MemorySize: 0x{:016x}\n", mNacp.getJitConfiguration().memory_size);
	}
	
	// PlayReportPermission
	if (mNacp.getPlayReportPermission() != nn::hac::nacp::PlayReportPermission::None || mCliOutputMode.show_extended_info)
	{
		fmt::print("  PlayReportPermission:                   {:s}\n", nn::hac::ApplicationControlPropertyUtil::getPlayReportPermissionAsString(mNacp.getPlayReportPermission()));
	}

	// CrashScreenshotForProd
	if (mNacp.getCrashScreenshotForProd() != nn::hac::nacp::CrashScreenshotForProd::Deny || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CrashScreenshotForProd:                 {:s}\n", nn::hac::ApplicationControlPropertyUtil::getCrashScreenshotForProdAsString(mNacp.getCrashScreenshotForProd()));
	}

	// CrashScreenshotForDev
	if (mNacp.getCrashScreenshotForDev() != nn::hac::nacp::CrashScreenshotForDev::Deny || mCliOutputMode.show_extended_info)
	{
		fmt::print("  CrashScreenshotForDev:                  {:s}\n", nn::hac::ApplicationControlPropertyUtil::getCrashScreenshotForDevAsString(mNacp.getCrashScreenshotForDev()));
	}

	// AccessibleLaunchRequiredVersion
	if (mNacp.getAccessibleLaunchRequiredVersionApplicationId().size() > 0)
	{
		fmt::print("  AccessibleLaunchRequiredVersion:\n");
		fmt::print("    ApplicationId:\n");
		for (auto itr = mNacp.getAccessibleLaunchRequiredVersionApplicationId().begin(); itr != mNacp.getAccessibleLaunchRequiredVersionApplicationId().end(); itr++)
		{
			fmt::print("      0x{:016x}\n", *itr);
		}
	}
	else if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  AccessibleLaunchRequiredVersion:        None\n");
	}
}