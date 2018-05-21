#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/SimpleFile.h>
#include <nx/NcaHeader.h>
#include "HashTreeMeta.h"


#include "nstool.h"

class NcaProcess
{
public:
	NcaProcess();
	~NcaProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile* file, size_t offset, size_t size);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputType type);
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
	fnd::IFile* mReader;
	const sKeyset* mKeyset;
	CliOutputType mCliOutputType;
	bool mVerify;

	struct sExtract
	{
		std::string path;
		bool doExtract;
	} mPartitionPath[nx::nca::kPartitionNum];

	bool mListFs;

	// data
	nx::sNcaHeaderBlock mHdrBlock;
	crypto::sha::sSha256Hash mHdrHash;
	nx::NcaHeader mHdr;

	// crypto
	struct sKeys
	{
		sOptional<crypto::aes::sAes128Key> aes_ctr;
		sOptional<crypto::aes::sAesXts128Key> aes_xts;
	} mBodyKeys;
	
	struct sPartitionInfo
	{
		fnd::IFile* reader;
		size_t offset;
		size_t size;

		// meta data
		nx::nca::FormatType format_type;
		nx::nca::HashType hash_type;
		nx::nca::EncryptionType enc_type;
		HashTreeMeta hash_tree_meta;
		crypto::aes::sAesIvCtr aes_ctr;
	} mPartitions[nx::nca::kPartitionNum];

	void generateNcaBodyEncryptionKeys();
	void generatePartitionConfiguration();
	void validateNcaSignatures();
	void displayHeader();
	void processPartitions();
};