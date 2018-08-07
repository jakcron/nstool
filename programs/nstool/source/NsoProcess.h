#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nn/hac/npdm.h>
#include <nn/hac/NsoHeader.h>

#include "nstool.h"
#include "RoMetadataProcess.h"

class NsoProcess
{
public:
	NsoProcess();
	~NsoProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setInstructionType(nn::hac::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	const RoMetadataProcess& getRoMetadataProcess() const;
private:
	const std::string kModuleName = "NsoProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;

	CliOutputMode mCliOutputMode;
	bool mVerify;
	nn::hac::npdm::InstructionType mInstructionType;
	bool mListApi;
	bool mListSymbols;

	nn::hac::NsoHeader mHdr;
	fnd::Vec<byte_t> mTextBlob, mRoBlob, mDataBlob;
	RoMetadataProcess mRoMeta;

	void importHeader();
	void importCodeSegments();
	void displayNsoHeader();
	void processRoMeta();
};