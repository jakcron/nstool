#include <nn/hac/XciHeader.h>

nn::hac::XciHeader::XciHeader()
{
	clear();
}

nn::hac::XciHeader::XciHeader(const XciHeader& other)
{
	*this = other;
}

void nn::hac::XciHeader::operator=(const XciHeader& other)
{
	mRomAreaStartPage = other.mRomAreaStartPage;
	mBackupAreaStartPage = other.mBackupAreaStartPage;
	mKekIndex = other.mKekIndex;
	mTitleKeyDecIndex = other.mTitleKeyDecIndex;
	mRomSize = other.mRomSize;
	mCardHeaderVersion = other.mCardHeaderVersion;
	mFlags = other.mFlags;
	mPackageId = other.mPackageId;
	mValidDataEndPage = other.mValidDataEndPage;
	mAesCbcIv = other.mAesCbcIv;
	mPartitionFsHeaderAddress = other.mPartitionFsHeaderAddress;
	mPartitionFsHeaderSize = other.mPartitionFsHeaderSize;
	mPartitionFsHeaderHash = other.mPartitionFsHeaderHash;
	mInitialDataHash = other.mInitialDataHash;
	mSelSec = other.mSelSec;
	mSelT1Key = other.mSelT1Key;
	mSelKey = other.mSelKey;
	mLimAreaPage = other.mLimAreaPage;
	mFwVersion[0] = other.mFwVersion[0];
	mFwVersion[1] = other.mFwVersion[1];
	mAccCtrl1 = other.mAccCtrl1;
	mWait1TimeRead = other.mWait1TimeRead;
	mWait2TimeRead = other.mWait2TimeRead;
	mWait1TimeWrite = other.mWait1TimeWrite;
	mWait2TimeWrite = other.mWait2TimeWrite;
	mFwMode = other.mFwMode;
	mUppVersion = other.mUppVersion;
	memcpy(mUppHash, other.mUppHash, xci::kUppHashLen);
	mUppId = other.mUppId;
}

bool nn::hac::XciHeader::operator==(const XciHeader& other) const
{
	return	(mRomAreaStartPage == other.mRomAreaStartPage)
		&&	(mBackupAreaStartPage == other.mBackupAreaStartPage)
		&&	(mKekIndex == other.mKekIndex)
		&&	(mTitleKeyDecIndex == other.mTitleKeyDecIndex)
		&&	(mRomSize == other.mRomSize)
		&&	(mCardHeaderVersion == other.mCardHeaderVersion)
		&&	(mFlags == other.mFlags)
		&&	(mPackageId == other.mPackageId)
		&&	(mValidDataEndPage == other.mValidDataEndPage)
		&&	(mAesCbcIv == other.mAesCbcIv)
		&&	(mPartitionFsHeaderAddress == other.mPartitionFsHeaderAddress)
		&&	(mPartitionFsHeaderSize == other.mPartitionFsHeaderSize)
		&&	(mPartitionFsHeaderHash == other.mPartitionFsHeaderHash)
		&&	(mInitialDataHash == other.mInitialDataHash)
		&&	(mSelSec == other.mSelSec)
		&&	(mSelT1Key == other.mSelT1Key)
		&&	(mSelKey == other.mSelKey)
		&&	(mLimAreaPage == other.mLimAreaPage)
		&&	(mFwVersion[0] == other.mFwVersion[0])
		&&	(mFwVersion[1] == other.mFwVersion[1])
		&&	(mAccCtrl1 == other.mAccCtrl1)
		&&	(mWait1TimeRead == other.mWait1TimeRead)
		&&	(mWait2TimeRead == other.mWait2TimeRead)
		&&	(mWait1TimeWrite == other.mWait1TimeWrite)
		&&	(mWait2TimeWrite == other.mWait2TimeWrite)
		&&	(mFwMode == other.mFwMode)
		&&	(mUppVersion == other.mUppVersion)
		&&	(memcmp(mUppHash, other.mUppHash, xci::kUppHashLen) == 0)
		&&	(mUppId == other.mUppId);
}

