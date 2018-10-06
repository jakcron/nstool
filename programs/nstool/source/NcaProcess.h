#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <fnd/LayeredIntegrityMetadata.h>
#include <nn/hac/NcaHeader.h>
#include "KeyConfiguration.h"


#include "common.h"

class NcaProcess
{
public:
	NcaProcess();

	void process();

	// generic
	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setKeyCfg(const KeyConfiguration& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// nca specfic
	void setPartition0ExtractPath(const std::string& path);
	void setPartition1ExtractPath(const std::string& path);
	void setPartition2ExtractPath(const std::string& path);
	void setPartition3ExtractPath(const std::string& path);
	void setListFs(bool list_fs);

private:
	const std::string kModuleName = "NcaProcess";
	const std::string kNpdmExefsPath = "main.npdm";

	// user options
	fnd::SharedPtr<fnd::IFile> mFile;
	KeyConfiguration mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	struct sExtract
	{
		std::string path;
		bool doExtract;
	} mPartitionPath[nn::hac::nca::kPartitionNum];

	bool mListFs;

	// data
	nn::hac::sNcaHeaderBlock mHdrBlock;
	fnd::sha::sSha256Hash mHdrHash;
	nn::hac::NcaHeader mHdr;

	// crypto
	struct sKeys
	{
		struct sKeyAreaKey
		{
			byte_t index;
			bool decrypted;
			fnd::aes::sAes128Key enc;
			fnd::aes::sAes128Key dec;

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
		fnd::List<sKeyAreaKey> kak_list;

		sOptional<fnd::aes::sAes128Key> aes_ctr;
	} mContentKey;
	
	struct sPartitionInfo
	{
		fnd::SharedPtr<fnd::IFile> reader;
		std::string fail_reason;
		size_t offset;
		size_t size;

		// meta data
		nn::hac::nca::FormatType format_type;
		nn::hac::nca::HashType hash_type;
		nn::hac::nca::EncryptionType enc_type;
		fnd::LayeredIntegrityMetadata layered_intergrity_metadata;
		fnd::aes::sAesIvCtr aes_ctr;
	} mPartitions[nn::hac::nca::kPartitionNum];

	void importHeader();
	void generateNcaBodyEncryptionKeys();
	void generatePartitionConfiguration();
	void validateNcaSignatures();
	void displayHeader();
	void processPartitions();

	const char* getFormatVersionStr(nn::hac::NcaHeader::FormatVersion format_ver) const;
	const char* getDistributionTypeStr(nn::hac::nca::DistributionType dist_type) const;
	const char* getContentTypeStr(nn::hac::nca::ContentType cont_type) const;
	const char* getEncryptionTypeStr(nn::hac::nca::EncryptionType enc_type) const;
	const char* getHashTypeStr(nn::hac::nca::HashType hash_type) const;
	const char* getFormatTypeStr(nn::hac::nca::FormatType format_type) const;
	const char* getKaekIndexStr(nn::hac::nca::KeyAreaEncryptionKeyIndex keak_index) const;
	const char* getContentTypeForMountStr(nn::hac::nca::ContentType cont_type) const;
	const char* getProgramPartitionNameStr(size_t i) const;
};