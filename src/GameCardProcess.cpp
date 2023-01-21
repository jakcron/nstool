#include "GameCardProcess.h"

#include <tc/crypto.h>
#include <tc/io/IOUtil.h>

#include <pietendo/hac/GameCardUtil.h>
#include <pietendo/hac/ContentMetaUtil.h>
#include <pietendo/hac/ContentArchiveUtil.h>

#include <pietendo/hac/GameCardFsSnapshotGenerator.h>
#include "FsProcess.h"


nstool::GameCardProcess::GameCardProcess() :
	mModuleName("nstool::GameCardProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mIsTrueSdkXci(false),
	mIsSdkXciEncrypted(false),
	mGcHeaderOffset(0),
	mProccessExtendedHeader(false),
	mFileSystem(),
	mFsProcess()
{
}

void nstool::GameCardProcess::process()
{
	importHeader();

	// validate header signature
	if (mVerify)
		validateXciSignature();

	// display header
	if (mCliOutputMode.show_basic_info)
		displayHeader();

	// process nested HFS0
	processRootPfs();
}

void nstool::GameCardProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::GameCardProcess::setKeyCfg(const KeyBag& keycfg)
{
	mKeyCfg = keycfg;
}

void nstool::GameCardProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::GameCardProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::GameCardProcess::setShowFsTree(bool show_fs_tree)
{
	mFsProcess.setShowFsTree(show_fs_tree);
}

void nstool::GameCardProcess::setExtractJobs(const std::vector<nstool::ExtractJob> extract_jobs)
{
	mFsProcess.setExtractJobs(extract_jobs);
}

void nstool::GameCardProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}
	
	// check stream is large enough for header
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(pie::hac::sSdkGcHeader)))
	{
		throw tc::Exception(mModuleName, "Corrupt GameCard Image: File too small.");
	}

	// allocate memory for header
	tc::ByteData scratch = tc::ByteData(sizeof(pie::hac::sSdkGcHeader));

	// read header region
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// determine if this is a SDK XCI or a "Community" XCI
	if (((pie::hac::sSdkGcHeader*)scratch.data())->signed_header.header.st_magic.unwrap() == pie::hac::gc::kGcHeaderStructMagic)
	{
		mIsTrueSdkXci = true;
		mGcHeaderOffset = sizeof(pie::hac::sGcKeyDataRegion);
	}
	else if (((pie::hac::sGcHeader_Rsa2048Signed*)scratch.data())->header.st_magic.unwrap() == pie::hac::gc::kGcHeaderStructMagic)
	{
		mIsTrueSdkXci = false;
		mGcHeaderOffset = 0;
	}
	else 
	{
		throw tc::Exception(mModuleName, "Corrupt GameCard Image: Unexpected magic bytes.");
	}

	pie::hac::sGcHeader_Rsa2048Signed* hdr_ptr = (pie::hac::sGcHeader_Rsa2048Signed*)(scratch.data() + mGcHeaderOffset);

	// generate hash of raw header
	tc::crypto::GenerateSha2256Hash(mHdrHash.data(), (byte_t*)&hdr_ptr->header, sizeof(pie::hac::sGcHeader));
	
	// save the signature
	memcpy(mHdrSignature.data(), hdr_ptr->signature.data(), mHdrSignature.size());
	
	// decrypt extended header
	byte_t xci_header_key_index = hdr_ptr->header.key_flag & 0xf;
	if (mKeyCfg.xci_header_key.find(xci_header_key_index) != mKeyCfg.xci_header_key.end())
	{
		pie::hac::GameCardUtil::decryptXciHeader(&hdr_ptr->header, mKeyCfg.xci_header_key[xci_header_key_index].data());
		mProccessExtendedHeader = true;
	}
	
	// deserialise header
	mHdr.fromBytes((byte_t*)&hdr_ptr->header, sizeof(pie::hac::sGcHeader));
}

