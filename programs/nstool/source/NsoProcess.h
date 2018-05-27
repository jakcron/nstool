#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/nso.h>

#include "nstool.h"

class NsoProcess
{
public:
	NsoProcess();
	~NsoProcess();

	void process();

	void setInputFile(fnd::IFile* file, size_t offset, size_t size);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);


private:
	const std::string kModuleName = "NsoProcess";

	fnd::IFile* mReader;
	CliOutputType mCliOutputType;
	bool mVerify;

	nx::sNsoHeader mNso;

	void displayHeader();
};