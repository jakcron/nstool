#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/SimpleFile.h>
#include <nx/NcaHeader.h>

#include "nstool.h"

class NcaProcess
{
public:
	NcaProcess();
	~NcaProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile* reader);
	void setInputFileOffset(size_t offset);
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

	// user options
	fnd::IFile* mReader;
	size_t mOffset;
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
	


	void displayHeader();

	void generateNcaBodyEncryptionKeys();

	void processPartitions();
};