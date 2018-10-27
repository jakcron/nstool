#include <nn/hac/GameCardHeader.h>

nn::hac::GameCardHeader::GameCardHeader()
{
	clear();
}

nn::hac::GameCardHeader::GameCardHeader(const GameCardHeader& other)
{
	*this = other;
}

void nn::hac::GameCardHeader::operator=(const GameCardHeader& other)
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
	memcpy(mUppHash, other.mUppHash, gc::kUppHashLen);
	mUppId = other.mUppId;
}

bool nn::hac::GameCardHeader::operator==(const GameCardHeader& other) const
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
		&&	(memcmp(mUppHash, other.mUppHash, gc::kUppHashLen) == 0)
		&&	(mUppId == other.mUppId);
}

bool nn::hac::GameCardHeader::operator!=(const GameCardHeader& other) const
{
	return !(*this == other);
}

void nn::hac::GameCardHeader::toBytes()
{
	fnd::Exception(kModuleName, "toBytes() not implemented");
}

void nn::hac::GameCardHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input data size
	if (len < sizeof(sGcHeader))
	{
		throw fnd::Exception(kModuleName, "GameCardImage header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sGcHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// get sGcHeader ptr
	const nn::hac::sGcHeader* hdr = (const nn::hac::sGcHeader*)mRawBinary.data();
	
	// check GameCardImage signature
	if (hdr->st_magic.get() != gc::kGcHeaderStructMagic)
	{
		throw fnd::Exception(kModuleName, "GameCardImage header corrupt");
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
		mFwVersion[gc::FWVER_MAJOR] = hdr->fw_version[gc::FWVER_MAJOR].get();
		mFwVersion[gc::FWVER_MINOR] = hdr->fw_version[gc::FWVER_MINOR].get();
		mAccCtrl1 = hdr->acc_ctrl_1.get();
		mWait1TimeRead = hdr->wait_1_time_read.get();
		mWait2TimeRead = hdr->wait_2_time_read.get();
		mWait1TimeWrite = hdr->wait_1_time_write.get();
		mWait2TimeWrite = hdr->wait_2_time_write.get();
		mFwMode = hdr->fw_mode.get();
		mUppVersion = hdr->upp_version.get();
		memcpy(mUppHash, hdr->upp_hash, gc::kUppHashLen);
		mUppId = hdr->upp_id.get();
	}

}

const fnd::Vec<byte_t>& nn::hac::GameCardHeader::getBytes() const
{
	return mRawBinary;
}

// variables
void nn::hac::GameCardHeader::clear()
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
	memset(mUppHash, 0, gc::kUppHashLen);
	mUppId = 0;
}

uint32_t nn::hac::GameCardHeader::getRomAreaStartPage() const
{
	return mRomAreaStartPage;
}

void nn::hac::GameCardHeader::setRomAreaStartPage(uint32_t startPage)
{
	mRomAreaStartPage = startPage;
}

uint32_t nn::hac::GameCardHeader::getBackupAreaStartPage() const
{
	return mBackupAreaStartPage;
}

void nn::hac::GameCardHeader::setBackupAreaStartPage(uint32_t startPage)
{
	mBackupAreaStartPage = startPage;
}

byte_t nn::hac::GameCardHeader::getKekIndex() const
{
	return mKekIndex;
}

void nn::hac::GameCardHeader::setKekIndex(byte_t kekIndex)
{
	mKekIndex = kekIndex;
}

byte_t nn::hac::GameCardHeader::getTitleKeyDecIndex() const
{
	return mTitleKeyDecIndex;
}

void nn::hac::GameCardHeader::setTitleKeyDecIndex(byte_t index)
{
	mTitleKeyDecIndex = index;
}

byte_t nn::hac::GameCardHeader::getRomSizeType() const
{
	return mRomSize;
}

void nn::hac::GameCardHeader::setRomSizeType(byte_t romSizeType)
{
	mRomSize = romSizeType;
}

byte_t nn::hac::GameCardHeader::getCardHeaderVersion() const
{
	return mCardHeaderVersion;
}

void nn::hac::GameCardHeader::setCardHeaderVersion(byte_t version)
{
	mCardHeaderVersion = version;
}

byte_t nn::hac::GameCardHeader::getFlags() const
{
	return mFlags;
}

void nn::hac::GameCardHeader::setFlags(byte_t flags)
{
	mFlags = flags;
}

uint64_t nn::hac::GameCardHeader::getPackageId() const
{
	return mPackageId;
}

void nn::hac::GameCardHeader::setPackageId(uint64_t id)
{
	mPackageId = id;
}

uint32_t nn::hac::GameCardHeader::getValidDataEndPage() const
{
	return mValidDataEndPage;
}

void nn::hac::GameCardHeader::setValidDataEndPage(uint32_t page)
{
	mValidDataEndPage = page;
}

const fnd::aes::sAesIvCtr& nn::hac::GameCardHeader::getAesCbcIv() const
{
	return mAesCbcIv;
}

void nn::hac::GameCardHeader::setAesCbcIv(const fnd::aes::sAesIvCtr& iv)
{
	mAesCbcIv = iv;
}

