#pragma once
#include "types.h"

#include <pietendo/hac/IniHeader.h>
#include <pietendo/hac/KernelInitialProcessHeader.h>

namespace nstool {

class IniProcess
{
public:
	IniProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setKipExtractPath(const tc::io::Path& path);
private:
	const size_t kCacheSize = 0x10000;

	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	tc::Optional<tc::io::Path> mKipExtractPath;

	pie::hac::IniHeader mHdr;
	struct InnerKipInfo
	{
		pie::hac::KernelInitialProcessHeader hdr;
		std::shared_ptr<tc::io::IStream> stream;
	};
	std::vector<InnerKipInfo> mKipList;

	void importHeader();
	void importKipList();
	void displayHeader();
	void displayKipList();
	void extractKipList();

	int64_t getKipSizeFromHeader(const pie::hac::KernelInitialProcessHeader& hdr) const;
};

}