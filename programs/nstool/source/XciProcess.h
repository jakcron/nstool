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

	void process();

	// generic
	void setInputFile(fnd::IFile& reader);
	void setInputFileOffset(size_t offset);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	// xci specific
	void setUpdateExtractPath(const std::string& path);
	void setNormalExtractPath(const std::string& path);
	void setSecureExtractPath(const std::string& path);
	void setListFs(bool list_fs);

private:
	const std::string kModuleName = "XciProcess";
	const std::string kXciMountPointName = "gamecard:/";
	static const size_t kFileExportBlockSize = 0x1000000;

	fnd::IFile* mReader;
	size_t mOffset;
	const sKeyset* mKeyset;
	CliOutputType mCliOutputType;
	bool mVerify;

	struct sExtract
	{
		std::string path;
		bool doExtract;
	} mUpdatePath, mNormalPath, mSecurePath;

	bool mListFs;

	nx::sXciHeaderPage mHdrPage;
	nx::XciHeader mHdr;
	PfsProcess mRootPfs, mUpdatePfs, mNormalPfs, mSecurePfs;

	void displayHeader();
	bool validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash);
	void validateXciSignature();
	void processRootPfs();
	void processPartitionPfs();
};