#pragma once
#include "types.h"

#include <pietendo/hac/KernelInitialProcessHeader.h>

namespace nstool {

class KipProcess
{
public:
	KipProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);
private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	pie::hac::KernelInitialProcessHeader mHdr;
	tc::ByteData mTextBlob, mRoBlob, mDataBlob;

	void importHeader();
	void importCodeSegments();
	size_t decompressData(const byte_t* src, size_t src_len, byte_t* dst, size_t dst_capacity);
	void displayHeader();
	void displayKernelCap(const pie::hac::KernelCapabilityControl& kern);

	std::string formatMappingAsString(const pie::hac::MemoryMappingHandler::sMemoryMapping& map) const;
};

}