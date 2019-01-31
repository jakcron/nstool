#include <iostream>
#include <iomanip>
#include <sstream>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/AesCtrWrappedIFile.h>
#include <fnd/LayeredIntegrityWrappedIFile.h>
#include <nn/hac/NcaUtils.h>
#include <nn/hac/AesKeygen.h>
#include <nn/hac/HierarchicalSha256Header.h>
#include <nn/hac/HierarchicalIntegrityHeader.h>
#include "NcaProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "NpdmProcess.h"


NcaProcess::NcaProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false),
	mListFs(false)
{
	for (size_t i = 0; i < nn::hac::nca::kPartitionNum; i++)
	{
		mPartitionPath[i].doExtract = false;
	}
}

void NcaProcess::process()
{
	// import header
	importHeader();

	// determine keys
	generateNcaBodyEncryptionKeys();

	// import/generate fs header data
	generatePartitionConfiguration();

	// validate signatures
	if (mVerify)
		validateNcaSignatures();

	// display header
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayHeader();

	// process partition
	processPartitions();
}

void NcaProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void NcaProcess::setKeyCfg(const KeyConfiguration& keycfg)
{
	mKeyCfg = keycfg;
}

void NcaProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
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

void NcaProcess::importHeader()
{
	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// read header block
	(*mFile)->read((byte_t*)&mHdrBlock, 0, sizeof(nn::hac::sNcaHeaderBlock));
	
	// decrypt header block
	fnd::aes::sAesXts128Key header_key;
	mKeyCfg.getNcaHeaderKey(header_key);
	nn::hac::NcaUtils::decryptNcaHeader((byte_t*)&mHdrBlock, (byte_t*)&mHdrBlock, header_key);

	// generate header hash
	fnd::sha::Sha256((byte_t*)&mHdrBlock.header, sizeof(nn::hac::sNcaHeader), mHdrHash.bytes);

	// proccess main header
	mHdr.fromBytes((byte_t*)&mHdrBlock.header, sizeof(nn::hac::sNcaHeader));
}

void NcaProcess::generateNcaBodyEncryptionKeys()
{
	// create zeros key
	fnd::aes::sAes128Key zero_aesctr_key;
	memset(zero_aesctr_key.key, 0, sizeof(zero_aesctr_key));
	
	// get key data from header
	byte_t masterkey_rev = nn::hac::NcaUtils::getMasterKeyRevisionFromKeyGeneration(mHdr.getKeyGeneration());
	byte_t keak_index = mHdr.getKaekIndex();

	// process key area
	sKeys::sKeyAreaKey kak;
	fnd::aes::sAes128Key key_area_enc_key;
	for (size_t i = 0; i < nn::hac::nca::kAesKeyNum; i++)
	{
		if (mHdr.getEncAesKeys()[i] != zero_aesctr_key)
		{
			kak.index = (byte_t)i;
			kak.enc = mHdr.getEncAesKeys()[i];
			// key[0-3]
			if (i < 4 && mKeyCfg.getNcaKeyAreaEncryptionKey(masterkey_rev, keak_index, key_area_enc_key) == true)
			{
				kak.decrypted = true;
				nn::hac::AesKeygen::generateKey(kak.dec.key, kak.enc.key, key_area_enc_key.key);
			}
			// key[4]
			else if (i == 4 && mKeyCfg.getNcaKeyAreaEncryptionKeyHw(masterkey_rev, keak_index, key_area_enc_key) == true)
			{
				kak.decrypted = true;
				nn::hac::AesKeygen::generateKey(kak.dec.key, kak.enc.key, key_area_enc_key.key);
			}
			else
			{
				kak.decrypted = false;
			}
			mContentKey.kak_list.addElement(kak);
		}
	}

	// set flag to indicate that the keys are not available
	mContentKey.aes_ctr.isSet = false;

	// if this has a rights id, the key needs to be sourced from a ticket
	if (mHdr.hasRightsId() == true)
	{
		fnd::aes::sAes128Key tmp_key;
		if (mKeyCfg.getNcaExternalContentKey(mHdr.getRightsId(), tmp_key) == true)
		{
			mContentKey.aes_ctr = tmp_key;
		}
		else if (mKeyCfg.getNcaExternalContentKey(kDummyRightsIdForUserTitleKey, tmp_key) == true)
		{
			fnd::aes::sAes128Key common_key;
			if (mKeyCfg.getETicketCommonKey(masterkey_rev, common_key) == true)
			{
				nn::hac::AesKeygen::generateKey(tmp_key.key, tmp_key.key, common_key.key);
			}
			mContentKey.aes_ctr = tmp_key;
		}
	}
	// otherwise decrypt key area
	else
	{
		fnd::aes::sAes128Key kak_aes_ctr = zero_aesctr_key;
		for (size_t i = 0; i < mContentKey.kak_list.size(); i++)
		{
			if (mContentKey.kak_list[i].index == nn::hac::nca::KEY_AESCTR && mContentKey.kak_list[i].decrypted)
			{
				kak_aes_ctr = mContentKey.kak_list[i].dec;
			}
		}

		if (kak_aes_ctr != zero_aesctr_key)
		{
			mContentKey.aes_ctr = kak_aes_ctr;
		}
	}

	// if the keys weren't generated, check if the keys were supplied by the user
	if (mContentKey.aes_ctr.isSet == false)
	{
		if (mKeyCfg.getNcaExternalContentKey(kDummyRightsIdForUserBodyKey, mContentKey.aes_ctr.var) == true)
			mContentKey.aes_ctr.isSet = true;
	}
	
	
	if (_HAS_BIT(mCliOutputMode, OUTPUT_KEY_DATA))
	{
		if (mContentKey.aes_ctr.isSet)
		{
			std::cout << "[NCA Content Key]" << std::endl;
			std::cout << "  AES-CTR Key: " << fnd::SimpleTextOutput::arrayToString(mContentKey.aes_ctr.var.key, sizeof(mContentKey.aes_ctr.var), true, ":") << std::endl;
		}
	}
	
	
}

