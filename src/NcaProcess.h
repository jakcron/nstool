#pragma once
#include "types.h"
#include "KeyBag.h"

#include <nn/hac/ContentArchiveHeader.h>

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

	// nca specfic
	void setPartition0ExtractPath(const tc::io::Path& path);
	void setPartition1ExtractPath(const tc::io::Path& path);
	void setPartition2ExtractPath(const tc::io::Path& path);
	void setPartition3ExtractPath(const tc::io::Path& path);
	void setListFs(bool list_fs);

private:
	const std::string kModuleName = "NcaProcess";
	const std::string kNpdmExefsPath = "main.npdm";

	// user options
	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	std::array<tc::Optional<tc::io::Path>, nn::hac::nca::kPartitionNum> mPartitionPath;

	bool mListFs;

	// data
	nn::hac::sContentArchiveHeaderBlock mHdrBlock;
	fnd::sha::sSha256Hash mHdrHash;
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
	
	struct sPartitionInfo
	{
		std::shared_ptr<tc::io::IStream> reader;
		std::string fail_reason;
		int64_t offset;
		int64_t size;

		// meta data
		nn::hac::nca::FormatType format_type;
		nn::hac::nca::HashType hash_type;
		nn::hac::nca::EncryptionType enc_type;
		//fnd::LayeredIntegrityMetadata layered_intergrity_metadata;
		nn::hac::detail::aes_iv_t aes_ctr;
	} 
	
	std::array<sPartitionInfo, nn::hac::nca::kPartitionNum> mPartitions;

	void importHeader();
	void generateNcaBodyEncryptionKeys();
	void generatePartitionConfiguration();
	void validateNcaSignatures();
	void displayHeader();
	void processPartitions();

	const char* getContentTypeForMountStr(nn::hac::nca::ContentType cont_type) const;
};

}