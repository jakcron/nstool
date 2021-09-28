#pragma once
#include "types.h"
#include "KeyBag.h"
#include "PfsProcess.h"

#include <nn/hac/GameCardHeader.h>

namespace nstool {

class GameCardProcess
{
public:
	GameCardProcess();

	void process();

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// xci specific
	void setPartitionForExtract(const std::string& partition_name, const std::string& extract_path);
	void setListFs(bool list_fs);

private:
	const std::string kModuleName = "GameCardProcess";
	const std::string kXciMountPointName = "gamecard:/";

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	bool mListFs;

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

	
	
	bool mIsTrueSdkXci;
	bool mIsSdkXciEncrypted;
	size_t mGcHeaderOffset;
	bool mProccessExtendedHeader;
	byte_t mHdrSignature[fnd::rsa::kRsa2048Size];
	fnd::sha::sSha256Hash mHdrHash;
	nn::hac::GameCardHeader mHdr;
	
	PfsProcess mRootPfs;
	std::vector<sExtractInfo> mExtractInfo;

	void importHeader();
	void displayHeader();
	bool validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash, bool use_salt, byte_t salt);
	bool validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash);
	void validateXciSignature();
	void processRootPfs();
	void processPartitionPfs();
};

}