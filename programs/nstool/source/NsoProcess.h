#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/npdm.h>
#include <nn/hac/NsoHeader.h>

#include "common.h"
#include "RoMetadataProcess.h"

class NsoProcess
{
public:
	NsoProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setInstructionType(nn::hac::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	const RoMetadataProcess& getRoMetadataProcess() const;
private:
	const std::string kModuleName = "NsoProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
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