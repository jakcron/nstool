#pragma once
#include "types.h"

#include <nn/hac/IniHeader.h>
#include <nn/hac/KernelInitialProcessHeader.h>

namespace nstool {

class IniProcess
{
public:
	IniProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setKipExtractPath(const std::string& path);
private:
	const std::string kModuleName = "IniProcess";
	const std::string kKipExtention = ".kip";
	const size_t kCacheSize = 0x10000;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	bool mDoExtractKip;
	std::string mKipExtractPath;

	nn::hac::IniHeader mHdr;
	std::vector<std::shared_ptr<tc::io::IStream>> mKipList;

	void importHeader();
	void importKipList();
	void displayHeader();
	void displayKipList();
	void extractKipList();

	size_t getKipSizeFromHeader(const nn::hac::KernelInitialProcessHeader& hdr) const;
};

}