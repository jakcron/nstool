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

	// fs specific
	void setArchiveJobs(const std::vector<nstool::ArchiveJob>& jobs);
	void setShowFsTree(bool show_fs_tree);
	void setExtractJobs(const std::vector<nstool::ExtractJob> extract_jobs);
private:
	const std::string kXciMountPointName = "gamecard";

	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	bool mIsTrueSdkXci;
	bool mIsSdkXciEncrypted;
	size_t mGcHeaderOffset;
	bool mProccessExtendedHeader;
	nn::hac::detail::rsa2048_signature_t mHdrSignature;
	nn::hac::detail::sha256_hash_t mHdrHash;
	nn::hac::GameCardHeader mHdr;
	
	// fs processing
	std::shared_ptr<tc::io::IStorage> mFileSystem;
	FsProcess mFsProcess;

	void importHeader();
	void displayHeader();
	bool validateRegionOfFile(int64_t offset, int64_t len, const byte_t* test_hash, bool use_salt, byte_t salt);
	bool validateRegionOfFile(int64_t offset, int64_t len, const byte_t* test_hash);
	void validateXciSignature();
	void processRootPfs();
};

}