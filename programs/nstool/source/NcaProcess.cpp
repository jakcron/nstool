#include <fnd/SimpleTextOutput.h>
#include <nx/NcaUtils.h>
#include <nx/AesKeygen.h>
#include "NcaProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "OffsetAdjustedIFile.h"
#include "AesCtrWrappedIFile.h"

std::string kFormatVersionStr[]
{
	"NCA2",
	"NCA3"
};

std::string kDistributionTypeStr[]
{
	"Download",
	"Game Card"
};

std::string kContentTypeStr[]
{
	"Program",
	"Meta",
	"Control",
	"Manual",
	"Data",
	"PublicData"
};

std::string kEncryptionTypeStr[]
{
	"Auto",
	"None",
	"AesXts",
	"AesCtr",
	"AesCtrEx"
};

std::string kHashTypeStr[]
{
	"Auto",
	"None",
	"HierarchicalSha256",
	"HierarchicalIntegrity"
};

std::string kFormatTypeStr[]
{
	"RomFs",
	"PartitionFs"
};

std::string kKaekIndexStr[]
{
	"Application",
	"Ocean",
	"System"
};

void NcaProcess::displayHeader()
{
	crypto::aes::sAes128Key zero_key;
	memset(zero_key.key, 0, sizeof(zero_key));

	printf("[NCA Header]\n");
	printf("  Format Type:     %s\n", kFormatVersionStr[mHdr.getFormatVersion()].c_str());
	printf("  Dist. Type:      %s\n", kDistributionTypeStr[mHdr.getDistributionType()].c_str());
	printf("  Content Type:    %s\n", kContentTypeStr[mHdr.getContentType()].c_str());
	printf("  Key Generation:  %d\n", mHdr.getKeyGeneration());
	printf("  Kaek Index:      %s (%d)\n", kKaekIndexStr[mHdr.getKaekIndex()].c_str(), mHdr.getKaekIndex());
	printf("  Size:            0x%" PRIx64 "\n", mHdr.getContentSize());
	printf("  ProgID:          0x%016" PRIx64 "\n", mHdr.getProgramId());
	printf("  Content Index:   %" PRIu32 "\n", mHdr.getContentIndex());
	uint32_t ver = mHdr.getSdkAddonVersion();
	printf("  SdkAddon Ver.:   v%d.%d.%d (v%" PRIu32 ")\n", (ver>>24 & 0xff),(ver>>16 & 0xff),(ver>>8 & 0xff), ver);
	printf("  RightsId:        ");
	fnd::SimpleTextOutput::hexDump(mHdr.getRightsId(), nx::nca::kRightsIdLen);
	printf("  Key Area Keys: (Encrypted)\n");
	for (size_t i = 0; i < mHdr.getEncAesKeys().getSize(); i++)
	{
		if (mHdr.getEncAesKeys()[i] != zero_key)
		{
			printf("    %2lu: ", i);
			fnd::SimpleTextOutput::hexDump(mHdr.getEncAesKeys()[i].key, crypto::aes::kAes128KeySize);
		}
	}

	/*
	if (mBodyKeyList.getSize() > 0)
	{
		printf("  Key Area Keys:\n");
		for (size_t i = 0; i < mBodyKeyList.getSize(); i++)
		{
			printf("    %2lu: ", i);
			fnd::SimpleTextOutput::hexDump(mBodyKeyList[i].key, crypto::aes::kAes128KeySize);
		}
	}
	*/

	printf("  Partitions:\n");
	for (size_t i = 0; i < mHdr.getPartitions().getSize(); i++)
	{
		const nx::NcaHeader::sPartition& partition = mHdr.getPartitions()[i];
		nx::sNcaFsHeader& fs_header = mHdrBlock.fs_header[partition.index];

		printf("    %lu:\n", i);
		printf("      Index:       %d\n", partition.index);
		printf("      Offset:      0x%" PRIx64 "\n", partition.offset);
		printf("      Size:        0x%" PRIx64 "\n", partition.size);
		
		
		crypto::sha::sSha256Hash ncaFsHeaderHash;
		crypto::sha::Sha256((byte_t*)&fs_header, sizeof(nx::sNcaFsHeader), ncaFsHeaderHash.bytes);
		if (partition.hash.compare(ncaFsHeaderHash) == false)
		{
			throw fnd::Exception(kModuleName, "NcaFsHeader has bad sha256 hash");
		}

		//fnd::SimpleTextOutput::hxdStyleDump((byte_t*)&fs_header, sizeof(nx::sNcaFsHeader));


		printf("      FsHeader:\n");
		printf("        Version:     0x%d\n", fs_header.version.get());
		printf("        Format Type: %s\n", kFormatTypeStr[fs_header.format_type].c_str());
		printf("        Hash Type:   %s\n", kHashTypeStr[fs_header.hash_type].c_str());
		printf("        Enc. Type:   %s\n", kEncryptionTypeStr[fs_header.encryption_type].c_str());
		if (fs_header.encryption_type == nx::nca::CRYPT_AESCTR)
		{
			printf("        CTR:         ");
			crypto::aes::sAesIvCtr ctr;
			nx::NcaUtils::getNcaPartitionAesCtr(&fs_header, ctr.iv);
			crypto::aes::AesIncrementCounter(ctr.iv, partition.offset>>4, ctr.iv);
			fnd::SimpleTextOutput::hexDump(ctr.iv, sizeof(crypto::aes::sAesIvCtr));
		}
		if (fs_header.hash_type == nx::nca::HASH_HIERARCHICAL_INTERGRITY)
		{
			printf("      HierarchicalIntegrity Header:\n");
			printf("        Id:                0x%x\n", fs_header.ivfc_header.id.get());
			printf("        MasterHashSize:    0x%x\n", fs_header.ivfc_header.master_hash_size.get());
			printf("        LevelNum:          %d\n", fs_header.ivfc_header.level_num.get());
			for (size_t i = 0; i < fs_header.ivfc_header.level_num.get(); i++)
			{
				printf("        Level %d:\n", i);
				printf("          LogicalOffset:   0x%" PRIx64 "\n", fs_header.ivfc_header.level_header[i].logical_offset.get());
				printf("          HashDataSize:    0x%" PRIx64 "\n", fs_header.ivfc_header.level_header[i].hash_data_size.get());
				printf("          BlockSize:       0x%" PRIx32 "\n", fs_header.ivfc_header.level_header[i].block_size.get());
			}
			printf("        Master Hash:       ");
			fnd::SimpleTextOutput::hexDump(fs_header.ivfc_header.master_hash.bytes, 0x20);

			
		}
		else if (fs_header.hash_type == nx::nca::HASH_HIERARCHICAL_SHA256)
		{
			nx::sHierarchicalSha256Header& hash_hdr = fs_header.hierarchicalsha256_header;
			printf("      HierarchicalSha256 Header:\n");
			printf("        Master Hash:       ");
			fnd::SimpleTextOutput::hexDump(hash_hdr.master_hash.bytes, 0x20);
			printf("        HashBlockSize:     0x%x\n", hash_hdr.hash_block_size.get());
			printf("        HashLevelNum:      0x%x\n", hash_hdr.hash_level_num.get());
			printf("        HashDataOffset:    0x%" PRIx64 "\n", hash_hdr.hash_data.offset.get());
			printf("        HashDataSize:      0x%" PRIx64 "\n", hash_hdr.hash_data.size.get());
			printf("        HashTargetOffset:  0x%" PRIx64 "\n", hash_hdr.hash_target.offset.get());
			printf("        HashTargetSize:    0x%" PRIx64 "\n", hash_hdr.hash_target.size.get());
		
		}
		else
		{
			printf("      Hash Superblock:\n");
			fnd::SimpleTextOutput::hxdStyleDump(fs_header.hash_superblock, nx::nca::kFsHeaderHashSuperblockLen);
		}
	}
}