bool nn::hac::XciHeader::operator!=(const XciHeader& other) const
{
	return !(*this == other);
}

void nn::hac::XciHeader::toBytes()
{
	fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nn::hac::XciHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input data size
	if (len < sizeof(sXciHeader))
	{
		throw fnd::Exception(kModuleName, "XCI header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sXciHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// get sXciHeader ptr
	const nn::hac::sXciHeader* hdr = (const nn::hac::sXciHeader*)mRawBinary.data();
	
	// check XCI signature
	if (hdr->st_magic.get() != xci::kXciStructMagic)
	{
		throw fnd::Exception(kModuleName, "XCI header corrupt");
	}

	mRomAreaStartPage = hdr->rom_area_start_page.get();
	mBackupAreaStartPage = hdr->backup_area_start_page.get();
	mKekIndex = hdr->key_flag & 7;
	mTitleKeyDecIndex = (hdr->key_flag >> 4) & 7;
	mRomSize = hdr->rom_size;
	mCardHeaderVersion = hdr->card_header_version;
	mFlags = hdr->flags;
	mPackageId = hdr->package_id.get();
	mValidDataEndPage = hdr->valid_data_end_page.get();
	for (size_t i = 0; i < fnd::aes::kAesBlockSize; i++)
		mAesCbcIv.iv[i] = hdr->aescbc_iv.iv[15-i];
	mPartitionFsHeaderAddress = hdr->partition_fs_header_address.get();
	mPartitionFsHeaderSize = hdr->partition_fs_header_size.get();
	mPartitionFsHeaderHash = hdr->partition_fs_header_hash;
	mInitialDataHash = hdr->initial_data_hash;
	mSelSec = hdr->sel_sec.get();
	mSelT1Key = hdr->sel_t1_key.get();
	mSelKey = hdr->sel_key.get();
	mLimAreaPage = hdr->lim_area.get();

	// if decrypted
	if (hdr->reserved_02[sizeof(hdr->reserved_02)-1] == 0x00 && hdr->reserved_02[sizeof(hdr->reserved_02)-2] == 0x00)
	{
		mFwVersion[xci::FWVER_MAJOR] = hdr->fw_version[xci::FWVER_MAJOR].get();
		mFwVersion[xci::FWVER_MINOR] = hdr->fw_version[xci::FWVER_MINOR].get();
		mAccCtrl1 = hdr->acc_ctrl_1.get();
		mWait1TimeRead = hdr->wait_1_time_read.get();
		mWait2TimeRead = hdr->wait_2_time_read.get();
		mWait1TimeWrite = hdr->wait_1_time_write.get();
		mWait2TimeWrite = hdr->wait_2_time_write.get();
		mFwMode = hdr->fw_mode.get();
		mUppVersion = hdr->upp_version.get();
		memcpy(mUppHash, hdr->upp_hash, xci::kUppHashLen);
		mUppId = hdr->upp_id.get();
	}

}

const fnd::Vec<byte_t>& nn::hac::XciHeader::getBytes() const
{
	return mRawBinary;
}

// variables
void nn::hac::XciHeader::clear()
{
	mRomAreaStartPage = 0;
	mBackupAreaStartPage = 0;
	mKekIndex = 0;
	mTitleKeyDecIndex = 0;
	mRomSize = 0;
	mCardHeaderVersion = 0;
	mFlags = 0;
	mPackageId = 0;
	mValidDataEndPage = 0;
	memset(mAesCbcIv.iv, 0, sizeof(mAesCbcIv));
	mPartitionFsHeaderAddress = 0;
	mPartitionFsHeaderSize = 0;
	memset(mPartitionFsHeaderHash.bytes, 0, sizeof(mPartitionFsHeaderHash));
	memset(mInitialDataHash.bytes, 0, sizeof(mInitialDataHash));
	mSelSec = 0;
	mSelT1Key = 0;
	mSelKey = 0;
	mLimAreaPage = 0;
	mFwVersion[0] = 0;
	mFwVersion[1] = 0;
	mAccCtrl1 = 0;
	mWait1TimeRead = 0;
	mWait2TimeRead = 0;
	mWait1TimeWrite = 0;
	mWait2TimeWrite = 0;
	mFwMode = 0;
	mUppVersion = 0;
	memset(mUppHash, 0, xci::kUppHashLen);
	mUppId = 0;
}

uint32_t nn::hac::XciHeader::getRomAreaStartPage() const
{
	return mRomAreaStartPage;
}

void nn::hac::XciHeader::setRomAreaStartPage(uint32_t startPage)
{
	mRomAreaStartPage = startPage;
}

uint32_t nn::hac::XciHeader::getBackupAreaStartPage() const
{
	return mBackupAreaStartPage;
}

void nn::hac::XciHeader::setBackupAreaStartPage(uint32_t startPage)
{
	mBackupAreaStartPage = startPage;
}

byte_t nn::hac::XciHeader::getKekIndex() const
{
	return mKekIndex;
}

void nn::hac::XciHeader::setKekIndex(byte_t kekIndex)
{
	mKekIndex = kekIndex;
}

byte_t nn::hac::XciHeader::getTitleKeyDecIndex() const
{
	return mTitleKeyDecIndex;
}

void nn::hac::XciHeader::setTitleKeyDecIndex(byte_t index)
{
	mTitleKeyDecIndex = index;
}

byte_t nn::hac::XciHeader::getRomSizeType() const
{
	return mRomSize;
}

void nn::hac::XciHeader::setRomSizeType(byte_t romSizeType)
{
	mRomSize = romSizeType;
}

byte_t nn::hac::XciHeader::getCardHeaderVersion() const
{
	return mCardHeaderVersion;
}

void nn::hac::XciHeader::setCardHeaderVersion(byte_t version)
{
	mCardHeaderVersion = version;
}

byte_t nn::hac::XciHeader::getFlags() const
{
	return mFlags;
}

void nn::hac::XciHeader::setFlags(byte_t flags)
{
	mFlags = flags;
}

uint64_t nn::hac::XciHeader::getPackageId() const
{
	return mPackageId;
}

void nn::hac::XciHeader::setPackageId(uint64_t id)
{
	mPackageId = id;
}

uint32_t nn::hac::XciHeader::getValidDataEndPage() const
{
	return mValidDataEndPage;
}

void nn::hac::XciHeader::setValidDataEndPage(uint32_t page)
{
	mValidDataEndPage = page;
}

const fnd::aes::sAesIvCtr& nn::hac::XciHeader::getAesCbcIv() const
{
	return mAesCbcIv;
}

void nn::hac::XciHeader::setAesCbcIv(const fnd::aes::sAesIvCtr& iv)
{
	mAesCbcIv = iv;
}

uint64_t nn::hac::XciHeader::getPartitionFsAddress() const
{
	return mPartitionFsHeaderAddress;
}

void nn::hac::XciHeader::setPartitionFsAddress(uint64_t address)
{
	mPartitionFsHeaderAddress = address;
}

uint64_t nn::hac::XciHeader::getPartitionFsSize() const
{
	return mPartitionFsHeaderSize;
}

void nn::hac::XciHeader::setPartitionFsSize(uint64_t size)
{
	mPartitionFsHeaderSize = size;
}

const fnd::sha::sSha256Hash& nn::hac::XciHeader::getPartitionFsHash() const
{
	return mPartitionFsHeaderHash;
}

void nn::hac::XciHeader::setPartitionFsHash(const fnd::sha::sSha256Hash& hash)
{
	mPartitionFsHeaderHash = hash;
}

const fnd::sha::sSha256Hash& nn::hac::XciHeader::getInitialDataHash() const
{
	return mInitialDataHash;
}

void nn::hac::XciHeader::setInitialDataHash(const fnd::sha::sSha256Hash& hash)
{
	mInitialDataHash = hash;
}

uint32_t nn::hac::XciHeader::getSelSec() const
{
	return mSelSec;
}

void nn::hac::XciHeader::setSelSec(uint32_t sel_sec)
{
	mSelSec = sel_sec;
}

uint32_t nn::hac::XciHeader::getSelT1Key() const
{
	return mSelT1Key;
}

void nn::hac::XciHeader::setSelT1Key(uint32_t sel_t1_key)
{
	mSelT1Key = sel_t1_key;
}

uint32_t nn::hac::XciHeader::getSelKey() const
{
	return mSelKey;
}

void nn::hac::XciHeader::setSelKey(uint32_t sel_key)
{
	mSelKey = sel_key;
}

uint32_t nn::hac::XciHeader::getLimAreaPage() const
{
	return mLimAreaPage;
}

void nn::hac::XciHeader::setLimAreaPage(uint32_t page)
{
	mLimAreaPage = page;
}


uint32_t nn::hac::XciHeader::getFwVerMajor() const
{
	return mFwVersion[xci::FWVER_MAJOR];
}

void nn::hac::XciHeader::setFwVerMajor(uint32_t ver)
{
	mFwVersion[xci::FWVER_MAJOR] = ver;
}

uint32_t nn::hac::XciHeader::getFwVerMinor() const
{
	return mFwVersion[xci::FWVER_MINOR];
}

void nn::hac::XciHeader::setFwVerMinor(uint32_t ver)
{
	mFwVersion[xci::FWVER_MINOR] = ver;
}

uint32_t nn::hac::XciHeader::getAccCtrl1() const
{
	return mAccCtrl1;
}

void nn::hac::XciHeader::setAccCtrl1(uint32_t acc_ctrl_1)
{
	mAccCtrl1 = acc_ctrl_1;
}

uint32_t nn::hac::XciHeader::getWait1TimeRead() const
{
	return mWait1TimeRead;
}

void nn::hac::XciHeader::setWait1TimeRead(uint32_t seconds)
{
	mWait1TimeRead = seconds;
}

uint32_t nn::hac::XciHeader::getWait2TimeRead() const
{
	return mWait2TimeRead;
}

void nn::hac::XciHeader::setWait2TimeRead(uint32_t seconds)
{
	mWait2TimeRead = seconds;
}

uint32_t nn::hac::XciHeader::getWait1TimeWrite() const
{
	return mWait1TimeWrite;
}

void nn::hac::XciHeader::setWait1TimeWrite(uint32_t seconds)
{
	mWait1TimeWrite = seconds;
}

uint32_t nn::hac::XciHeader::getWait2TimeWrite() const
{
	return mWait2TimeWrite;
}

void nn::hac::XciHeader::setWait2TimeWrite(uint32_t seconds)
{
	mWait2TimeWrite = seconds;
}

uint32_t nn::hac::XciHeader::getFwMode() const
{
	return mFwMode;
}

void nn::hac::XciHeader::setFwMode(uint32_t fw_mode)
{
	mFwMode = fw_mode;
}

uint32_t nn::hac::XciHeader::getUppVersion() const
{
	return mUppVersion;
}

void nn::hac::XciHeader::setUppVersion(uint32_t version)
{
	mUppVersion = version;
}

const byte_t* nn::hac::XciHeader::getUppHash() const
{
	return mUppHash;
}

void nn::hac::XciHeader::setUppHash(const byte_t* hash)
{
	memcpy(mUppHash, hash, xci::kUppHashLen);
}

uint64_t nn::hac::XciHeader::getUppId() const
{
	return mUppId;
}

void nn::hac::XciHeader::setUppId(uint64_t id)
{
	mUppId = id;
}

