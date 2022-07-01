#pragma once
#include "types.h"
#include "NacpProcess.h"
#include "RomfsProcess.h"

#include <pietendo/hac/AssetHeader.h>

namespace nstool {

class AssetProcess
{
public:
	AssetProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setIconExtractPath(const tc::io::Path& path);
	void setNacpExtractPath(const tc::io::Path& path);
	
	void setRomfsShowFsTree(bool show_fs_tree);
	void setRomfsExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs);
private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	tc::Optional<tc::io::Path> mIconExtractPath;
	tc::Optional<tc::io::Path> mNacpExtractPath;

	pie::hac::AssetHeader mHdr;
	NacpProcess mNacp;
	RomfsProcess mRomfs;

	void importHeader();
	void processSections();
	void displayHeader();
};

}