#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/SimpleFile.h>
#include <nx/NcaHeader.h>

#include "nstool.h"

class NcaProcess
{
public:
	NcaProcess();

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

	fnd::IFile* mReader;
	size_t mOffset;
	const sKeyset* mKeyset;
	CliOutputType mCliOutputType;
	bool mVerify;

	nx::sNcaHeaderBlock mHdrBlock;
	nx::NcaHeader mHdr;

	fnd::List<crypto::aes::sAes128Key> mBodyKeyList;


	void displayHeader();

	void decryptBodyKeyList();
};