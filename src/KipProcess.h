#pragma once
#include "types.h"

#include <nn/hac/KernelInitialProcessHeader.h>

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
	const std::string kModuleName = "KipProcess";

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::KernelInitialProcessHeader mHdr;
	tc::ByteData mTextBlob, mRoBlob, mDataBlob;

	void importHeader();
	void importCodeSegments();
	void displayHeader();
	void displayKernelCap(const nn::hac::KernelCapabilityControl& kern);
};

}