void NcaProcess::generateNcaBodyEncryptionKeys()
{
	// create zeros key
	crypto::aes::sAes128Key zero_aesctr_key;
	memset(zero_aesctr_key.key, 0, sizeof(zero_aesctr_key));
	crypto::aes::sAesXts128Key zero_aesxts_key;
	memset(zero_aesxts_key.key, 0, sizeof(zero_aesxts_key));
	
	// get key data from header
	byte_t masterkey_rev = nx::NcaUtils::getMasterKeyRevisionFromKeyGeneration(mHdr.getKeyGeneration());
	byte_t keak_index = mHdr.getKaekIndex();

	// set flag to indicate that the keys are not available
	mBodyKeys.aes_ctr.isSet = false;
	mBodyKeys.aes_xts.isSet = false;

	// if this has a rights id, the key needs to be sourced from a ticket
	if (mHdr.hasRightsId() == true)
	{
		// if the titlekey_kek is available
		if (mKeyset->ticket.titlekey_kek[masterkey_rev] != zero_aesctr_key)
		{
			// the title key is provided (sourced from ticket)
			if (mKeyset->nca.manual_title_key_aesctr != zero_aesctr_key)
			{
				nx::AesKeygen::generateKey(mBodyKeys.aes_ctr.var.key, mKeyset->nca.manual_title_key_aesctr.key, mKeyset->ticket.titlekey_kek[masterkey_rev].key);
				mBodyKeys.aes_ctr.isSet = true;
			}
			if (mKeyset->nca.manual_title_key_aesxts != zero_aesxts_key)
			{
				nx::AesKeygen::generateKey(mBodyKeys.aes_xts.var.key[0], mKeyset->nca.manual_title_key_aesxts.key[0], mKeyset->ticket.titlekey_kek[masterkey_rev].key);
				nx::AesKeygen::generateKey(mBodyKeys.aes_xts.var.key[1], mKeyset->nca.manual_title_key_aesxts.key[1], mKeyset->ticket.titlekey_kek[masterkey_rev].key);
				mBodyKeys.aes_xts.isSet = true;
			}
		}
	}
	// otherwise decrypt key area
	else
	{
		// if the titlekey_kek is available
		if (mKeyset->nca.key_area_key[keak_index][masterkey_rev] != zero_aesctr_key)
		{
			nx::AesKeygen::generateKey(mBodyKeys.aes_ctr.var.key, mHdr.getEncAesKeys()[nx::nca::KEY_AESCTR].key, mKeyset->nca.key_area_key[keak_index][masterkey_rev].key);
			mBodyKeys.aes_ctr.isSet = true;
			
			nx::AesKeygen::generateKey(mBodyKeys.aes_xts.var.key[0], mHdr.getEncAesKeys()[nx::nca::KEY_AESXTS_0].key, mKeyset->nca.key_area_key[keak_index][masterkey_rev].key);
			nx::AesKeygen::generateKey(mBodyKeys.aes_xts.var.key[1], mHdr.getEncAesKeys()[nx::nca::KEY_AESXTS_1].key, mKeyset->nca.key_area_key[keak_index][masterkey_rev].key);
			mBodyKeys.aes_xts.isSet = true;
		}
	}

	// if the keys weren't generated, check if the keys were supplied by the user
	if (mBodyKeys.aes_ctr.isSet == false && mKeyset->nca.manual_body_key_aesctr != zero_aesctr_key)
	{
		mBodyKeys.aes_ctr = mKeyset->nca.manual_body_key_aesctr;
	}
	if (mBodyKeys.aes_xts.isSet == false && mKeyset->nca.manual_body_key_aesxts != zero_aesxts_key)
	{
		mBodyKeys.aes_xts = mKeyset->nca.manual_body_key_aesxts;
	}
}

