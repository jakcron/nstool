#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/ContentMeta.h>

#include "common.h"

class CnmtProcess
{
public:
	CnmtProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::ContentMeta& getContentMeta() const;

private:
	const std::string kModuleName = "CnmtProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::ContentMeta mCnmt;

	void importCnmt();
	void displayCnmt();

	void displayContentMetaInfo(const nn::hac::ContentMetaInfo& content_meta_info, const std::string& prefix);
	void displayContentMetaInfoList(const std::vector<nn::hac::ContentMetaInfo>& content_meta_info_list, const std::string& prefix);
};