void NcaProcess::generatePartitionConfiguration()
{
	std::stringstream error;

	for (size_t i = 0; i < mHdr.getPartitions().size(); i++)
	{
		// get reference to relevant structures
		const nn::hac::NcaHeader::sPartition& partition = mHdr.getPartitions()[i];
		nn::hac::sNcaFsHeader& fs_header = mHdrBlock.fs_header[partition.index];

		// output structure
		sPartitionInfo& info = mPartitions[partition.index];

		// validate header hash
		fnd::sha::sSha256Hash calc_hash;
		fnd::sha::Sha256((const byte_t*)&mHdrBlock.fs_header[partition.index], sizeof(nn::hac::sNcaFsHeader), calc_hash.bytes);
		if (calc_hash.compare(partition.hash) == false)
		{
			error.clear();
			error <<  "NCA FS Header [" << partition.index << "] Hash: FAIL \n";
			throw fnd::Exception(kModuleName, error.str());
		}
			

		if (fs_header.version.get() != nn::hac::nca::kDefaultFsHeaderVersion)
		{
			error.clear();
			error <<  "NCA FS Header [" << partition.index << "] Version(" << fs_header.version.get() << "): UNSUPPORTED";
			throw fnd::Exception(kModuleName, error.str());
		}

		// setup AES-CTR 
		nn::hac::NcaUtils::getNcaPartitionAesCtr(&fs_header, info.aes_ctr.iv);

		// save partition config
		info.reader = nullptr;
		info.offset = partition.offset;
		info.size = partition.size;
		info.format_type = (nn::hac::nca::FormatType)fs_header.format_type;
		info.hash_type = (nn::hac::nca::HashType)fs_header.hash_type;
		info.enc_type = (nn::hac::nca::EncryptionType)fs_header.encryption_type;
		if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_SHA256)
		{
			// info.hash_tree_meta.importData(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen, LayeredIntegrityMetadata::HASH_TYPE_SHA256);
			nn::hac::HierarchicalSha256Header hdr;
			fnd::List<fnd::LayeredIntegrityMetadata::sLayer> hash_layers;
			fnd::LayeredIntegrityMetadata::sLayer data_layer;
			fnd::List<fnd::sha::sSha256Hash> master_hash_list;

			// import raw data
			hdr.fromBytes(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen);
			for (size_t i = 0; i < hdr.getLayerInfo().size(); i++)
			{
				fnd::LayeredIntegrityMetadata::sLayer layer;
				layer.offset = hdr.getLayerInfo()[i].offset;
				layer.size = hdr.getLayerInfo()[i].size;
				layer.block_size = hdr.getHashBlockSize();
				if (i + 1 == hdr.getLayerInfo().size())
				{
					data_layer = layer;
				}
				else
				{
					hash_layers.addElement(layer);
				}
			}
			master_hash_list.addElement(hdr.getMasterHash());

			// write data into metadata
			info.layered_intergrity_metadata.setAlignHashToBlock(false);
			info.layered_intergrity_metadata.setHashLayerInfo(hash_layers);
			info.layered_intergrity_metadata.setDataLayerInfo(data_layer);
			info.layered_intergrity_metadata.setMasterHashList(master_hash_list);
		}	
		else if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_INTERGRITY)
		{
			// info.hash_tree_meta.importData(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen, LayeredIntegrityMetadata::HASH_TYPE_INTEGRITY);
			nn::hac::HierarchicalIntegrityHeader hdr;
			fnd::List<fnd::LayeredIntegrityMetadata::sLayer> hash_layers;
			fnd::LayeredIntegrityMetadata::sLayer data_layer;
			fnd::List<fnd::sha::sSha256Hash> master_hash_list;

			hdr.fromBytes(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen);
			for (size_t i = 0; i < hdr.getLayerInfo().size(); i++)
			{
				fnd::LayeredIntegrityMetadata::sLayer layer;
				layer.offset = hdr.getLayerInfo()[i].offset;
				layer.size = hdr.getLayerInfo()[i].size;
				layer.block_size = _BIT(hdr.getLayerInfo()[i].block_size);
				if (i + 1 == hdr.getLayerInfo().size())
				{
					data_layer = layer;
				}
				else
				{
					hash_layers.addElement(layer);
				}
			}

			// write data into metadata
			info.layered_intergrity_metadata.setAlignHashToBlock(true);
			info.layered_intergrity_metadata.setHashLayerInfo(hash_layers);
			info.layered_intergrity_metadata.setDataLayerInfo(data_layer);
			info.layered_intergrity_metadata.setMasterHashList(hdr.getMasterHashList());
		}

		// create reader
		try 
		{
			// filter out unrecognised format types
			switch (info.format_type)
			{
				case (nn::hac::nca::FORMAT_PFS0):
				case (nn::hac::nca::FORMAT_ROMFS):
					break;
				default:
					error.clear();
					error <<  "FormatType(" << info.format_type << "): UNKNOWN";
					throw fnd::Exception(kModuleName, error.str());
			}

			// create reader based on encryption type0
			if (info.enc_type == nn::hac::nca::CRYPT_NONE)
			{
				info.reader = new fnd::OffsetAdjustedIFile(mFile, info.offset, info.size);
			}
			else if (info.enc_type == nn::hac::nca::CRYPT_AESCTR)
			{
				if (mContentKey.aes_ctr.isSet == false)
					throw fnd::Exception(kModuleName, "AES-CTR Key was not determined");
				info.reader = new fnd::OffsetAdjustedIFile(new fnd::AesCtrWrappedIFile(mFile, mContentKey.aes_ctr.var, info.aes_ctr), info.offset, info.size);
			}
			else if (info.enc_type == nn::hac::nca::CRYPT_AESXTS || info.enc_type == nn::hac::nca::CRYPT_AESCTREX)
			{
				error.clear();
				error <<  "EncryptionType(" << getEncryptionTypeStr(info.enc_type) << "): UNSUPPORTED";
				throw fnd::Exception(kModuleName, error.str());
			}
			else
			{
				error.clear();
				error <<  "EncryptionType(" << info.enc_type << "): UNKNOWN";
				throw fnd::Exception(kModuleName, error.str());
			}

			// filter out unrecognised hash types, and hash based readers
			if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_SHA256 || info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_INTERGRITY)
			{	
				info.reader = new fnd::LayeredIntegrityWrappedIFile(info.reader, info.layered_intergrity_metadata);
			}
			else if (info.hash_type != nn::hac::nca::HASH_NONE)
			{
				error.clear();
				error <<  "HashType(" << info.hash_type << "): UNKNOWN";
				throw fnd::Exception(kModuleName, error.str());
			}
		}
		catch (const fnd::Exception& e)
		{
			info.fail_reason = std::string(e.error());
		}
	}
}