void NcaProcess::processPartitions()
{
	for (size_t i = 0; i < mHdr.getPartitions().getSize(); i++)
	{
		const nx::NcaHeader::sPartition& partition = mHdr.getPartitions()[i];
		nx::sNcaFsHeader& fs_header = mHdrBlock.fs_header[partition.index];

		crypto::aes::sAesIvCtr ctr;
		nx::NcaUtils::getNcaPartitionAesCtr(&fs_header, ctr.iv);

		// create reader
		fnd::IFile* partitionReader = nullptr;

		AesCtrWrappedIFile aesCtrFile = AesCtrWrappedIFile(mReader, mBodyKeys.aes_ctr.var, ctr);
		switch(fs_header.encryption_type)
		{
			case (nx::nca::CRYPT_AESXTS):
			case (nx::nca::CRYPT_AESCTREX):
				partitionReader = nullptr;
				break;
			case (nx::nca::CRYPT_AESCTR):
				partitionReader = mBodyKeys.aes_ctr.isSet? &aesCtrFile : nullptr;
				break;
			case (nx::nca::CRYPT_NONE):
				partitionReader = mReader;
				break;
		}

		// if the reader is null, skip
		if (partitionReader == nullptr)
		{
			printf("[WARNING] NCA Partition %d not readable\n", partition.index);
			continue;
		}

		size_t data_offset = 0;
		switch (fs_header.hash_type)
		{
			case (nx::nca::HASH_HIERARCHICAL_SHA256):
				data_offset = fs_header.hierarchicalsha256_header.hash_target.offset.get();
				break;
			case (nx::nca::HASH_HIERARCHICAL_INTERGRITY):
				data_offset = fs_header.ivfc_header.level_header[5].logical_offset.get();
				break;
			case (nx::nca::HASH_NONE):
				data_offset = 0;
				break;
			default:
				throw fnd::Exception(kModuleName, "Unknown hash type");
		}

		if (fs_header.format_type == nx::nca::FORMAT_PFS0)
		{
			PfsProcess pfs;
			pfs.setInputFile(partitionReader);
			pfs.setInputFileOffset(partition.offset + data_offset);
			pfs.setCliOutputMode(mCliOutputType);
			pfs.setListFs(mListFs);
			if (mPartitionPath[partition.index].doExtract)
				pfs.setExtractPath(mPartitionPath[partition.index].path);
			pfs.process();
		}
		else if (fs_header.format_type == nx::nca::FORMAT_ROMFS)
		{
			RomfsProcess romfs;
			romfs.setInputFile(partitionReader);
			romfs.setInputFileOffset(partition.offset + data_offset);
			romfs.setCliOutputMode(mCliOutputType);
			romfs.setListFs(mListFs);
			if (mPartitionPath[partition.index].doExtract)
				romfs.setExtractPath(mPartitionPath[partition.index].path);
			romfs.process();
		}
		else
		{
			throw fnd::Exception(kModuleName, "Unknown format type");
		}
	}
}

