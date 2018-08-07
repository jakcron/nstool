#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nn/hac/AssetHeader.h>
#include "NacpProcess.h"
#include "RomfsProcess.h"

#include "nstool.h"

class AssetProcess
{
public:
	AssetProcess();
	~AssetProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setListFs(bool list);

	void setIconExtractPath(const std::string& path);
	void setNacpExtractPath(const std::string& path);
	void setRomfsExtractPath(const std::string& path);


private:
	const std::string kModuleName = "AssetProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
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