void NcaProcess::validateNcaSignatures()
{
	// validate signature[0]
	fnd::rsa::sRsa2048Key sign0_key;
	mKeyCfg.getNcaHeader0SignKey(sign0_key);
	if (fnd::rsa::pss::rsaVerify(sign0_key, fnd::sha::HASH_SHA256, mHdrHash.bytes, mHdrBlock.signature_main) != 0)
	{
		std::cout << "[WARNING] NCA Header Main Signature: FAIL" << std::endl;
	}

	// validate signature[1]
	if (mHdr.getContentType() == nn::hac::nca::TYPE_PROGRAM)
	{
		if (mPartitions[nn::hac::nca::PARTITION_CODE].format_type == nn::hac::nca::FORMAT_PFS0)
		{
			if (*mPartitions[nn::hac::nca::PARTITION_CODE].reader != nullptr)
			{
				PfsProcess exefs;
				exefs.setInputFile(mPartitions[nn::hac::nca::PARTITION_CODE].reader);
				exefs.setCliOutputMode(0);
				exefs.process();

				// open main.npdm
				if (exefs.getPfsHeader().getFileList().hasElement(kNpdmExefsPath) == true)
				{
					const nn::hac::PfsHeader::sFile& file = exefs.getPfsHeader().getFileList().getElement(kNpdmExefsPath);

					NpdmProcess npdm;
					npdm.setInputFile(new fnd::OffsetAdjustedIFile(mPartitions[nn::hac::nca::PARTITION_CODE].reader, file.offset, file.size));
					npdm.setCliOutputMode(0);
					npdm.process();

					if (fnd::rsa::pss::rsaVerify(npdm.getNpdmBinary().getAcid().getNcaHeaderSignature2Key(), fnd::sha::HASH_SHA256, mHdrHash.bytes, mHdrBlock.signature_acid) != 0)
					{
						std::cout << "[WARNING] NCA Header ACID Signature: FAIL" << std::endl;
					}
									
				}
				else
				{
					std::cout << "[WARNING] NCA Header ACID Signature: FAIL (\"" << kNpdmExefsPath << "\" not present in ExeFs)" << std::endl;
				}
			}
			else
			{
				std::cout << "[WARNING] NCA Header ACID Signature: FAIL (ExeFs unreadable)" << std::endl;
			}
		}
		else
		{
			std::cout << "[WARNING] NCA Header ACID Signature: FAIL (No ExeFs partition)" << std::endl;
		}
	}
}

