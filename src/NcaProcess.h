#pragma once
#include "types.h"
#include "KeyBag.h"
#include "FsProcess.h"

#include <pietendo/hac/ContentArchiveHeader.h>
#include <pietendo/hac/HierarchicalIntegrityHeader.h>
#include <pietendo/hac/HierarchicalSha256Header.h>

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
	void setBaseNcaPath(const tc::Optional<tc::io::Path>& nca_path);


	// fs specific
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
	tc::Optional<tc::io::Path> mBaseNcaPath;

	// fs processing
	std::shared_ptr<tc::io::IFileSystem> mFileSystem;
	FsProcess mFsProcess;

	// nca data
	pie::hac::sContentArchiveHeaderBlock mHdrBlock;
	pie::hac::detail::sha256_hash_t mHdrHash;
	pie::hac::ContentArchiveHeader mHdr;

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

		tc::Optional<pie::hac::detail::aes128_key_t> aes_ctr;
	} mContentKey;

	struct SparseInfo
	{

	};

	// raw partition data
	struct sPartitionInfo
	{
		std::shared_ptr<tc::io::IStream> raw_reader; // raw unprocessed partition stream
		std::shared_ptr<tc::io::IStream> decrypt_reader; // partition stream with transparent decryption
		std::shared_ptr<tc::io::IStream> reader; // partition stream with transparent decryption & hash layer processing
		tc::io::VirtualFileSystem::FileSystemSnapshot fs_snapshot;
		std::shared_ptr<tc::io::IFileSystem> fs_reader;
		std::string fail_reason;
		int64_t offset;
		int64_t size;

		// meta data
		pie::hac::nca::FormatType format_type;
		pie::hac::nca::HashType hash_type;
		pie::hac::nca::EncryptionType enc_type;
		pie::hac::nca::MetaDataHashType metadata_hash_type;

		// hash meta data
		pie::hac::HierarchicalIntegrityHeader hierarchicalintegrity_hdr;
		pie::hac::HierarchicalSha256Header hierarchicalsha256_hdr;

		// crypto metadata
		pie::hac::detail::aes_iv_t aes_ctr;

		// sparse metadata
		SparseInfo sparse_info;
	};
	
	std::array<sPartitionInfo, pie::hac::nca::kPartitionNum> mPartitions;

	void importHeader();
	void generateNcaBodyEncryptionKeys();
	void generatePartitionConfiguration();
	void validateNcaSignatures();
	void displayHeader();
	void processPartitions();

	NcaProcess readBaseNCA();

	std::string getContentTypeForMountStr(pie::hac::nca::ContentType cont_type) const;
};

}