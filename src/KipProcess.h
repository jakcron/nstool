#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/KernelInitialProcessHeader.h>

#include "common.h"

class KipProcess
{
public:
	KipProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);
private:
	const std::string kModuleName = "KipProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::KernelInitialProcessHeader mHdr;
	fnd::Vec<byte_t> mTextBlob, mRoBlob, mDataBlob;

	void importHeader();
	void importCodeSegments();
	void displayHeader();
	void displayKernelCap(const nn::hac::KernelCapabilityControl& kern);
};