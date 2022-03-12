#pragma once
#include "types.h"
#include "KeyBag.h"
#include "FsProcess.h"

#include <nn/hac/ContentArchiveHeader.h>
#include <nn/hac/HierarchicalIntegrityHeader.h>
#include <nn/hac/HierarchicalSha256Header.h>

namespace nstool {

class NcaProcess
{
public:
	NcaProcess();

	void process();

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// fs specific
	void setArchiveJobs(const std::vector<nstool::ArchiveJob>& jobs);
	void setShowFsTree(bool show_fs_tree);
	void setFsRootLabel(const std::string& root_label);
	void setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs);

	// post process() get FS out
	const std::shared_ptr<tc::io::IFileSystem>& getFileSystem() const;
private:
	const std::string kNpdmExefsPath = "/main.npdm";

	std::string mModuleName;

	// user options
	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	// fs processing
	std::shared_ptr<tc::io::IFileSystem> mFileSystem;
	FsProcess mFsProcess;

	// nca data
	nn::hac::sContentArchiveHeaderBlock mHdrBlock;
	nn::hac::detail::sha256_hash_t mHdrHash;
	nn::hac::ContentArchiveHeader mHdr;

	// crypto
	struct sKeys
	{
		struct sKeyAreaKey
		{
			byte_t index;
			bool decrypted;
			KeyBag::aes128_key_t enc;
			KeyBag::aes128_key_t dec;

			void operator=(const sKeyAreaKey& other)
			{
				index = other.index;
				decrypted = other.decrypted;
				enc = other.enc;
				dec = other.dec;
			}

			bool operator==(const sKeyAreaKey& other) const
			{
				return (index == other.index) \
					&& (decrypted == other.decrypted) \
					&& (enc == other.enc) \
					&& (dec == other.dec);
			}

			bool operator!=(const sKeyAreaKey& other) const
			{
				return !(*this == other);
			}
		};
		std::vector<sKeyAreaKey> kak_list;

		tc::Optional<nn::hac::detail::aes128_key_t> aes_ctr;
	} mContentKey;

	struct SparseInfo
	{

	};

	// raw partition data
	struct sPartitionInfo
	{
		std::shared_ptr<tc::io::IStream> reader;
		tc::io::VirtualFileSystem::FileSystemSnapshot fs_snapshot;
		std::shared_ptr<tc::io::IFileSystem> fs_reader;
		std::string fail_reason;
		int64_t offset;
		int64_t size;

		// meta data
		nn::hac::nca::FormatType format_type;
		nn::hac::nca::HashType hash_type;
		nn::hac::nca::EncryptionType enc_type;

		// hash meta data
		nn::hac::HierarchicalIntegrityHeader hierarchicalintegrity_hdr;
		nn::hac::HierarchicalSha256Header hierarchicalsha256_hdr;

		// crypto metadata
		nn::hac::detail::aes_iv_t aes_ctr;

		// sparse metadata
		SparseInfo sparse_info;
	};
	
	std::array<sPartitionInfo, nn::hac::nca::kPartitionNum> mPartitions;

	void importHeader();
	void generateNcaBodyEncryptionKeys();
	void generatePartitionConfiguration();
	void validateNcaSignatures();
	void displayHeader();
	void processPartitions();

	std::string getContentTypeForMountStr(nn::hac::nca::ContentType cont_type) const;
};

}