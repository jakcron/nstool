#include <iostream>
#include <sstream>
#include <fnd/SimpleTextOutput.h>
#include <nn/hac/NcaUtils.h>
#include <nn/hac/AesKeygen.h>
#include "NcaProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "NpdmProcess.h"
#include "OffsetAdjustedIFile.h"
#include "AesCtrWrappedIFile.h"
#include "HashTreeWrappedIFile.h"

const char* getFormatVersionStr(nn::hac::NcaHeader::FormatVersion format_ver)
{
	const char* str;
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

const char* getDistributionTypeStr(nn::hac::nca::DistributionType dist_type)
{
	const char* str;
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


 const char* getContentTypeStr(nn::hac::nca::ContentType cont_type)
{
	const char* str;
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

const char* getEncryptionTypeStr(nn::hac::nca::EncryptionType enc_type)
{
	const char* str;
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

inline const char* getHashTypeStr(nn::hac::nca::HashType hash_type)
{
	const char* str;
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

inline const char* getFormatTypeStr(nn::hac::nca::FormatType format_type)
{
	const char* str;
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

inline const char* getKaekIndexStr(nn::hac::nca::KeyAreaEncryptionKeyIndex keak_index)
{
	const char* str;
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

inline const char* getContentTypeForMountStr(nn::hac::nca::ContentType cont_type)
{
	const char* str;
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

const char* getProgramPartitionNameStr(size_t i)
{
	const char* str;
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


NcaProcess::NcaProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mKeyset(nullptr),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false),
	mListFs(false)
{
	for (size_t i = 0; i < nn::hac::nca::kPartitionNum; i++)
	{
		mPartitionPath[i].doExtract = false;
		mPartitions[i].reader = nullptr;
	}
}

NcaProcess::~NcaProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}

	for (size_t i = 0; i < nn::hac::nca::kPartitionNum; i++)
	{
		if (mPartitions[i].reader != nullptr)
		{
			delete mPartitions[i].reader;
		}
	}
}

void NcaProcess::process()
{
	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// read header block
	mFile->read((byte_t*)&mHdrBlock, 0, sizeof(nn::hac::sNcaHeaderBlock));
	
	// decrypt header block
	nn::hac::NcaUtils::decryptNcaHeader((byte_t*)&mHdrBlock, (byte_t*)&mHdrBlock, mKeyset->nca.header_key);

	// generate header hash
	fnd::sha::Sha256((byte_t*)&mHdrBlock.header, sizeof(nn::hac::sNcaHeader), mHdrHash.bytes);

	// proccess main header
	mHdr.fromBytes((byte_t*)&mHdrBlock.header, sizeof(nn::hac::sNcaHeader));

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

void NcaProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void NcaProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
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

void NcaProcess::generateNcaBodyEncryptionKeys()
{
	// create zeros key
	fnd::aes::sAes128Key zero_aesctr_key;
	memset(zero_aesctr_key.key, 0, sizeof(zero_aesctr_key));
	fnd::aes::sAesXts128Key zero_aesxts_key;
	memset(zero_aesxts_key.key, 0, sizeof(zero_aesxts_key));
	
	// get key data from header
	byte_t masterkey_rev = nn::hac::NcaUtils::getMasterKeyRevisionFromKeyGeneration(mHdr.getKeyGeneration());
	byte_t keak_index = mHdr.getKaekIndex();

	// process key area
	sKeys::sKeyAreaKey keak;
	for (size_t i = 0; i < nn::hac::nca::kAesKeyNum; i++)
	{
		if (mHdr.getEncAesKeys()[i] != zero_aesctr_key)
		{
			keak.index = (byte_t)i;
			keak.enc = mHdr.getEncAesKeys()[i];
			if (i < 4 && mKeyset->nca.key_area_key[keak_index][masterkey_rev] != zero_aesctr_key)
			{
				keak.decrypted = true;
				nn::hac::AesKeygen::generateKey(keak.dec.key, keak.enc.key, mKeyset->nca.key_area_key[keak_index][masterkey_rev].key);
			}
			else
			{
				keak.decrypted = false;
			}
			mBodyKeys.keak_list.addElement(keak);
		}
	}

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
				nn::hac::AesKeygen::generateKey(mBodyKeys.aes_ctr.var.key, mKeyset->nca.manual_title_key_aesctr.key, mKeyset->ticket.titlekey_kek[masterkey_rev].key);
				mBodyKeys.aes_ctr.isSet = true;
			}
			if (mKeyset->nca.manual_title_key_aesxts != zero_aesxts_key)
			{
				nn::hac::AesKeygen::generateKey(mBodyKeys.aes_xts.var.key[0], mKeyset->nca.manual_title_key_aesxts.key[0], mKeyset->ticket.titlekey_kek[masterkey_rev].key);
				nn::hac::AesKeygen::generateKey(mBodyKeys.aes_xts.var.key[1], mKeyset->nca.manual_title_key_aesxts.key[1], mKeyset->ticket.titlekey_kek[masterkey_rev].key);
				mBodyKeys.aes_xts.isSet = true;
			}
		}
	}
	// otherwise decrypt key area
	else
	{
		fnd::aes::sAes128Key keak_aesctr_key = zero_aesctr_key;
		fnd::aes::sAesXts128Key keak_aesxts_key = zero_aesxts_key;
		for (size_t i = 0; i < mBodyKeys.keak_list.size(); i++)
		{
			if (mBodyKeys.keak_list[i].index == nn::hac::nca::KEY_AESCTR && mBodyKeys.keak_list[i].decrypted)
			{
				keak_aesctr_key = mBodyKeys.keak_list[i].dec;
			}
			else if (mBodyKeys.keak_list[i].index == nn::hac::nca::KEY_AESXTS_0 && mBodyKeys.keak_list[i].decrypted)
			{
				memcpy(keak_aesxts_key.key[0], mBodyKeys.keak_list[i].dec.key, sizeof(fnd::aes::sAes128Key));
			}
			else if (mBodyKeys.keak_list[i].index == nn::hac::nca::KEY_AESXTS_1 && mBodyKeys.keak_list[i].decrypted)
			{
				memcpy(keak_aesxts_key.key[1], mBodyKeys.keak_list[i].dec.key, sizeof(fnd::aes::sAes128Key));
			}
		}

		if (keak_aesctr_key != zero_aesctr_key)
		{
			mBodyKeys.aes_ctr = keak_aesctr_key;
		}
		if (keak_aesxts_key != zero_aesxts_key)
		{
			mBodyKeys.aes_xts = keak_aesxts_key;
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
	
	if (_HAS_BIT(mCliOutputMode, OUTPUT_KEY_DATA))
	{
		if (mBodyKeys.aes_ctr.isSet)
		{
			printf("[NCA Body Key]\n");
			printf("  AES-CTR Key: ");
			fnd::SimpleTextOutput::hexDump(mBodyKeys.aes_ctr.var.key, sizeof(mBodyKeys.aes_ctr.var));
		}
		
		if (mBodyKeys.aes_xts.isSet)
		{
			printf("[NCA Body Key]\n");
			printf("  AES-XTS Key0: ");
			fnd::SimpleTextOutput::hexDump(mBodyKeys.aes_xts.var.key[0], sizeof(mBodyKeys.aes_ctr.var));
			printf("  AES-XTS Key1: ");
			fnd::SimpleTextOutput::hexDump(mBodyKeys.aes_xts.var.key[1], sizeof(mBodyKeys.aes_ctr.var));
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
			info.hash_tree_meta.importData(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen, HashTreeMeta::HASH_TYPE_SHA256);
		else if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_INTERGRITY)
			info.hash_tree_meta.importData(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen, HashTreeMeta::HASH_TYPE_INTEGRITY);

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
				info.reader = new OffsetAdjustedIFile(mFile, SHARED_IFILE, info.offset, info.size);
			}
			else if (info.enc_type == nn::hac::nca::CRYPT_AESCTR)
			{
				if (mBodyKeys.aes_ctr.isSet == false)
					throw fnd::Exception(kModuleName, "AES-CTR Key was not determined");
				info.reader = new OffsetAdjustedIFile(new AesCtrWrappedIFile(mFile, SHARED_IFILE, mBodyKeys.aes_ctr.var, info.aes_ctr), OWN_IFILE, info.offset, info.size);
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
				fnd::IFile* tmp = info.reader;
				info.reader = nullptr;
				info.reader = new HashTreeWrappedIFile(tmp, OWN_IFILE, info.hash_tree_meta);
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
			if (info.reader != nullptr)
				delete info.reader;
			info.reader = nullptr;
		}
	}
}

void NcaProcess::validateNcaSignatures()
{
	// validate signature[0]
	if (fnd::rsa::pss::rsaVerify(mKeyset->nca.header_sign_key, fnd::sha::HASH_SHA256, mHdrHash.bytes, mHdrBlock.signature_main) != 0)
	{
		printf("[WARNING] NCA Header Main Signature: FAIL \n");
	}

	// validate signature[1]
	if (mHdr.getContentType() == nn::hac::nca::TYPE_PROGRAM)
	{
		if (mPartitions[nn::hac::nca::PARTITION_CODE].format_type == nn::hac::nca::FORMAT_PFS0)
		{
			if (mPartitions[nn::hac::nca::PARTITION_CODE].reader != nullptr)
			{
				PfsProcess exefs;
				exefs.setInputFile(mPartitions[nn::hac::nca::PARTITION_CODE].reader, SHARED_IFILE);
				exefs.setCliOutputMode(0);
				exefs.process();

				// open main.npdm
				if (exefs.getPfsHeader().getFileList().hasElement(kNpdmExefsPath) == true)
				{
					const nn::hac::PfsHeader::sFile& file = exefs.getPfsHeader().getFileList().getElement(kNpdmExefsPath);

					NpdmProcess npdm;
					npdm.setInputFile(new OffsetAdjustedIFile(mPartitions[nn::hac::nca::PARTITION_CODE].reader, SHARED_IFILE, file.offset, file.size), OWN_IFILE);
					npdm.setCliOutputMode(0);
					npdm.process();

					if (fnd::rsa::pss::rsaVerify(npdm.getNpdmBinary().getAcid().getNcaHeaderSignature2Key(), fnd::sha::HASH_SHA256, mHdrHash.bytes, mHdrBlock.signature_acid) != 0)
					{
						printf("[WARNING] NCA Header ACID Signature: FAIL \n");
					}
									
				}
				else
				{
					printf("[WARNING] NCA Header ACID Signature: FAIL (\"%s\" not present in ExeFs)\n", kNpdmExefsPath.c_str());
				}
			}
			else
			{
				printf("[WARNING] NCA Header ACID Signature: FAIL (ExeFs unreadable)\n");
			}
		}
		else
		{
			printf("[WARNING] NCA Header ACID Signature: FAIL (No ExeFs partition)\n");
		}
	}
}

void NcaProcess::displayHeader()
{
#define _HEXDUMP_U(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02X", var[a__a__A]); } while(0)
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	printf("[NCA Header]\n");
	printf("  Format Type:     %s\n", getFormatVersionStr(mHdr.getFormatVersion()));
	printf("  Dist. Type:      %s\n", getDistributionTypeStr(mHdr.getDistributionType()));
	printf("  Content Type:    %s\n", getContentTypeStr(mHdr.getContentType()));
	printf("  Key Generation:  %d\n", mHdr.getKeyGeneration());
	printf("  Kaek Index:      %s (%d)\n", getKaekIndexStr((nn::hac::nca::KeyAreaEncryptionKeyIndex)mHdr.getKaekIndex()), mHdr.getKaekIndex());
	printf("  Size:            0x%" PRIx64 "\n", mHdr.getContentSize());
	printf("  ProgID:          0x%016" PRIx64 "\n", mHdr.getProgramId());
	printf("  Content Index:   %" PRIu32 "\n", mHdr.getContentIndex());
#define _SPLIT_VER(ver) ( (ver>>24) & 0xff), ( (ver>>16) & 0xff), ( (ver>>8) & 0xff)
	printf("  SdkAddon Ver.:   v%" PRIu32 " (%d.%d.%d)\n", mHdr.getSdkAddonVersion(), _SPLIT_VER(mHdr.getSdkAddonVersion()));
#undef _SPLIT_VER
	if (mHdr.hasRightsId())
	{
		printf("  RightsId:        ");
		fnd::SimpleTextOutput::hexDump(mHdr.getRightsId(), nn::hac::nca::kRightsIdLen);
	}
	

	if (mBodyKeys.keak_list.size() > 0 && _HAS_BIT(mCliOutputMode, OUTPUT_KEY_DATA))
	{
		printf("  Key Area: \n");
		printf("    <--------------------------------------------------------------------------->\n");
		printf("    | IDX | ENCRYPTED KEY                    | DECRYPTED KEY                    |\n");
		printf("    |-----|----------------------------------|----------------------------------|\n");
		for (size_t i = 0; i < mBodyKeys.keak_list.size(); i++)
		{
			printf("    | %3d | ", mBodyKeys.keak_list[i].index);
			
			_HEXDUMP_L(mBodyKeys.keak_list[i].enc.key, 16);
			//for (size_t j = 0; j < 16; j++) printf("%02x", mBodyKeys.keak_list[i].enc.key[j]);
			
			printf(" | ");
			
			if (mBodyKeys.keak_list[i].decrypted)
				_HEXDUMP_L(mBodyKeys.keak_list[i].dec.key, 16);
			else
				printf("<unable to decrypt>             ");
			
			printf(" |\n");
		}
		printf("    <--------------------------------------------------------------------------->\n");
	}

	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		printf("  Partitions:\n");
		for (size_t i = 0; i < mHdr.getPartitions().size(); i++)
		{
			size_t index = mHdr.getPartitions()[i].index;
			sPartitionInfo& info = mPartitions[index];

			printf("    %d:\n", (int)index);
			printf("      Offset:      0x%" PRIx64 "\n", (uint64_t)info.offset);
			printf("      Size:        0x%" PRIx64 "\n", (uint64_t)info.size);
			printf("      Format Type: %s\n", getFormatTypeStr(info.format_type));
			printf("      Hash Type:   %s\n", getHashTypeStr(info.hash_type));
			printf("      Enc. Type:   %s\n", getEncryptionTypeStr(info.enc_type));
			if (info.enc_type == nn::hac::nca::CRYPT_AESCTR)
			{
				printf("        AES-CTR:     ");
				fnd::aes::sAesIvCtr ctr;
				fnd::aes::AesIncrementCounter(info.aes_ctr.iv, info.offset>>4, ctr.iv);
				fnd::SimpleTextOutput::hexDump(ctr.iv, sizeof(fnd::aes::sAesIvCtr));
			}
			if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_INTERGRITY)
			{
				HashTreeMeta& hash_hdr = info.hash_tree_meta;
				printf("      HierarchicalIntegrity Header:\n");
				//printf("        TypeId:            0x%x\n", hash_hdr.type_id.get());
				//printf("        MasterHashSize:    0x%x\n", hash_hdr.master_hash_size.get());
				//printf("        LayerNum:          %d\n", hash_hdr.getLayerInfo().size());
				for (size_t j = 0; j < hash_hdr.getHashLayerInfo().size(); j++)
				{
					printf("        Hash Layer %d:\n", (int)j);
					printf("          Offset:          0x%" PRIx64 "\n", (uint64_t)hash_hdr.getHashLayerInfo()[j].offset);
					printf("          Size:            0x%" PRIx64 "\n", (uint64_t)hash_hdr.getHashLayerInfo()[j].size);
					printf("          BlockSize:       0x%" PRIx32 "\n", (uint32_t)hash_hdr.getHashLayerInfo()[j].block_size);
				}

				printf("        Data Layer:\n");
				printf("          Offset:          0x%" PRIx64 "\n", (uint64_t)hash_hdr.getDataLayer().offset);
				printf("          Size:            0x%" PRIx64 "\n", (uint64_t)hash_hdr.getDataLayer().size);
				printf("          BlockSize:       0x%" PRIx32 "\n", (uint32_t)hash_hdr.getDataLayer().block_size);
				for (size_t j = 0; j < hash_hdr.getMasterHashList().size(); j++)
				{
					printf("        Master Hash %d:     ", (int)j);
					fnd::SimpleTextOutput::hexDump(hash_hdr.getMasterHashList()[j].bytes, sizeof(fnd::sha::sSha256Hash));
				}
			}
			else if (info.hash_type == nn::hac::nca::HASH_HIERARCHICAL_SHA256)
			{
				HashTreeMeta& hash_hdr = info.hash_tree_meta;
				printf("      HierarchicalSha256 Header:\n");
				printf("        Master Hash:       ");
				fnd::SimpleTextOutput::hexDump(hash_hdr.getMasterHashList()[0].bytes, sizeof(fnd::sha::sSha256Hash));
				printf("        HashBlockSize:     0x%" PRIx32 "\n", (uint32_t)hash_hdr.getDataLayer().block_size);
				//printf("        LayerNum:          %d\n", hash_hdr.getLayerInfo().size());
				printf("        Hash Layer:\n");
				printf("          Offset:          0x%" PRIx64 "\n", (uint64_t)hash_hdr.getHashLayerInfo()[0].offset);
				printf("          Size:            0x%" PRIx64 "\n", (uint64_t)hash_hdr.getHashLayerInfo()[0].size);
				printf("        Data Layer:\n");
				printf("          Offset:          0x%" PRIx64 "\n", (uint64_t)hash_hdr.getDataLayer().offset);
				printf("          Size:            0x%" PRIx64 "\n", (uint64_t)hash_hdr.getDataLayer().size);
			}
			//else
			//{
			//	printf("      Hash Superblock:\n");
			//	fnd::SimpleTextOutput::hxdStyleDump(fs_header.hash_superblock, nn::hac::nca::kFsHeaderHashSuperblockLen);
			//}
		}
	}
	
#undef _HEXDUMP_U
#undef _HEXDUMP_L
}


void NcaProcess::processPartitions()
{
	for (size_t i = 0; i < mHdr.getPartitions().size(); i++)
	{
		size_t index = mHdr.getPartitions()[i].index;
		struct sPartitionInfo& partition = mPartitions[index];

		// if the reader is null, skip
		if (partition.reader == nullptr)
		{
			printf("[WARNING] NCA Partition %d not readable.", (int)index);
			if (partition.fail_reason.empty() == false)
			{
				printf(" (%s)", partition.fail_reason.c_str());
			}
			printf("\n");
			continue;
		}

		if (partition.format_type == nn::hac::nca::FORMAT_PFS0)
		{
			PfsProcess pfs;
			pfs.setInputFile(partition.reader, SHARED_IFILE);
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
			//printf("pfs.process(%lx)\n",partition.data_offset);
			pfs.process();
			//printf("pfs.process() end\n");
		}
		else if (partition.format_type == nn::hac::nca::FORMAT_ROMFS)
		{
			RomfsProcess romfs;
			romfs.setInputFile(partition.reader, SHARED_IFILE);
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
			//printf("romfs.process(%lx)\n", partition.data_offset);
			romfs.process();
			//printf("romfs.process() end\n");
		}
	}
}
