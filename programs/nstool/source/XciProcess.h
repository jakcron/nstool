#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <fnd/List.h>
#include <nn/hac/XciHeader.h>
#include "KeyConfiguration.h"
#include "PfsProcess.h"

#include "common.h"

class XciProcess
{
public:
	XciProcess();

	void process();

	// generic
	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setKeyCfg(const KeyConfiguration& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// xci specific
	void setPartitionForExtract(const std::string& partition_name, const std::string& extract_path);
	void setListFs(bool list_fs);

private:
	const std::string kModuleName = "XciProcess";
	const std::string kXciMountPointName = "gamecard:/";

	fnd::SharedPtr<fnd::IFile> mFile;
	KeyConfiguration mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	struct sExtractInfo
	{
		std::string partition_name;
		std::string extract_path;

		void operator=(const sExtractInfo& other)
		{
			partition_name = other.partition_name;
			extract_path = other.extract_path;
		}

		bool operator==(const std::string& name) const
		{
			return name == partition_name;
		}
	};

	bool mListFs;

	nn::hac::sXciHeaderPage mHdrPage;
	nn::hac::XciHeader mHdr;
	PfsProcess mRootPfs;
	fnd::List<sExtractInfo> mExtractInfo;

	void importHeader();
	void displayHeader();
	bool validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash);
	void validateXciSignature();
	void processRootPfs();
	void processPartitionPfs();

	// strings
	const char* getRomSizeStr(byte_t rom_size) const;
	const char* getHeaderFlagStr(byte_t flag) const;
	const char* getCardClockRate(uint32_t acc_ctrl_1) const;
};