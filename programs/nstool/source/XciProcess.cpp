#include <fnd/SimpleTextOutput.h>
#include <nx/XciUtils.h>
#include "OffsetAdjustedIFile.h"
#include "XciProcess.h"

XciProcess::XciProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mKeyset(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
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
	fnd::MemoryBlob scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// read header page
	mFile->read((byte_t*)&mHdrPage, 0, sizeof(nx::sXciHeaderPage));

	// allocate memory for and decrypt sXciHeader
	scratch.alloc(sizeof(nx::sXciHeader));
	nx::XciUtils::decryptXciHeader((const byte_t*)&mHdrPage.header, scratch.getBytes(), mKeyset->xci.header_key.key);

	// validate header signature
	if (mVerify)
	{
		validateXciSignature();
	}

	// deserialise header
	mHdr.importBinary(scratch.getBytes(), scratch.getSize());

	// display header
	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		displayHeader();
	}

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

void XciProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void XciProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void XciProcess::setPartitionForExtract(const std::string& partition_name, const std::string& extract_path)
{
	mExtractInfo.push_back({partition_name, extract_path});
}

void XciProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}

inline const char* getBoolStr(bool isTrue)
{
	return isTrue? "TRUE" : "FALSE";
}

inline const char* getRomSizeStr(byte_t rom_size)
{
	const char* str = "unknown";
	switch (rom_size)
	{
		case (nx::xci::ROM_SIZE_1GB) :
			str = "1GB";
			break;
		case (nx::xci::ROM_SIZE_2GB) :
			str = "2GB";
			break;
		case (nx::xci::ROM_SIZE_4GB) :
			str = "4GB";
			break;
		case (nx::xci::ROM_SIZE_8GB) :
			str = "8GB";
			break;
		case (nx::xci::ROM_SIZE_16GB) :
			str = "16GB";
			break;
		case (nx::xci::ROM_SIZE_32GB) :
			str = "32GB";
			break;
	}
	return str;
}

inline const char* getCardClockRate(uint32_t acc_ctrl_1)
{
	const char* str = "unknown";
	switch (acc_ctrl_1)
	{
		case (nx::xci::CLOCK_RATE_25) :
			str = "20 MHz";
			break;
		case (nx::xci::CLOCK_RATE_50) :
			str = "50 MHz";
			break;
		
	}
	return str;
}

void XciProcess::displayHeader()
{
	printf("[XCI HEADER]\n");
	printf("  Magic:                HEAD\n");
	printf("  RomAreaStartPage:     0x%0x", mHdr.getRomAreaStartPage());
	if (mHdr.getRomAreaStartPage() != (uint32_t)(-1))
		printf(" (0x%" PRIx64 ")", nx::XciUtils::blockToAddr(mHdr.getRomAreaStartPage()));
	printf("\n");
	printf("  BackupAreaStartPage:  0x%0x", mHdr.getBackupAreaStartPage());
	if (mHdr.getBackupAreaStartPage() != (uint32_t)(-1))
		printf(" (0x%" PRIx64 ")", nx::XciUtils::blockToAddr(mHdr.getBackupAreaStartPage()));
	printf("\n");
	printf("  KekIndex:             %d\n", mHdr.getKekIndex());
	printf("  TitleKeyDecIndex:     %d\n", mHdr.getTitleKeyDecIndex());
	printf("  RomSize:              0x%x (%s)\n", mHdr.getRomSizeType(), getRomSizeStr(mHdr.getRomSizeType()));
	printf("  CardHeaderVersion:    %d\n", mHdr.getCardHeaderVersion());
	printf("  Flags:                0x%x\n", mHdr.getFlags());
	printf("    AutoBoot:           %s\n", getBoolStr(_HAS_BIT(mHdr.getFlags(), nx::xci::FLAG_AUTOBOOT)));
	printf("    HistoryErase:       %s\n", getBoolStr(_HAS_BIT(mHdr.getFlags(), nx::xci::FLAG_HISTORY_ERASE)));
	printf("    RepairTool:         %s\n", getBoolStr(_HAS_BIT(mHdr.getFlags(), nx::xci::FLAG_REPAIR_TOOL)));
	printf("  PackageId:            0x%" PRIx64 "\n", mHdr.getPackageId());
	printf("  ValidDataEndPage:     0x%x", mHdr.getValidDataEndPage());
	if (mHdr.getValidDataEndPage() != (uint32_t)(-1))
		printf(" (0x%" PRIx64 ")", nx::XciUtils::blockToAddr(mHdr.getValidDataEndPage()));
	printf("\n");
	printf("  AesIv:                ");
	fnd::SimpleTextOutput::hexDump(mHdr.getAesCbcIv().iv, sizeof(mHdr.getAesCbcIv().iv));
	printf("  PartitionFs:\n");
	printf("    Offset:             0x%" PRIx64 "\n", mHdr.getPartitionFsAddress());
	printf("    Size:               0x%" PRIx64 "\n", mHdr.getPartitionFsSize());
	printf("    Hash:               ");
	fnd::SimpleTextOutput::hexDump(mHdr.getPartitionFsHash().bytes, sizeof(mHdr.getPartitionFsHash().bytes));
	printf("  InitialData:\n");
	printf("    Hash:               ");
	fnd::SimpleTextOutput::hexDump(mHdr.getInitialDataHash().bytes, sizeof(mHdr.getInitialDataHash().bytes));
	printf("  SelSec:               0x%x\n", mHdr.getSelSec());
	printf("  SelT1Key:             0x%x\n", mHdr.getSelT1Key());
	printf("  SelKey:               0x%x\n", mHdr.getSelKey());
	printf("  LimArea:              0x%x", mHdr.getLimAreaPage());
	if (mHdr.getLimAreaPage() != -1)
		printf(" (0x%" PRIx64 ")", nx::XciUtils::blockToAddr(mHdr.getLimAreaPage()));
	printf("\n");


	printf("  FwVersion:            v%d.%d\n", mHdr.getFwVerMajor(), mHdr.getFwVerMinor());
	printf("  AccCtrl1:             0x%x\n", mHdr.getAccCtrl1());
	printf("    CardClockRate:      %s\n", getCardClockRate(mHdr.getAccCtrl1()));
	printf("  Wait1TimeRead:        0x%x\n", mHdr.getWait1TimeRead());
	printf("  Wait2TimeRead:        0x%x\n", mHdr.getWait2TimeRead());
	printf("  Wait1TimeWrite:       0x%x\n", mHdr.getWait1TimeWrite());
	printf("  Wait2TimeWrite:       0x%x\n", mHdr.getWait2TimeWrite());
	printf("  FwMode:               0x%x\n", mHdr.getFwMode());
#define _SPLIT_VER(ver) ( (ver>>26) & 0x3f), ( (ver>>20) & 0x3f), ( (ver>>16) & 0xf), (ver & 0xffff)
	printf("  UppVersion:           v%" PRId32 " (%d.%d.%d.%d)\n", mHdr.getUppVersion(), _SPLIT_VER(mHdr.getUppVersion()));
#undef _SPLIT_VER
	printf("  UppHash:              ");
	fnd::SimpleTextOutput::hexDump(mHdr.getUppHash(), 8);
	printf("  UppId:                %016" PRIx64 "\n", mHdr.getUppId());

}

