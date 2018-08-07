#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nn/hac/PfsHeader.h>

#include "nstool.h"

class PfsProcess
{
public:
	PfsProcess();
	~PfsProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile* file, bool ownIFile);
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

	fnd::IFile* mFile;
	bool mOwnIFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	std::string mExtractPath;
	bool mExtract;
	std::string mMountName;
	bool mListFs;

	fnd::Vec<byte_t> mCache;

	nn::hac::PfsHeader mPfs;

	void displayHeader();
	void displayFs();
	size_t determineHeaderSize(const nn::hac::sPfsHeader* hdr);
	bool validateHeaderMagic(const nn::hac::sPfsHeader* hdr);
	void validateHfs();
	void extractFs();
};