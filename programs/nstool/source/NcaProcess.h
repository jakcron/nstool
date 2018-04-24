#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/SimpleFile.h>
#include <nx/nca.h>
#include <nx/NcaHeader.h>

#include "nstool.h"

class NcaProcess
{
public:
	NcaProcess();
	~NcaProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile& reader);
	void setInputFileOffset(size_t offset);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	// nca specfic

private:
	const std::string kModuleName = "NcaProcess";

	byte_t mRawHeader[nx::nca::kHeaderSize];
	std::string mPath;
	const sKeyset* mKeyset;
};