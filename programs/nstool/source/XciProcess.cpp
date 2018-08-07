#include <fnd/SimpleTextOutput.h>
#include <nn/hac/XciUtils.h>
#include "OffsetAdjustedIFile.h"
#include "XciProcess.h"

XciProcess::XciProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mKeyset(nullptr),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false),
	mListFs(false),
	mRootPfs(),
	mExtractInfo()
{
}

XciProcess::~XciProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void XciProcess::process()
{
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// read header page
	mFile->read((byte_t*)&mHdrPage, 0, sizeof(nn::hac::sXciHeaderPage));

	// allocate memory for and decrypt sXciHeader
	scratch.alloc(sizeof(nn::hac::sXciHeader));
	nn::hac::XciUtils::decryptXciHeader((const byte_t*)&mHdrPage.header, scratch.data(), mKeyset->xci.header_key.key);

	// validate header signature
	if (mVerify)
	{
		validateXciSignature();
	}

	// deserialise header
	mHdr.fromBytes(scratch.data(), scratch.size());

	// display header
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayHeader();

	// process root partition
	processRootPfs();

	// process partitions
	processPartitionPfs();
}

void XciProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void XciProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void XciProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void XciProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void XciProcess::setPartitionForExtract(const std::string& partition_name, const std::string& extract_path)
{
	mExtractInfo.addElement({partition_name, extract_path});
}

void XciProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}

void XciProcess::displayHeader()
{
	printf("[XCI Header]\n");
	printf("  CardHeaderVersion:      %d\n", mHdr.getCardHeaderVersion());
	printf("  RomSize:                %s", getRomSizeStr(mHdr.getRomSizeType()));
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		printf(" (0x%x)", mHdr.getRomSizeType());
	printf("\n");
	printf("  PackageId:              0x%" PRIx64 "\n", mHdr.getPackageId());
	printf("  Flags:                  0x%x\n", mHdr.getFlags());
	if (mHdr.getFlags() != 0)
	{
		for (uint32_t i = 0; i < 8; i++)
		{
			if (_HAS_BIT(mHdr.getFlags(), i))
			{
				printf("    %s\n", getHeaderFlagStr(i));
			}
		}
	}
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("  InitialData:\n");
		printf("    KekIndex:             %d\n", mHdr.getKekIndex());
		printf("    TitleKeyDecIndex:     %d\n", mHdr.getTitleKeyDecIndex());
		printf("    Hash:\n");
		fnd::SimpleTextOutput::hexDump(mHdr.getInitialDataHash().bytes, sizeof(mHdr.getInitialDataHash().bytes), 0x10, 6);
	}
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("  Enc Header AES-IV:\n");
		fnd::SimpleTextOutput::hexDump(mHdr.getAesCbcIv().iv, sizeof(mHdr.getAesCbcIv().iv), 0x10, 4);
	}
	printf("  SelSec:                 0x%x\n", mHdr.getSelSec());
	printf("  SelT1Key:               0x%x\n", mHdr.getSelT1Key());
	printf("  SelKey:                 0x%x\n", mHdr.getSelKey());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		printf("  RomAreaStartPage:       0x%0x", mHdr.getRomAreaStartPage());
		if (mHdr.getRomAreaStartPage() != (uint32_t)(-1))
			printf(" (0x%" PRIx64 ")", nn::hac::XciUtils::blockToAddr(mHdr.getRomAreaStartPage()));
		printf("\n");

		printf("  BackupAreaStartPage:    0x%0x", mHdr.getBackupAreaStartPage());
		if (mHdr.getBackupAreaStartPage() != (uint32_t)(-1))
			printf(" (0x%" PRIx64 ")", nn::hac::XciUtils::blockToAddr(mHdr.getBackupAreaStartPage()));
		printf("\n");

		printf("  ValidDataEndPage:       0x%x", mHdr.getValidDataEndPage());
		if (mHdr.getValidDataEndPage() != (uint32_t)(-1))
			printf(" (0x%" PRIx64 ")", nn::hac::XciUtils::blockToAddr(mHdr.getValidDataEndPage()));
		printf("\n");

		printf("  LimArea:                0x%x", mHdr.getLimAreaPage());
		if (mHdr.getLimAreaPage() != (uint32_t)(-1))
			printf(" (0x%" PRIx64 ")", nn::hac::XciUtils::blockToAddr(mHdr.getLimAreaPage()));
		printf("\n");

		printf("  PartitionFs Header:\n");
		printf("    Offset:               0x%" PRIx64 "\n", mHdr.getPartitionFsAddress());
		printf("    Size:                 0x%" PRIx64 "\n", mHdr.getPartitionFsSize());
		if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		{
			printf("    Hash:\n");
			fnd::SimpleTextOutput::hexDump(mHdr.getPartitionFsHash().bytes, sizeof(mHdr.getPartitionFsHash().bytes), 0x10, 6);
		}
	}

	
	if (mHdr.getFwVerMinor() != 0)
	{
		printf("[XCI Extended Header]\n");
		printf("  FwVersion:              v%d.%d\n", mHdr.getFwVerMajor(), mHdr.getFwVerMinor());
		printf("  AccCtrl1:               0x%x\n", mHdr.getAccCtrl1());
		printf("    CardClockRate:        %s\n", getCardClockRate(mHdr.getAccCtrl1()));
		printf("  Wait1TimeRead:          0x%x\n", mHdr.getWait1TimeRead());
		printf("  Wait2TimeRead:          0x%x\n", mHdr.getWait2TimeRead());
		printf("  Wait1TimeWrite:         0x%x\n", mHdr.getWait1TimeWrite());
		printf("  Wait2TimeWrite:         0x%x\n", mHdr.getWait2TimeWrite());
		printf("  FwMode:                 0x%x\n", mHdr.getFwMode());
		printf("  Update Partition Info:\n");
#define _SPLIT_VER(ver) ( (ver>>26) & 0x3f), ( (ver>>20) & 0x3f), ( (ver>>16) & 0xf), (ver & 0xffff)
		printf("    CUP Version:          v%" PRId32 " (%d.%d.%d.%d)\n", mHdr.getUppVersion(), _SPLIT_VER(mHdr.getUppVersion()));
#undef _SPLIT_VER
		printf("    CUP TitleId:          %016" PRIx64 "\n", mHdr.getUppId());
		printf("    Partition Hash:       ");
		fnd::SimpleTextOutput::hexDump(mHdr.getUppHash(), 8);
	}	
}

