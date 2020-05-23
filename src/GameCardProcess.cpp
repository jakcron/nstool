#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <nn/hac/GameCardUtil.h>
#include <nn/hac/ContentMetaUtil.h>
#include <nn/hac/ContentArchiveUtil.h>
#include "GameCardProcess.h"

GameCardProcess::GameCardProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false),
	mListFs(false),
	mProccessExtendedHeader(false),
	mRootPfs(),
	mExtractInfo()
{
}

void GameCardProcess::process()
{
	importHeader();

	// validate header signature
	if (mVerify)
		validateXciSignature();

	// display header
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayHeader();

	// process root partition
	processRootPfs();

	// process partitions
	processPartitionPfs();
}

void GameCardProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void GameCardProcess::setKeyCfg(const KeyConfiguration& keycfg)
{
	mKeyCfg = keycfg;
}

void GameCardProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void GameCardProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void GameCardProcess::setPartitionForExtract(const std::string& partition_name, const std::string& extract_path)
{
	mExtractInfo.addElement({partition_name, extract_path});
}

void GameCardProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}

void GameCardProcess::importHeader()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// allocate memory for header
	scratch.alloc(sizeof(nn::hac::sSdkGcHeader));

	// read header region
	(*mFile)->read((byte_t*)scratch.data(), 0, sizeof(nn::hac::sSdkGcHeader));

	// determine if this is a SDK XCI or a "Community" XCI
	if (((nn::hac::sSdkGcHeader*)scratch.data())->signed_header.header.st_magic.get() == nn::hac::gc::kGcHeaderStructMagic)
	{
		mIsTrueSdkXci = true;
		mGcHeaderOffset = sizeof(nn::hac::sGcKeyDataRegion);
	}
	else if (((nn::hac::sGcHeader_Rsa2048Signed*)scratch.data())->header.st_magic.get() == nn::hac::gc::kGcHeaderStructMagic)
	{
		mIsTrueSdkXci = false;
		mGcHeaderOffset = 0;
	}
	else 
	{
		throw fnd::Exception(kModuleName, "GameCard image did not have expected magic bytes");
	}

	nn::hac::sGcHeader_Rsa2048Signed* hdr_ptr = (nn::hac::sGcHeader_Rsa2048Signed*)(scratch.data() + mGcHeaderOffset);

	// generate hash of raw header
	fnd::sha::Sha256((byte_t*)&hdr_ptr->header, sizeof(nn::hac::sGcHeader), mHdrHash.bytes);
	
	// save the signature
	memcpy(mHdrSignature, hdr_ptr->signature, fnd::rsa::kRsa2048Size);
	
	// decrypt extended header
	fnd::aes::sAes128Key header_key;
	if (mKeyCfg.getXciHeaderKey(header_key))
	{
		nn::hac::GameCardUtil::decryptXciHeader(&hdr_ptr->header, header_key.key);
		mProccessExtendedHeader = true;
	}
	
	// deserialise header
	mHdr.fromBytes((byte_t*)&hdr_ptr->header, sizeof(nn::hac::sGcHeader));
}

