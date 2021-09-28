#pragma once
#include "types.h"
#include "RoMetadataProcess.h"

#include <nn/hac/define/meta.h>
#include <nn/hac/NsoHeader.h>

namespace nstool {

class NsoProcess
{
public:
	NsoProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setIs64BitInstruction(bool flag);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	const RoMetadataProcess& getRoMetadataProcess() const;
private:
	const std::string kModuleName = "NsoProcess";

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	bool mIs64BitInstruction;
	bool mListApi;
	bool mListSymbols;

	nn::hac::NsoHeader mHdr;
	tc::ByteData mTextBlob, mRoBlob, mDataBlob;
	RoMetadataProcess mRoMeta;

	void importHeader();
	void importCodeSegments();
	void displayNsoHeader();
	void processRoMeta();
};

}