bool XciProcess::validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash)
{
	fnd::Vec<byte_t> scratch;
	fnd::sha::sSha256Hash calc_hash;
	scratch.alloc(len);
	mFile->read(scratch.data(), offset, scratch.size());
	fnd::sha::Sha256(scratch.data(), scratch.size(), calc_hash.bytes);
	return calc_hash.compare(test_hash);
}

void XciProcess::validateXciSignature()
{
	fnd::sha::sSha256Hash calc_hash;
	fnd::sha::Sha256((byte_t*)&mHdrPage.header, sizeof(nn::hac::sXciHeader), calc_hash.bytes);
	if (fnd::rsa::pkcs::rsaVerify(mKeyset->xci.header_sign_key, fnd::sha::HASH_SHA256, calc_hash.bytes, mHdrPage.signature) != 0)
	{
		printf("[WARNING] XCI Header Signature: FAIL \n");
	}
}

void XciProcess::processRootPfs()
{
	if (mVerify && validateRegionOfFile(mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize(), mHdr.getPartitionFsHash().bytes) == false)
	{
		printf("[WARNING] XCI Root HFS0: FAIL (bad hash)\n");
	}
	mRootPfs.setInputFile(new OffsetAdjustedIFile(mFile, SHARED_IFILE, mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize()), OWN_IFILE);
	mRootPfs.setListFs(mListFs);
	mRootPfs.setVerifyMode(false);
	mRootPfs.setCliOutputMode(mCliOutputMode);
	mRootPfs.setMountPointName(kXciMountPointName);
	mRootPfs.process();
}

void XciProcess::processPartitionPfs()
{
	const fnd::List<nn::hac::PfsHeader::sFile>& rootPartitions = mRootPfs.getPfsHeader().getFileList();
	for (size_t i = 0; i < rootPartitions.size(); i++)
	{
		// this must be validated here because only the size of the root partiton header is known at verification time
		if (mVerify && validateRegionOfFile(mHdr.getPartitionFsAddress() + rootPartitions[i].offset, rootPartitions[i].hash_protected_size, rootPartitions[i].hash.bytes) == false)
		{
			printf("[WARNING] XCI %s Partition HFS0: FAIL (bad hash)\n", rootPartitions[i].name.c_str());
		}

		PfsProcess tmp;
		tmp.setInputFile(new OffsetAdjustedIFile(mFile, SHARED_IFILE, mHdr.getPartitionFsAddress() + rootPartitions[i].offset, rootPartitions[i].size), OWN_IFILE);
		tmp.setListFs(mListFs);
		tmp.setVerifyMode(mVerify);
		tmp.setCliOutputMode(mCliOutputMode);
		tmp.setMountPointName(kXciMountPointName + rootPartitions[i].name);
		if (mExtractInfo.hasElement<std::string>(rootPartitions[i].name))
			tmp.setExtractPath(mExtractInfo.getElement<std::string>(rootPartitions[i].name).extract_path);
	
		tmp.process();
	}
}

const char* XciProcess::getRomSizeStr(byte_t rom_size) const
{
	const char* str = "unknown";
	switch (rom_size)
	{
		case (nn::hac::xci::ROM_SIZE_1GB):
			str = "1GB";
			break;
		case (nn::hac::xci::ROM_SIZE_2GB):
			str = "2GB";
			break;
		case (nn::hac::xci::ROM_SIZE_4GB):
			str = "4GB";
			break;
		case (nn::hac::xci::ROM_SIZE_8GB):
			str = "8GB";
			break;
		case (nn::hac::xci::ROM_SIZE_16GB):
			str = "16GB";
			break;
		case (nn::hac::xci::ROM_SIZE_32GB):
			str = "32GB";
			break;
	}
	return str;
}

const char* XciProcess::getHeaderFlagStr(byte_t flag) const
{
	const char* str = "unknown";
	switch (flag)
	{
		case (nn::hac::xci::FLAG_AUTOBOOT):
			str = "AutoBoot";
			break;
		case (nn::hac::xci::FLAG_HISTORY_ERASE):
			str = "HistoryErase";
			break;
		case (nn::hac::xci::FLAG_REPAIR_TOOL):
			str = "RepairTool";
			break;
	}
	return str;
}


const char* XciProcess::getCardClockRate(uint32_t acc_ctrl_1) const
{
	const char* str = "unknown";
	switch (acc_ctrl_1)
	{
		case (nn::hac::xci::CLOCK_RATE_25):
			str = "20 MHz";
			break;
		case (nn::hac::xci::CLOCK_RATE_50):
			str = "50 MHz";
			break;

	}
	return str;
}
