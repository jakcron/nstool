#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nn/hac/ApplicationControlPropertyBinary.h>

#include "nstool.h"

class NacpProcess
{
public:
	NacpProcess();
	~NacpProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::ApplicationControlPropertyBinary& getApplicationControlPropertyBinary() const;

private:
	const std::string kModuleName = "NacpProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::ApplicationControlPropertyBinary mNacp;

	void displayNacp();
};