uint64_t nn::hac::GameCardHeader::getPartitionFsAddress() const
{
	return mPartitionFsHeaderAddress;
}

void nn::hac::GameCardHeader::setPartitionFsAddress(uint64_t address)
{
	mPartitionFsHeaderAddress = address;
}

uint64_t nn::hac::GameCardHeader::getPartitionFsSize() const
{
	return mPartitionFsHeaderSize;
}

void nn::hac::GameCardHeader::setPartitionFsSize(uint64_t size)
{
	mPartitionFsHeaderSize = size;
}

const fnd::sha::sSha256Hash& nn::hac::GameCardHeader::getPartitionFsHash() const
{
	return mPartitionFsHeaderHash;
}

void nn::hac::GameCardHeader::setPartitionFsHash(const fnd::sha::sSha256Hash& hash)
{
	mPartitionFsHeaderHash = hash;
}

const fnd::sha::sSha256Hash& nn::hac::GameCardHeader::getInitialDataHash() const
{
	return mInitialDataHash;
}

void nn::hac::GameCardHeader::setInitialDataHash(const fnd::sha::sSha256Hash& hash)
{
	mInitialDataHash = hash;
}

uint32_t nn::hac::GameCardHeader::getSelSec() const
{
	return mSelSec;
}

void nn::hac::GameCardHeader::setSelSec(uint32_t sel_sec)
{
	mSelSec = sel_sec;
}

uint32_t nn::hac::GameCardHeader::getSelT1Key() const
{
	return mSelT1Key;
}

void nn::hac::GameCardHeader::setSelT1Key(uint32_t sel_t1_key)
{
	mSelT1Key = sel_t1_key;
}

uint32_t nn::hac::GameCardHeader::getSelKey() const
{
	return mSelKey;
}

void nn::hac::GameCardHeader::setSelKey(uint32_t sel_key)
{
	mSelKey = sel_key;
}

uint32_t nn::hac::GameCardHeader::getLimAreaPage() const
{
	return mLimAreaPage;
}

void nn::hac::GameCardHeader::setLimAreaPage(uint32_t page)
{
	mLimAreaPage = page;
}


uint32_t nn::hac::GameCardHeader::getFwVerMajor() const
{
	return mFwVersion[gc::FWVER_MAJOR];
}

void nn::hac::GameCardHeader::setFwVerMajor(uint32_t ver)
{
	mFwVersion[gc::FWVER_MAJOR] = ver;
}

uint32_t nn::hac::GameCardHeader::getFwVerMinor() const
{
	return mFwVersion[gc::FWVER_MINOR];
}

void nn::hac::GameCardHeader::setFwVerMinor(uint32_t ver)
{
	mFwVersion[gc::FWVER_MINOR] = ver;
}

uint32_t nn::hac::GameCardHeader::getAccCtrl1() const
{
	return mAccCtrl1;
}

void nn::hac::GameCardHeader::setAccCtrl1(uint32_t acc_ctrl_1)
{
	mAccCtrl1 = acc_ctrl_1;
}

uint32_t nn::hac::GameCardHeader::getWait1TimeRead() const
{
	return mWait1TimeRead;
}

void nn::hac::GameCardHeader::setWait1TimeRead(uint32_t seconds)
{
	mWait1TimeRead = seconds;
}

uint32_t nn::hac::GameCardHeader::getWait2TimeRead() const
{
	return mWait2TimeRead;
}

void nn::hac::GameCardHeader::setWait2TimeRead(uint32_t seconds)
{
	mWait2TimeRead = seconds;
}

uint32_t nn::hac::GameCardHeader::getWait1TimeWrite() const
{
	return mWait1TimeWrite;
}

void nn::hac::GameCardHeader::setWait1TimeWrite(uint32_t seconds)
{
	mWait1TimeWrite = seconds;
}

uint32_t nn::hac::GameCardHeader::getWait2TimeWrite() const
{
	return mWait2TimeWrite;
}

void nn::hac::GameCardHeader::setWait2TimeWrite(uint32_t seconds)
{
	mWait2TimeWrite = seconds;
}

uint32_t nn::hac::GameCardHeader::getFwMode() const
{
	return mFwMode;
}

void nn::hac::GameCardHeader::setFwMode(uint32_t fw_mode)
{
	mFwMode = fw_mode;
}

uint32_t nn::hac::GameCardHeader::getUppVersion() const
{
	return mUppVersion;
}

void nn::hac::GameCardHeader::setUppVersion(uint32_t version)
{
	mUppVersion = version;
}

const byte_t* nn::hac::GameCardHeader::getUppHash() const
{
	return mUppHash;
}

void nn::hac::GameCardHeader::setUppHash(const byte_t* hash)
{
	memcpy(mUppHash, hash, gc::kUppHashLen);
}

uint64_t nn::hac::GameCardHeader::getUppId() const
{
	return mUppId;
}

void nn::hac::GameCardHeader::setUppId(uint64_t id)
{
	mUppId = id;
}