void GameCardProcess::displayHeader()
{
	std::cout << "[GameCard Header]" << std::endl;
	std::cout << "  CardHeaderVersion:      " << std::dec << (uint32_t)mHdr.getCardHeaderVersion() << std::endl;
	std::cout << "  RomSize:                " << nn::hac::GameCardUtil::getRomSizeAsString((nn::hac::gc::RomSize)mHdr.getRomSizeType());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << " (0x" << std::hex << (uint32_t)mHdr.getRomSizeType() << ")";
	std::cout << std::endl;
	std::cout << "  PackageId:              0x" << std::hex << std::setw(16) << std::setfill('0') << mHdr.getPackageId() << std::endl;
	std::cout << "  Flags:                  0x" << std::dec << (uint32_t)mHdr.getFlags() << std::endl;
	if (mHdr.getFlags() != 0)
	{
		for (uint32_t i = 0; i < 8; i++)
		{
			if (_HAS_BIT(mHdr.getFlags(), i))
			{
				std::cout << "    " << nn::hac::GameCardUtil::getHeaderFlagsAsString((nn::hac::gc::HeaderFlags)i) << std::endl;
			}
		}
	}
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  KekIndex:               " << nn::hac::GameCardUtil::getKekIndexAsString((nn::hac::gc::KekIndex)mHdr.getKekIndex()) << " (" << std::dec << (uint32_t)mHdr.getKekIndex() << ")" << std::endl;
		std::cout << "  TitleKeyDecIndex:       " << std::dec << (uint32_t)mHdr.getTitleKeyDecIndex() << std::endl;
		std::cout << "  InitialData:" << std::endl;
		std::cout << "    Hash:" << std::endl;
		std::cout << "      " << fnd::SimpleTextOutput::arrayToString(mHdr.getInitialDataHash().bytes, 0x10, true, ":") << std::endl;
		std::cout << "      " << fnd::SimpleTextOutput::arrayToString(mHdr.getInitialDataHash().bytes+0x10, 0x10, true, ":") << std::endl;
	}
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Extended Header AesCbc IV:" << std::endl;
		std::cout << "    " << fnd::SimpleTextOutput::arrayToString(mHdr.getAesCbcIv().iv, sizeof(mHdr.getAesCbcIv().iv), true, ":") << std::endl;
	}
	std::cout << "  SelSec:                 0x" << std::hex << mHdr.getSelSec() << std::endl;
	std::cout << "  SelT1Key:               0x" << std::hex << mHdr.getSelT1Key() << std::endl;
	std::cout << "  SelKey:                 0x" << std::hex << mHdr.getSelKey() << std::endl;
	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		std::cout << "  RomAreaStartPage:       0x" << std::hex << mHdr.getRomAreaStartPage();
		if (mHdr.getRomAreaStartPage() != (uint32_t)(-1))
			std::cout << " (0x" << std::hex << nn::hac::GameCardUtil::blockToAddr(mHdr.getRomAreaStartPage()) << ")";
		std::cout << std::endl;

		std::cout << "  BackupAreaStartPage:    0x" << std::hex << mHdr.getBackupAreaStartPage();
		if (mHdr.getBackupAreaStartPage() != (uint32_t)(-1))
			std::cout << " (0x" << std::hex << nn::hac::GameCardUtil::blockToAddr(mHdr.getBackupAreaStartPage()) << ")";
		std::cout << std::endl;

		std::cout << "  ValidDataEndPage:       0x" << std::hex << mHdr.getValidDataEndPage();
		if (mHdr.getValidDataEndPage() != (uint32_t)(-1))
			std::cout << " (0x" << std::hex << nn::hac::GameCardUtil::blockToAddr(mHdr.getValidDataEndPage()) << ")";
		std::cout << std::endl;

		std::cout << "  LimArea:                0x" << std::hex << mHdr.getLimAreaPage();
		if (mHdr.getLimAreaPage() != (uint32_t)(-1))
			std::cout << " (0x" << std::hex << nn::hac::GameCardUtil::blockToAddr(mHdr.getLimAreaPage()) << ")";
		std::cout << std::endl;

		std::cout << "  PartitionFs Header:" << std::endl;
		std::cout << "    Offset:               0x" << std::hex << mHdr.getPartitionFsAddress() << std::endl;
		std::cout << "    Size:                 0x" << std::hex << mHdr.getPartitionFsSize() << std::endl;
		if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		{
			std::cout << "    Hash:" << std::endl;
			std::cout << "      " << fnd::SimpleTextOutput::arrayToString(mHdr.getPartitionFsHash().bytes, 0x10, true, ":") << std::endl;
			std::cout << "      " << fnd::SimpleTextOutput::arrayToString(mHdr.getPartitionFsHash().bytes+0x10, 0x10, true, ":") << std::endl;
		}
	}

	
	if (mProccessExtendedHeader)
	{
		std::cout << "[GameCard Extended Header]" << std::endl;
		std::cout << "  FwVersion:              v" << std::dec << mHdr.getFwVersion() << " (" << nn::hac::GameCardUtil::getCardFwVersionDescriptionAsString((nn::hac::gc::FwVersion)mHdr.getFwVersion()) << ")" << std::endl;
		std::cout << "  AccCtrl1:               0x" << std::hex << mHdr.getAccCtrl1() << std::endl;
		std::cout << "    CardClockRate:        " << nn::hac::GameCardUtil::getCardClockRateAsString((nn::hac::gc::CardClockRate)mHdr.getAccCtrl1()) << std::endl;
		std::cout << "  Wait1TimeRead:          0x" << std::hex << mHdr.getWait1TimeRead() << std::endl;
		std::cout << "  Wait2TimeRead:          0x" << std::hex << mHdr.getWait2TimeRead() << std::endl;
		std::cout << "  Wait1TimeWrite:         0x" << std::hex << mHdr.getWait1TimeWrite() << std::endl;
		std::cout << "  Wait2TimeWrite:         0x" << std::hex << mHdr.getWait2TimeWrite() << std::endl;
		std::cout << "  SdkAddon Version:       " << nn::hac::ContentArchiveUtil::getSdkAddonVersionAsString(mHdr.getFwMode()) << " (v" << std::dec << mHdr.getFwMode() << ")" << std::endl; 
		std::cout << "  CompatibilityType:      " << nn::hac::GameCardUtil::getCompatibilityTypeAsString((nn::hac::gc::CompatibilityType)mHdr.getCompatibilityType()) << " (" << std::dec << (uint32_t) mHdr.getCompatibilityType() << ")" << std::endl;
		std::cout << "  Update Partition Info:" << std::endl;
		std::cout << "    CUP Version:          " << nn::hac::ContentMetaUtil::getVersionAsString(mHdr.getUppVersion()) << " (v" << std::dec << mHdr.getUppVersion() << ")" << std::endl;
		std::cout << "    CUP TitleId:          0x" << std::hex << std::setw(16) << std::setfill('0') << mHdr.getUppId() << std::endl;
		std::cout << "    CUP Digest:           " << fnd::SimpleTextOutput::arrayToString(mHdr.getUppHash(), 8, true, ":") << std::endl;
	}
}