void NcaProcess::displayHeader()
{
	std::cout << "[NCA Header]" << std::endl;
	std::cout << "  Format Type:     " << getFormatVersionStr(mHdr.getFormatVersion()) << std::endl;
	std::cout << "  Dist. Type:      " << getDistributionTypeStr(mHdr.getDistributionType()) << std::endl;
	std::cout << "  Content Type:    " << getContentTypeStr(mHdr.getContentType()) << std::endl;
	std::cout << "  Key Generation:  " << std::dec << (uint32_t)mHdr.getKeyGeneration() << std::endl;
	std::cout << "  Kaek Index:      " << getKaekIndexStr((nn::hac::nca::KeyAreaEncryptionKeyIndex)mHdr.getKaekIndex()) << " (" << std::dec << (uint32_t)mHdr.getKaekIndex() << ")" << std::endl;
	std::cout << "  Size:            0x" << std::hex << mHdr.getContentSize() << std::endl;
	std::cout << "  ProgID:          0x" << std::hex << std::setw(16) << std::setfill('0') << mHdr.getProgramId() << std::endl;
	std::cout << "  Content Index:   " << std::dec << mHdr.getContentIndex() << std::endl;
#define _SPLIT_VER(ver) std::dec << (uint32_t)((ver>>24) & 0xff) << "." << (uint32_t)((ver>>16) & 0xff) << "." << (uint32_t)((ver>>8) & 0xff)
	std::cout << "  SdkAddon Ver.:   v" << std::dec << mHdr.getSdkAddonVersion() << " (" << _SPLIT_VER(mHdr.getSdkAddonVersion()) << ")" << std::endl;
#undef _SPLIT_VER
	if (mHdr.hasRightsId())
	{
		std::cout << "  RightsId:        " << fnd::SimpleTextOutput::arrayToString(mHdr.getRightsId(), nn::hac::nca::kRightsIdLen, true, "") << std::endl;
	}
	
	if (mContentKey.kak_list.size() > 0 && _HAS_BIT(mCliOutputMode, OUTPUT_KEY_DATA))
	{
		std::cout << "  Key Area:" << std::endl;
		std::cout << "    <--------------------------------------------------------------------------------------------------------->" << std::endl;
		std::cout << "    | IDX | ENCRYPTED KEY                                   | DECRYPTED KEY                                   |" << std::endl;
		std::cout << "    |-----|-------------------------------------------------|-------------------------------------------------|" << std::endl;
		for (size_t i = 0; i < mContentKey.kak_list.size(); i++)
		{
			std::cout << "    | " << std::dec << std::setw(3) << std::setfill(' ') << (uint32_t)mContentKey.kak_list[i].index << " | ";
			
			std::cout << fnd::SimpleTextOutput::arrayToString(mContentKey.kak_list[i].enc.key, 16, true, ":") << " | ";
			
			
			if (mContentKey.kak_list[i].decrypted)
				std::cout << fnd::SimpleTextOutput::arrayToString(mContentKey.kak_list[i].dec.key, 16, true, ":");
			else
				std::cout << "<unable to decrypt>             ";
			
			std::cout << " |" << std::endl;
		}
		std::cout << "    <--------------------------------------------------------------------------------------------------------->" << std::endl;
	}

	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		std::cout << "  Partitions:" << std::endl;
		for (size_t i = 0; i < mHdr.getPartitions().size(); i++)
		{
			size_t index = mHdr.getPartitions()[i].index;
			sPartitionInfo& info = mPartitions[index];

			std::cout << "    " << std::dec << index << ":" << std::endl;
			std::cout << "      Offset:      0x" << std::hex << (uint64_t)info.offset << std::endl;
			std::cout << "      Size:        0x" << std::hex << (uint64_t)info.size << std::endl;
			std::cout << "      Format Type: " << getFormatTypeStr(info.format_type) << std::endl;
			std::cout << "      Hash Type:   " << getHashTypeStr(info.hash_type) << std::endl;
			std::cout << "      Enc. Type:   " << getEncryptionTypeStr(info.enc_type) << std::endl;
			if (info.enc_type == nn::hac::nca::CRYPT_AESCTR)
			{
				fnd::aes::sAesIvCtr ctr;
				fnd::aes::AesIncrementCounter(info.aes_ctr.iv, info.offset>>4, ctr.iv);
				std::cout << "      AesCtr Counter:" << std::endl;
				std::cout << "        " << fnd::SimpleTextOutput::arrayToString(ctr.iv, sizeof(fnd::aes::sAesIvCtr), true, ":") << std::endl;
			}
			if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_INTERGRITY)
			{
				fnd::LayeredIntegrityMetadata& hash_hdr = info.layered_intergrity_metadata;
				std::cout << "      HierarchicalIntegrity Header:" << std::endl;
				for (size_t j = 0; j < hash_hdr.getHashLayerInfo().size(); j++)
				{
					std::cout << "        Hash Layer " << std::dec << j << ":" << std::endl;
					std::cout << "          Offset:          0x" << std::hex << (uint64_t)hash_hdr.getHashLayerInfo()[j].offset << std::endl;
					std::cout << "          Size:            0x" << std::hex << (uint64_t)hash_hdr.getHashLayerInfo()[j].size << std::endl;
					std::cout << "          BlockSize:       0x" << std::hex << (uint32_t)hash_hdr.getHashLayerInfo()[j].block_size << std::endl;
				}

				std::cout << "        Data Layer:" << std::endl;
				std::cout << "          Offset:          0x" << std::hex << (uint64_t)hash_hdr.getDataLayer().offset << std::endl;
				std::cout << "          Size:            0x" << std::hex << (uint64_t)hash_hdr.getDataLayer().size << std::endl;
				std::cout << "          BlockSize:       0x" << std::hex << (uint32_t)hash_hdr.getDataLayer().block_size << std::endl;
				for (size_t j = 0; j < hash_hdr.getMasterHashList().size(); j++)
				{
					std::cout << "        Master Hash " << std::dec << j << ":" << std::endl;
					std::cout << "          " << fnd::SimpleTextOutput::arrayToString(hash_hdr.getMasterHashList()[j].bytes, 0x10, true, ":") << std::endl;
					std::cout << "          " << fnd::SimpleTextOutput::arrayToString(hash_hdr.getMasterHashList()[j].bytes+0x10, 0x10, true, ":") << std::endl;
				}
			}
			else if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_SHA256)
			{
				fnd::LayeredIntegrityMetadata& hash_hdr = info.layered_intergrity_metadata;
				std::cout << "      HierarchicalSha256 Header:" << std::endl;
				std::cout << "        Master Hash:" << std::endl;
				std::cout << "          " << fnd::SimpleTextOutput::arrayToString(hash_hdr.getMasterHashList()[0].bytes, 0x10, true, ":") << std::endl;
				std::cout << "          " << fnd::SimpleTextOutput::arrayToString(hash_hdr.getMasterHashList()[0].bytes+0x10, 0x10, true, ":") << std::endl;
				std::cout << "        HashBlockSize:     0x" << std::hex << (uint32_t)hash_hdr.getDataLayer().block_size << std::endl;
				std::cout << "        Hash Layer:" << std::endl;
				std::cout << "          Offset:          0x" << std::hex << (uint64_t)hash_hdr.getHashLayerInfo()[0].offset << std::endl;
				std::cout << "          Size:            0x" << std::hex << (uint64_t)hash_hdr.getHashLayerInfo()[0].size << std::endl;
				std::cout << "        Data Layer:" << std::endl;
				std::cout << "          Offset:          0x" << std::hex << (uint64_t)hash_hdr.getDataLayer().offset << std::endl;
				std::cout << "          Size:            0x" << std::hex << (uint64_t)hash_hdr.getDataLayer().size << std::endl;
			}
		}
	}
}


