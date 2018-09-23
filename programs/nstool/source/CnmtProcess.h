#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/ContentMetaBinary.h>

#include "common.h"

class CnmtProcess
{
public:
	CnmtProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::ContentMetaBinary& getContentMetaBinary() const;

private:
	const std::string kModuleName = "CnmtProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::ContentMetaBinary mCnmt;

	void importCnmt();
	void displayCnmt();

	const char* getBoolStr(bool state) const;
	const char* getContentTypeStr(nn::hac::cnmt::ContentType type) const;
	const char* getContentMetaTypeStr(nn::hac::cnmt::ContentMetaType type) const;
	const char* getUpdateTypeStr(nn::hac::cnmt::UpdateType type) const;
	const char* getContentMetaAttrStr(nn::hac::cnmt::ContentMetaAttribute attr) const;
};