bool XciProcess::validateRegionOfFile(size_t offset, size_t len, const byte_t* test_hash)
{
	fnd::MemoryBlob scratch;
	crypto::sha::sSha256Hash calc_hash;
	scratch.alloc(len);
	mFile->read(scratch.getBytes(), offset, len);
	crypto::sha::Sha256(scratch.getBytes(), scratch.getSize(), calc_hash.bytes);
	return calc_hash.compare(test_hash);
}

void XciProcess::validateXciSignature()
{
	crypto::sha::sSha256Hash calc_hash;
	crypto::sha::Sha256((byte_t*)&mHdrPage.header, sizeof(nx::sXciHeader), calc_hash.bytes);
	if (crypto::rsa::pkcs::rsaVerify(mKeyset->xci.header_sign_key, crypto::sha::HASH_SHA256, calc_hash.bytes, mHdrPage.signature) != 0)
	{
		// this is minimal even though it's a warning because it's a validation method
		if (mCliOutputType >= OUTPUT_MINIMAL)
			printf("[WARNING] XCI Header Signature: FAIL \n");
	}
}

void XciProcess::processRootPfs()
{
	if (mVerify)
	{
		if (validateRegionOfFile(mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize(), mHdr.getPartitionFsHash().bytes) == false)
		{
			printf("[WARNING] XCI Root HFS0: FAIL (bad hash)\n");
		}
	}
	mRootPfs.setInputFile(new OffsetAdjustedIFile(mFile, SHARED_IFILE, mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize()), OWN_IFILE);
	mRootPfs.setListFs(mListFs);
	mRootPfs.setVerifyMode(mVerify);
	mRootPfs.setCliOutputMode(mCliOutputType);
	mRootPfs.setMountPointName(kXciMountPointName);
	mRootPfs.process();
}

void XciProcess::processPartitionPfs()
{
	const fnd::List<nx::PfsHeader::sFile>& rootPartitions = mRootPfs.getPfsHeader().getFileList();
	for (size_t i = 0; i < rootPartitions.getSize(); i++)
	{	
		PfsProcess tmp;
		tmp.setInputFile(new OffsetAdjustedIFile(mFile, SHARED_IFILE, mHdr.getPartitionFsAddress() + rootPartitions[i].offset, rootPartitions[i].size), OWN_IFILE);
		tmp.setListFs(mListFs);
		tmp.setVerifyMode(mVerify);
		tmp.setCliOutputMode(mCliOutputType);
		tmp.setMountPointName(kXciMountPointName + rootPartitions[i].name);
		for (size_t j = 0; j < mExtractInfo.size(); j++)
		{
			if (mExtractInfo[j].partition_name == rootPartitions[i].name)
			{
				tmp.setExtractPath(mExtractInfo[j].extract_path);
			}
		}
		tmp.process();
	}
}