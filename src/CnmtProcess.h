#pragma once
#include "types.h"

#include <pietendo/hac/ContentMeta.h>

namespace nstool {

class CnmtProcess
{
public:
	CnmtProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const pie::hac::ContentMeta& getContentMeta() const;
private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	pie::hac::ContentMeta mCnmt;

	void importCnmt();
	void displayCnmt();

	void displayContentMetaInfo(const pie::hac::ContentMetaInfo& content_meta_info, const std::string& prefix);
	void displayContentMetaInfoList(const std::vector<pie::hac::ContentMetaInfo>& content_meta_info_list, const std::string& prefix);
};

}