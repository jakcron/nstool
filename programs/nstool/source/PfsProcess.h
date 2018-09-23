#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/PfsHeader.h>

#include "common.h"

class PfsProcess
{
public:
	PfsProcess();

	void process();

	// generic
	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// pfs specific
	void setMountPointName(const std::string& mount_name);
	void setExtractPath(const std::string& path);
	void setListFs(bool list_fs);

	const nn::hac::PfsHeader& getPfsHeader() const;

private:
	const std::string kModuleName = "PfsProcess";
	static const size_t kCacheSize = 0x10000;

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	std::string mExtractPath;
	bool mExtract;
	std::string mMountName;
	bool mListFs;

	fnd::Vec<byte_t> mCache;

	nn::hac::PfsHeader mPfs;

	void importHeader();
	void displayHeader();
	void displayFs();
	size_t determineHeaderSize(const nn::hac::sPfsHeader* hdr);
	bool validateHeaderMagic(const nn::hac::sPfsHeader* hdr);
	void validateHfs();
	void extractFs();

	const char* getFsTypeStr(nn::hac::PfsHeader::FsType type) const;
};