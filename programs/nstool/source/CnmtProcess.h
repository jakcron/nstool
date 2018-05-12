#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/ContentMetaBinary.h>

#include "nstool.h"

class CnmtProcess
{
public:
	CnmtProcess();
	~CnmtProcess();

	void process();

	void setInputFile(fnd::IFile* file, size_t offset, size_t size);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	const nx::ContentMetaBinary& getContentMetaBinary() const;

private:
	const std::string kModuleName = "CnmtProcess";

	fnd::IFile* mReader;
	CliOutputType mCliOutputType;
	bool mVerify;

	nx::ContentMetaBinary mCnmt;

	void displayCmnt();
};