void nstool::GameCardProcess::displayHeader()
{
	const pie::hac::sGcHeader* raw_hdr = (const pie::hac::sGcHeader*)mHdr.getBytes().data();

	fmt::print("[GameCard/Header]\n");
	fmt::print("  CardHeaderVersion:      {:d}\n", mHdr.getCardHeaderVersion());
	fmt::print("  RomSize:                {:s}", pie::hac::GameCardUtil::getRomSizeAsString((pie::hac::gc::RomSize)mHdr.getRomSizeType()));
	if (mCliOutputMode.show_extended_info)
		fmt::print(" (0x{:x})", mHdr.getRomSizeType());
	fmt::print("\n");
	fmt::print("  PackageId:              0x{:016x}\n", mHdr.getPackageId());
	fmt::print("  Flags:                  0x{:02x}\n", *((byte_t*)&raw_hdr->flags));
	for (auto itr = mHdr.getFlags().begin(); itr != mHdr.getFlags().end(); itr++)
	{
		fmt::print("    {:s}\n", pie::hac::GameCardUtil::getHeaderFlagsAsString((pie::hac::gc::HeaderFlags)*itr));
	}
	
	
	if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  KekIndex:               {:s} ({:d})\n", pie::hac::GameCardUtil::getKekIndexAsString((pie::hac::gc::KekIndex)mHdr.getKekIndex()), mHdr.getKekIndex());
		fmt::print("  TitleKeyDecIndex:       {:d}\n", mHdr.getTitleKeyDecIndex());
		fmt::print("  InitialData:\n");
		fmt::print("    Hash:\n");
		fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(mHdr.getInitialDataHash().data(), mHdr.getInitialDataHash().size(), true, "", 0x10, 6, false));
	}
	if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  Extended Header AesCbc IV:\n");
		fmt::print("    {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getAesCbcIv().data(), mHdr.getAesCbcIv().size(), true, ""));
	}
	fmt::print("  SelSec:                 0x{:x}\n", mHdr.getSelSec());
	fmt::print("  SelT1Key:               0x{:x}\n", mHdr.getSelT1Key());
	fmt::print("  SelKey:                 0x{:x}\n", mHdr.getSelKey());
	if (mCliOutputMode.show_layout)
	{
		fmt::print("  RomAreaStartPage:       0x{:x}", mHdr.getRomAreaStartPage());
		if (mHdr.getRomAreaStartPage() != (uint32_t)(-1))
			fmt::print(" (0x{:x})", pie::hac::GameCardUtil::blockToAddr(mHdr.getRomAreaStartPage()));
		fmt::print("\n");

		fmt::print("  BackupAreaStartPage:    0x{:x}", mHdr.getBackupAreaStartPage());
		if (mHdr.getBackupAreaStartPage() != (uint32_t)(-1))
			fmt::print(" (0x{:x})", pie::hac::GameCardUtil::blockToAddr(mHdr.getBackupAreaStartPage()));
		fmt::print("\n");

		fmt::print("  ValidDataEndPage:       0x{:x}", mHdr.getValidDataEndPage());
		if (mHdr.getValidDataEndPage() != (uint32_t)(-1))
			fmt::print(" (0x{:x})", pie::hac::GameCardUtil::blockToAddr(mHdr.getValidDataEndPage()));
		fmt::print("\n");

		fmt::print("  LimArea:                0x{:x}", mHdr.getLimAreaPage());
		if (mHdr.getLimAreaPage() != (uint32_t)(-1))
			fmt::print(" (0x{:x})", pie::hac::GameCardUtil::blockToAddr(mHdr.getLimAreaPage()));
		fmt::print("\n");

		fmt::print("  PartitionFs Header:\n");
		fmt::print("    Offset:               0x{:x}\n", mHdr.getPartitionFsAddress());
		fmt::print("    Size:                 0x{:x}\n", mHdr.getPartitionFsSize());
		if (mCliOutputMode.show_extended_info)
		{
			fmt::print("    Hash:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(mHdr.getPartitionFsHash().data(), mHdr.getPartitionFsHash().size(), true, "", 0x10, 6, false));
		}
	}

	
	if (mProccessExtendedHeader)
	{
		fmt::print("[GameCard/ExtendedHeader]\n");
		fmt::print("  FwVersion:              v{:d} ({:s})\n", mHdr.getFwVersion(), pie::hac::GameCardUtil::getCardFwVersionDescriptionAsString((pie::hac::gc::FwVersion)mHdr.getFwVersion()));
		fmt::print("  AccCtrl1:               0x{:x}\n", mHdr.getAccCtrl1());
		fmt::print("    CardClockRate:        {:s}\n", pie::hac::GameCardUtil::getCardClockRateAsString((pie::hac::gc::CardClockRate)mHdr.getAccCtrl1()));
		fmt::print("  Wait1TimeRead:          0x{:x}\n", mHdr.getWait1TimeRead());
		fmt::print("  Wait2TimeRead:          0x{:x}\n", mHdr.getWait2TimeRead());
		fmt::print("  Wait1TimeWrite:         0x{:x}\n", mHdr.getWait1TimeWrite());
		fmt::print("  Wait2TimeWrite:         0x{:x}\n", mHdr.getWait2TimeWrite());
		fmt::print("  SdkAddon Version:       {:s} (v{:d})\n", pie::hac::ContentArchiveUtil::getSdkAddonVersionAsString(mHdr.getFwMode()), mHdr.getFwMode());
		fmt::print("  CompatibilityType:      {:s} ({:d})\n", pie::hac::GameCardUtil::getCompatibilityTypeAsString((pie::hac::gc::CompatibilityType)mHdr.getCompatibilityType()), mHdr.getCompatibilityType());
		fmt::print("  Update Partition Info:\n");
		fmt::print("    CUP Version:          {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mHdr.getUppVersion()), mHdr.getUppVersion());
		fmt::print("    CUP TitleId:          0x{:016x}\n", mHdr.getUppId());
		fmt::print("    CUP Digest:           {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getUppHash().data(), mHdr.getUppHash().size(), true, ""));
	}
}

bool nstool::GameCardProcess::validateRegionOfFile(int64_t offset, int64_t len, const byte_t* test_hash, bool use_salt, byte_t salt)
{
	// read region into memory
	tc::ByteData scratch = tc::ByteData(tc::io::IOUtil::castInt64ToSize(len));
	mFile->seek(offset, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// update hash
	tc::crypto::Sha2256Generator sha256_gen;
	sha256_gen.initialize();
	sha256_gen.update(scratch.data(), scratch.size());
	if (use_salt)
		sha256_gen.update(&salt, sizeof(salt));

	// calculate hash
	pie::hac::detail::sha256_hash_t calc_hash;
	sha256_gen.getHash(calc_hash.data());

	return memcmp(calc_hash.data(), test_hash, calc_hash.size()) == 0;
}

bool nstool::GameCardProcess::validateRegionOfFile(int64_t offset, int64_t len, const byte_t* test_hash)
{
	return validateRegionOfFile(offset, len, test_hash, false, 0);
}

void nstool::GameCardProcess::validateXciSignature()
{
	if (mKeyCfg.xci_header_sign_key.isSet())
	{
		if (tc::crypto::VerifyRsa2048Pkcs1Sha2256(mHdrSignature.data(), mHdrHash.data(), mKeyCfg.xci_header_sign_key.get()) == false)
		{
			fmt::print("[WARNING] GameCard Header Signature: FAIL\n");
		}
	}
	else 
	{
		fmt::print("[WARNING] GameCard Header Signature: FAIL (Failed to load rsa public key.)\n");
	}
}

void nstool::GameCardProcess::processRootPfs()
{
	if (mVerify && validateRegionOfFile(mHdr.getPartitionFsAddress(), mHdr.getPartitionFsSize(), mHdr.getPartitionFsHash().data(), mHdr.getCompatibilityType() != pie::hac::gc::CompatibilityType_Global, mHdr.getCompatibilityType()) == false)
	{
		fmt::print("[WARNING] GameCard Root HFS0: FAIL (bad hash)\n");
	}

	std::shared_ptr<tc::io::IStream> gc_fs_raw = std::make_shared<tc::io::SubStream>(tc::io::SubStream(mFile, mHdr.getPartitionFsAddress(), pie::hac::GameCardUtil::blockToAddr(mHdr.getValidDataEndPage()+1) - mHdr.getPartitionFsAddress()));

	auto gc_vfs_snapshot = pie::hac::GameCardFsSnapshotGenerator(gc_fs_raw, mHdr.getPartitionFsSize(), mVerify ? pie::hac::GameCardFsSnapshotGenerator::ValidationMode_Warn : pie::hac::GameCardFsSnapshotGenerator::ValidationMode_None);
	mFileSystem = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(gc_vfs_snapshot) );

	mFsProcess.setInputFileSystem(mFileSystem);
	mFsProcess.setFsFormatName("PartitionFs");
	mFsProcess.setFsProperties({
		fmt::format("Type:      Nested HFS0"),
		fmt::format("DirNum:    {:d}", gc_vfs_snapshot.dir_entries.empty() ? 0 : gc_vfs_snapshot.dir_entries.size() - 1), // -1 to not include root directory
		fmt::format("FileNum:   {:d}", gc_vfs_snapshot.file_entries.size())
	});
	mFsProcess.setShowFsInfo(mCliOutputMode.show_basic_info);
	mFsProcess.setFsRootLabel(kXciMountPointName);
	mFsProcess.process();
}