bool GameCardProcess::validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash, bool use_salt, byte_t salt)
{
	fnd::Vec<byte_t> scratch;
	fnd::sha::sSha256Hash calc_hash;
	if (use_salt)
	{
		scratch.alloc(len + 1);		
		scratch.data()[len] = salt;
	}
	else
	{
		scratch.alloc(len);
	}

	(*mFile)->read(scratch.data(), offset, len);
	fnd::sha::Sha256(scratch.data(), scratch.size(), calc_hash.bytes);

	return calc_hash.compare(test_hash);
}

bool GameCardProcess::validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash)
{
	return validateRegionOfFile(offset, len, test_hash, false, 0);
}

void GameCardProcess::validateXciSignature()
{
	fnd::rsa::sRsa2048Key header_sign_key;

	mKeyCfg.getXciHeaderSignKey(header_sign_key);
	if (fnd::rsa::pkcs::rsaVerify(header_sign_key, fnd::sha::HASH_SHA256, mHdrHash.bytes, mHdrSignature) != 0)
	{
		std::cout << "[WARNING] GameCard Header Signature: FAIL" << std::endl;
	}
}

void GameCardProcess::processRootPfs()
{
	if (mVerify && validateRegionOfFile(mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize(), mHdr.getPartitionFsHash().bytes, mHdr.getCompatibilityType() != nn::hac::gc::COMPAT_GLOBAL, mHdr.getCompatibilityType()) == false)
	{
		std::cout << "[WARNING] GameCard Root HFS0: FAIL (bad hash)" << std::endl;
	}
	mRootPfs.setInputFile(new fnd::OffsetAdjustedIFile(mFile, mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize()));
	mRootPfs.setListFs(mListFs);
	mRootPfs.setVerifyMode(false);
	mRootPfs.setCliOutputMode(mCliOutputMode);
	mRootPfs.setMountPointName(kXciMountPointName);
	mRootPfs.process();
}

void GameCardProcess::processPartitionPfs()
{
	const fnd::List<nn::hac::PartitionFsHeader::sFile>& rootPartitions = mRootPfs.getPfsHeader().getFileList();
	for (size_t i = 0; i < rootPartitions.size(); i++)
	{
		// this must be validated here because only the size of the root partiton header is known at verification time
		if (mVerify && validateRegionOfFile(mHdr.getPartitionFsAddress() + rootPartitions[i].offset, rootPartitions[i].hash_protected_size, rootPartitions[i].hash.bytes) == false)
		{
			std::cout << "[WARNING] GameCard " << rootPartitions[i].name << " Partition HFS0: FAIL (bad hash)" << std::endl;
		}

		PfsProcess tmp;
		tmp.setInputFile(new fnd::OffsetAdjustedIFile(mFile, mHdr.getPartitionFsAddress() + rootPartitions[i].offset, rootPartitions[i].size));
		tmp.setListFs(mListFs);
		tmp.setVerifyMode(mVerify);
		tmp.setCliOutputMode(mCliOutputMode);
		tmp.setMountPointName(kXciMountPointName + rootPartitions[i].name);
		if (mExtractInfo.hasElement<std::string>(rootPartitions[i].name))
			tmp.setExtractPath(mExtractInfo.getElement<std::string>(rootPartitions[i].name).extract_path);
	
		tmp.process();
	}
}