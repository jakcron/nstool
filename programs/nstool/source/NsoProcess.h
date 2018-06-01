#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/NsoHeader.h>

#include "nstool.h"

class NsoProcess
{
public:
	NsoProcess();
	~NsoProcess();

	void process();

	void setInputFile(fnd::IFile* file, size_t offset, size_t size);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	// processed data
	const nx::NsoHeader& getNsoHeader() const;
	const fnd::MemoryBlob& getTextBlob() const;
	const fnd::MemoryBlob& getRoBlob() const;
	const fnd::MemoryBlob& getDataBlob() const;
	const std::vector<std::string>& getApiList() const;

private:
	const std::string kModuleName = "NsoProcess";

	fnd::IFile* mReader;
	CliOutputType mCliOutputType;
	bool mVerify;

	nx::NsoHeader mHdr;
	fnd::MemoryBlob mTextBlob, mRoBlob, mDataBlob;
	std::vector<std::string> mApiList;

	void importHeader();
	void importCodeSegments();
	void importApiList();
	void displayHeader();
};