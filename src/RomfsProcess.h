#pragma once
#include "types.h"
#include "FsProcess.h"

#include <pietendo/hac/define/romfs.h>

namespace nstool {

class RomfsProcess
{
public:
	RomfsProcess();

	void process();

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// fs specific
	void setFsRootLabel(const std::string& root_label);
	void setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs);
	void setShowFsTree(bool show_fs_tree);
private:
	static const size_t kCacheSize = 0x10000;

	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	pie::hac::sRomfsHeader mRomfsHeader;
	size_t mDirNum;
	size_t mFileNum;

	std::shared_ptr<tc::io::IFileSystem> mFileSystem;
	FsProcess mFsProcess;
};

}