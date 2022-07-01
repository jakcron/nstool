#pragma once
#include "types.h"
#include "RoMetadataProcess.h"

#include <pietendo/hac/define/meta.h>
#include <pietendo/hac/NsoHeader.h>

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

	const nstool::RoMetadataProcess& getRoMetadataProcess() const;
private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	bool mIs64BitInstruction;
	bool mListApi;
	bool mListSymbols;

	pie::hac::NsoHeader mHdr;
	tc::ByteData mTextBlob, mRoBlob, mDataBlob;
	nstool::RoMetadataProcess mRoMeta;

	void importHeader();
	void importCodeSegments();
	void displayNsoHeader();
	void processRoMeta();

	size_t decompressData(const byte_t* src, size_t src_len, byte_t* dst, size_t dst_capacity);
};

}