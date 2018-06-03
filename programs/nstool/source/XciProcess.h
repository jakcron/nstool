#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/XciHeader.h>

#include "nstool.h"

#include "PfsProcess.h"


class XciProcess
{
public:
	XciProcess();
	~XciProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	// xci specific
	void setPartitionForExtract(const std::string& partition_name, const std::string& extract_path);
	void setListFs(bool list_fs);

private:
	const std::string kModuleName = "XciProcess";
	const std::string kXciMountPointName = "gamecard:/";
	static const size_t kFileExportBlockSize = 0x1000000;

	fnd::IFile* mFile;
	bool mOwnIFile;
	const sKeyset* mKeyset;
	CliOutputType mCliOutputType;
	bool mVerify;

	struct sExtractInfo
	{
		std::string partition_name;
		std::string extract_path;
	};

	bool mListFs;

	nx::sXciHeaderPage mHdrPage;
	nx::XciHeader mHdr;
	PfsProcess mRootPfs;
	std::vector<sExtractInfo> mExtractInfo;

	void displayHeader();
	bool validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash);
	void validateXciSignature();
	void processRootPfs();
	void processPartitionPfs();
};