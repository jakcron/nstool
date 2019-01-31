#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/List.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/IniHeader.h>
#include <nn/hac/KernelInitialProcessHeader.h>

#include "common.h"

class IniProcess
{
public:
	IniProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setKipExtractPath(const std::string& path);
private:
	const std::string kModuleName = "IniProcess";
	const std::string kKipExtention = ".kip";
	const size_t kCacheSize = 0x10000;

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	bool mDoExtractKip;
	std::string mKipExtractPath;

	nn::hac::IniHeader mHdr;
	fnd::List<fnd::SharedPtr<fnd::IFile>> mKipList;

	void importHeader();
	void importKipList();
	void displayHeader();
	void displayKipList();
	void extractKipList();

	size_t getKipSizeFromHeader(const nn::hac::KernelInitialProcessHeader& hdr) const;
};