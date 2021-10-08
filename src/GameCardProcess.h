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

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// fs specific
	void setShowFsTree(bool show_fs_tree);
	void setExtractJobs(const std::vector<nstool::ExtractJob> extract_jobs);

	void process();

private:
	const std::string kXciMountPointName = "gamecard";

	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	bool mListFs;
	
	bool mIsTrueSdkXci;
	bool mIsSdkXciEncrypted;
	size_t mGcHeaderOffset;
	bool mProccessExtendedHeader;
	nn::hac::detail::rsa2048_signature_t mHdrSignature;
	nn::hac::detail::sha256_hash_t mHdrHash;
	nn::hac::GameCardHeader mHdr;
	
	PfsProcess mRootPfs;
	std::vector<nstool::ExtractJob> mExtractJobs;

	void importHeader();
	void displayHeader();
	bool validateRegionOfFile(int64_t offset, int64_t len, const byte_t* test_hash, bool use_salt, byte_t salt);
	bool validateRegionOfFile(int64_t offset, int64_t len, const byte_t* test_hash);
	void validateXciSignature();
	void processRootPfs();
};

}