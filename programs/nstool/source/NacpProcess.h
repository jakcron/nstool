#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/ApplicationControlPropertyBinary.h>

#include "common.h"

class NacpProcess
{
public:
	NacpProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::ApplicationControlPropertyBinary& getApplicationControlPropertyBinary() const;

private:
	const std::string kModuleName = "NacpProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::ApplicationControlPropertyBinary mNacp;

	void importNacp();
	void displayNacp();
	const char* getLanguageStr(nn::hac::nacp::Language var) const;
	const char* getStartupUserAccountStr(nn::hac::nacp::StartupUserAccount var) const;
	const char* getTouchScreenUsageModeStr(nn::hac::nacp::TouchScreenUsageMode var) const;
	const char* getAocRegistrationTypeStr(nn::hac::nacp::AocRegistrationType var) const;
	const char* getAttributeFlagStr(nn::hac::nacp::AttributeFlag var) const;
	const char* getParentalControlFlagStr(nn::hac::nacp::ParentalControlFlag var) const;
	const char* getScreenshotModeStr(nn::hac::nacp::ScreenshotMode var) const;
	const char* getVideoCaptureModeStr(nn::hac::nacp::VideoCaptureMode var) const;
	const char* getDataLossConfirmationStr(nn::hac::nacp::DataLossConfirmation var) const;
	const char* getPlayLogPolicyStr(nn::hac::nacp::PlayLogPolicy var) const;
	const char* getOrganisationStr(nn::hac::nacp::Organisation var) const;
	const char* getLogoTypeStr(nn::hac::nacp::LogoType var) const;
	const char* getLogoHandlingStr(nn::hac::nacp::LogoHandling var) const;
	const char* getRuntimeAocInstallModeStr(nn::hac::nacp::RuntimeAocInstallMode var) const;
	const char* getCrashReportModeStr(nn::hac::nacp::CrashReportMode var) const;
	const char* getHdcpStr(nn::hac::nacp::Hdcp var) const;
	const char* getPlayLogQueryCapabilityStr(nn::hac::nacp::PlayLogQueryCapability var) const;
	const char* getRepairFlagStr(nn::hac::nacp::RepairFlag var) const;
	std::string getSaveDataSizeStr(int64_t size) const;
};