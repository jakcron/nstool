#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/AssetHeader.h>
#include "NacpProcess.h"
#include "RomfsProcess.h"

#include "common.h"

class AssetProcess
{
public:
	AssetProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setListFs(bool list);

	void setIconExtractPath(const std::string& path);
	void setNacpExtractPath(const std::string& path);
	void setRomfsExtractPath(const std::string& path);


private:
	const std::string kModuleName = "AssetProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	sOptional<std::string> mIconExtractPath;
	sOptional<std::string> mNacpExtractPath;

	nn::hac::AssetHeader mHdr;
	NacpProcess mNacp;
	RomfsProcess mRomfs;

	void importHeader();
	void processSections();
	void displayHeader();
};