NcaProcess::NcaProcess() :
	mReader(nullptr),
	mOffset(0),
	mKeyset(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false),
	mListFs(false)
{
	mPartitionPath[0].doExtract = false;
	mPartitionPath[1].doExtract = false;
	mPartitionPath[2].doExtract = false;
	mPartitionPath[3].doExtract = false;
}

NcaProcess::~NcaProcess()
{

}

void NcaProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// read header block
	mReader->read((byte_t*)&mHdrBlock, mOffset, sizeof(nx::sNcaHeaderBlock));
	
	// decrypt header block
	nx::NcaUtils::decryptNcaHeader((byte_t*)&mHdrBlock, (byte_t*)&mHdrBlock, mKeyset->nca.header_key);

	// generate header hash
	crypto::sha::Sha256((byte_t*)&mHdrBlock.header, sizeof(nx::sNcaHeader), mHdrHash.bytes);

	// validate signature[0]
	if (mVerify)
	{
		if (crypto::rsa::pss::rsaVerify(mKeyset->nca.header_sign_key, crypto::sha::HASH_SHA256, mHdrHash.bytes, mHdrBlock.signature_main) != 0)
		{
			// this is minimal even though it's a warning because it's a validation method
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] NCA Header Main Signature: FAIL \n");
		}
	}

	// proccess main header
	mHdr.importBinary((byte_t*)&mHdrBlock.header, sizeof(nx::sNcaHeader));

	// validate fs headers
	if (mVerify)
	{
		crypto::sha::sSha256Hash calc_hash;

		for (size_t i = 0; i < mHdr.getPartitions().getSize(); i++)
		{
			const nx::NcaHeader::sPartition& partition = mHdr.getPartitions()[i];

			crypto::sha::Sha256((const byte_t*)&mHdrBlock.fs_header[partition.index], sizeof(nx::sNcaFsHeader), calc_hash.bytes);

			if (calc_hash.compare(partition.hash) == false)
			{
				// this is minimal even though it's a warning because it's a validation method
				if (mCliOutputType >= OUTPUT_MINIMAL)
					printf("[WARNING] NCA FsHeader[%d] Hash: FAIL \n", partition.index);
			}
		}
	}

	// determine keys
	

	/*
	NCA is a file container
	A hashed and signed file container

	To verify a NCA: (R=regular step)
	1 - decrypt header (R)
	2 - verify signature[0]
	3 - validate hashes of fs_headers
	4 - determine how to read/decrypt the partitions (R)
	5 - validate the partitions depending on their hash method
	6 - if this NCA is a Program or Patch, open main.npdm from partition0
	7 - validate ACID
	8 - use public key in ACID to verify NCA signature[1]

	Things to consider
	* because of the manditory steps between verifcation steps
	  the NCA should be ready to be pulled to pieces before any printing is done
	  so the verification text can be presented without interuption

	*/


	// decrypt key area
	generateNcaBodyEncryptionKeys();

	if (mCliOutputType >= OUTPUT_NORMAL)
		displayHeader();

	processPartitions();
}

void NcaProcess::setInputFile(fnd::IFile* reader)
{
	mReader = reader;
}

void NcaProcess::setInputFileOffset(size_t offset)
{
	mOffset = offset;
}

void NcaProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void NcaProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void NcaProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void NcaProcess::setPartition0ExtractPath(const std::string& path)
{
	mPartitionPath[0].path = path;
	mPartitionPath[0].doExtract = true;
}

void NcaProcess::setPartition1ExtractPath(const std::string& path)
{
	mPartitionPath[1].path = path;
	mPartitionPath[1].doExtract = true;
}

void NcaProcess::setPartition2ExtractPath(const std::string& path)
{
	mPartitionPath[2].path = path;
	mPartitionPath[2].doExtract = true;
}

void NcaProcess::setPartition3ExtractPath(const std::string& path)
{
	mPartitionPath[3].path = path;
	mPartitionPath[3].doExtract = true;
}

void NcaProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}