void NcaProcess::processPartitions()
{
	for (size_t i = 0; i < mHdr.getPartitions().size(); i++)
	{
		size_t index = mHdr.getPartitions()[i].index;
		struct sPartitionInfo& partition = mPartitions[index];

		// if the reader is null, skip
		if (*partition.reader == nullptr)
		{
			std::cout << "[WARNING] NCA Partition " << std::dec << index << " not readable.";
			if (partition.fail_reason.empty() == false)
			{
				std::cout << " (" << partition.fail_reason << ")";
			}
			std::cout << std::endl;
			continue;
		}

		if (partition.format_type == nn::hac::nca::FORMAT_PFS0)
		{
			PfsProcess pfs;
			pfs.setInputFile(partition.reader);
			pfs.setCliOutputMode(mCliOutputMode);
			pfs.setListFs(mListFs);
			if (mHdr.getContentType() == nn::hac::nca::TYPE_PROGRAM)
			{
				pfs.setMountPointName(std::string(getContentTypeForMountStr(mHdr.getContentType())) + ":/" + std::string(getProgramPartitionNameStr(index)));
			}
			else
			{
				pfs.setMountPointName(std::string(getContentTypeForMountStr(mHdr.getContentType())) + ":/");
			}
			
			if (mPartitionPath[index].doExtract)
				pfs.setExtractPath(mPartitionPath[index].path);
			pfs.process();
		}
		else if (partition.format_type == nn::hac::nca::FORMAT_ROMFS)
		{
			RomfsProcess romfs;
			romfs.setInputFile(partition.reader);
			romfs.setCliOutputMode(mCliOutputMode);
			romfs.setListFs(mListFs);
			if (mHdr.getContentType() == nn::hac::nca::TYPE_PROGRAM)
			{
				romfs.setMountPointName(std::string(getContentTypeForMountStr(mHdr.getContentType())) + ":/" + std::string(getProgramPartitionNameStr(index)));
			}
			else
			{
				romfs.setMountPointName(std::string(getContentTypeForMountStr(mHdr.getContentType())) + ":/");
			}

			if (mPartitionPath[index].doExtract)
				romfs.setExtractPath(mPartitionPath[index].path);
			romfs.process();
		}
	}
}

