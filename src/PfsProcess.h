#pragma once
#include "types.h"
#include "FsProcess.h"

#include <nn/hac/PartitionFsHeader.h>

namespace nstool {

class PfsProcess
{
public:
	PfsProcess();

	void process();

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// pfs specific
	void setMountPointName(const std::string& mount_name);
	void setExtractPath(const tc::io::Path& path);
	void setListFs(bool list_fs);

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

	void importHeader();
	void displayHeader();
	size_t determineHeaderSize(const nn::hac::sPfsHeader* hdr);
	bool validateHeaderMagic(const nn::hac::sPfsHeader* hdr);
};

}