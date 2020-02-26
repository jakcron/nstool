#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/ApplicationControlProperty.h>

#include "common.h"

class NacpProcess
{
public:
	NacpProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::ApplicationControlProperty& getApplicationControlProperty() const;

private:
	const std::string kModuleName = "NacpProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::ApplicationControlProperty mNacp;

	void importNacp();
	void displayNacp();
};