const char* NcaProcess::getFormatVersionStr(nn::hac::NcaHeader::FormatVersion format_ver) const
{
	const char* str = nullptr;

	switch (format_ver)
	{
		case (nn::hac::NcaHeader::NCA2_FORMAT):
			str = "NCA2";
			break;
		case (nn::hac::NcaHeader::NCA3_FORMAT):
			str = "NCA3";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}

const char* NcaProcess::getDistributionTypeStr(nn::hac::nca::DistributionType dist_type) const
{
	const char* str = nullptr;

	switch (dist_type)
	{
		case (nn::hac::nca::DIST_DOWNLOAD):
			str = "Download";
			break;
		case (nn::hac::nca::DIST_GAME_CARD):
			str = "Game Card";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}


const char* NcaProcess::getContentTypeStr(nn::hac::nca::ContentType cont_type) const
{
	const char* str = nullptr;

	switch (cont_type)
	{
		case (nn::hac::nca::TYPE_PROGRAM):
			str = "Program";
			break;
		case (nn::hac::nca::TYPE_META):
			str = "Meta";
			break;
		case (nn::hac::nca::TYPE_CONTROL):
			str = "Control";
			break;
		case (nn::hac::nca::TYPE_MANUAL):
			str = "Manual";
			break;
		case (nn::hac::nca::TYPE_DATA):
			str = "Data";
			break;
		case (nn::hac::nca::TYPE_PUBLIC_DATA):
			str = "PublicData";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}

const char* NcaProcess::getEncryptionTypeStr(nn::hac::nca::EncryptionType enc_type) const
{
	const char* str = nullptr;

	switch (enc_type)
	{
		case (nn::hac::nca::CRYPT_AUTO):
			str = "Auto";
			break;
		case (nn::hac::nca::CRYPT_NONE):
			str = "None";
			break;
		case (nn::hac::nca::CRYPT_AESXTS):
			str = "AesXts";
			break;
		case (nn::hac::nca::CRYPT_AESCTR):
			str = "AesCtr";
			break;
		case (nn::hac::nca::CRYPT_AESCTREX):
			str = "AesCtrEx";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}

const char* NcaProcess::getHashTypeStr(nn::hac::nca::HashType hash_type) const
{
	const char* str = nullptr;

	switch (hash_type)
	{
		case (nn::hac::nca::HASH_AUTO):
			str = "Auto";
			break;
		case (nn::hac::nca::HASH_NONE):
			str = "None";
			break;
		case (nn::hac::nca::HASH_HIERARCHICAL_SHA256):
			str = "HierarchicalSha256";
			break;
		case (nn::hac::nca::HASH_HIERARCHICAL_INTERGRITY):
			str = "HierarchicalIntegrity";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}

const char* NcaProcess::getFormatTypeStr(nn::hac::nca::FormatType format_type) const
{
	const char* str = nullptr;

	switch (format_type)
	{
		case (nn::hac::nca::FORMAT_ROMFS):
			str = "RomFs";
			break;
		case (nn::hac::nca::FORMAT_PFS0):
			str = "PartitionFs";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}

const char* NcaProcess::getKaekIndexStr(nn::hac::nca::KeyAreaEncryptionKeyIndex keak_index) const
{
	const char* str = nullptr;

	switch (keak_index)
	{
		case (nn::hac::nca::KAEK_IDX_APPLICATION):
			str = "Application";
			break;
		case (nn::hac::nca::KAEK_IDX_OCEAN):
			str = "Ocean";
			break;
		case (nn::hac::nca::KAEK_IDX_SYSTEM):
			str = "System";
			break;
		default:
			str = "Unknown";
			break;
	}

	return str;
}

const char* NcaProcess::getContentTypeForMountStr(nn::hac::nca::ContentType cont_type) const
{
	const char* str = nullptr;

	switch (cont_type)
	{
		case (nn::hac::nca::TYPE_PROGRAM):
			str = "program";
			break;
		case (nn::hac::nca::TYPE_META):
			str = "meta";
			break;
		case (nn::hac::nca::TYPE_CONTROL):
			str = "control";
			break;
		case (nn::hac::nca::TYPE_MANUAL):
			str = "manual";
			break;
		case (nn::hac::nca::TYPE_DATA):
			str = "data";
			break;
		case (nn::hac::nca::TYPE_PUBLIC_DATA):
			str = "publicData";
			break;
		default:
			str = "";
			break;
	}

	return str;
}

const char* NcaProcess::getProgramPartitionNameStr(size_t i) const
{
	const char* str = nullptr;

	switch (i)
	{
		case (nn::hac::nca::PARTITION_CODE):
			str = "code";
			break;
		case (nn::hac::nca::PARTITION_DATA):
			str = "data";
			break;
		case (nn::hac::nca::PARTITION_LOGO):
			str = "logo";
			break;
		default:
			str = "";
			break;
	}

	return str;
}