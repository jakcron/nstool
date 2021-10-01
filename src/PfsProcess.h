#pragma once
#include "types.h"

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

private:
	const std::string kModuleName = "PfsProcess";
	static const size_t kCacheSize = 0x10000;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	tc::Optional<tc::io::Path> mExtractPath;
	std::string mMountName;
	bool mListFs;

	tc::ByteData mCache;

	nn::hac::PartitionFsHeader mPfs;

	void importHeader();
	void displayHeader();
	void displayFs();
	size_t determineHeaderSize(const nn::hac::sPfsHeader* hdr);
	bool validateHeaderMagic(const nn::hac::sPfsHeader* hdr);
	void validateHfs();
	void extractFs();
};

}