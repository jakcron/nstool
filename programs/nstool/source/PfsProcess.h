#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/PfsHeader.h>

#include "nstool.h"

class PfsProcess
{
public:
	PfsProcess();
	~PfsProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile* file, size_t offset, size_t size);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	// pfs specific
	void setMountPointName(const std::string& mount_name);
	void setExtractPath(const std::string& path);
	void setListFs(bool list_fs);

	const nx::PfsHeader& getPfsHeader() const;

private:
	const std::string kModuleName = "PfsProcess";
	static const size_t kFileExportBlockSize = 0x1000000;

	fnd::IFile* mReader;
	CliOutputType mCliOutputType;
	bool mVerify;

	std::string mExtractPath;
	bool mExtract;
	std::string mMountName;
	bool mListFs;

	nx::PfsHeader mPfs;

	void displayHeader();
	void displayFs();
	size_t determineHeaderSize(const nx::sPfsHeader* hdr);
	bool validateHeaderMagic(const nx::sPfsHeader* hdr);
	void validateHfs();
	void extractFs();
};