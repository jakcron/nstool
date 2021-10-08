#pragma once
#include "types.h"
#include "FsProcess.h"

#include <nn/hac/PartitionFsHeader.h>

namespace nstool {

class PfsProcess
{
public:
	PfsProcess();

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// fs specific
	void setShowFsTree(bool show_fs_tree);
	void setFsRootLabel(const std::string& root_label);
	void setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs);

	void process();

	// post process() get PFS/FS out
	const nn::hac::PartitionFsHeader& getPfsHeader() const;
	const std::shared_ptr<tc::io::IStorage>& getFileSystem() const;

private:
	static const size_t kCacheSize = 0x10000;

	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::PartitionFsHeader mPfs;

	std::shared_ptr<tc::io::IStorage> mFileSystem;
	FsProcess mFsProcess;
	
	size_t determineHeaderSize(const nn::hac::sPfsHeader* hdr);
	bool validateHeaderMagic(const nn::hac::sPfsHeader* hdr);
};

}