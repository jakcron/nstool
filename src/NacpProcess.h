#pragma once
#include "types.h"

#include <nn/hac/ApplicationControlProperty.h>

namespace nstool {

class NacpProcess
{
public:
	NacpProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::ApplicationControlProperty& getApplicationControlProperty() const;

private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::ApplicationControlProperty mNacp;

	void importNacp();
	void displayNacp();
};

}