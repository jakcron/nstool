#include <nx/XciHeader.h>

bool nx::XciHeader::isEqual(const XciHeader& other) const
{
	return	(  mRomAreaStartPage == other.mRomAreaStartPage \
			&& mBackupAreaStartPage == other.mBackupAreaStartPage \
			&& mKekIndex == other.mKekIndex \
			&& mTitleKeyDecIndex == other.mTitleKeyDecIndex \
			&& mRomSize == other.mRomSize \
			&& mCardHeaderVersion == other.mCardHeaderVersion \
			&& mFlags == other.mFlags \
			&& mPackageId == other.mPackageId \
			&& mValidDataEndPage == other.mValidDataEndPage \
			&& mAesCbcIv == other.mAesCbcIv \
			&& mPartitionFsHeaderAddress == other.mPartitionFsHeaderAddress \
			&& mPartitionFsHeaderSize == other.mPartitionFsHeaderSize \
			&& mPartitionFsHeaderHash == other.mPartitionFsHeaderHash \
			&& mInitialDataHash == other.mInitialDataHash \
			&& mSelSec == other.mSelSec \
			&& mSelT1Key == other.mSelT1Key \
			&& mSelKey == other.mSelKey \
			&& mLimAreaPage == other.mLimAreaPage \
			&& mFwVersion[0] == other.mFwVersion[0] \
			&& mFwVersion[1] == other.mFwVersion[1] \
			&& mAccCtrl1 == other.mAccCtrl1 \
			&& mWait1TimeRead == other.mWait1TimeRead \
			&& mWait2TimeRead == other.mWait2TimeRead \
			&& mWait1TimeWrite == other.mWait1TimeWrite \
			&& mWait2TimeWrite == other.mWait2TimeWrite \
			&& mFwMode == other.mFwMode \
			&& mUppVersion == other.mUppVersion \
			&& memcmp(mUppHash, other.mUppHash, xci::kUppHashLen) \
			&& mUppId == other.mUppId );
}

void nx::XciHeader::copyFrom(const XciHeader& other)
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

nx::XciHeader::XciHeader()
{

}

nx::XciHeader::XciHeader(const XciHeader& other)
{
	importBinary(other.getBytes(), other.getSize());
}
nx::XciHeader::XciHeader(const byte_t* bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::XciHeader::operator==(const XciHeader& other) const
{
	return isEqual(other);
}
bool nx::XciHeader::operator!=(const XciHeader& other) const
{
	return isEqual(other) ==  false;
}
void nx::XciHeader::operator=(const XciHeader& other)
{
	copyFrom(other);
}

// to be used after export
const byte_t* nx::XciHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::XciHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

// export/import binary
void nx::XciHeader::exportBinary()
{
	fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nx::XciHeader::importBinary(const byte_t* bytes, size_t len)
{
	// check input data size
	if (len < sizeof(sXciHeader))
	{
		throw fnd::Exception(kModuleName, "XCI header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mBinaryBlob.alloc(sizeof(sXciHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	// get sXciHeader ptr
	const nx::sXciHeader* hdr = (const nx::sXciHeader*)mBinaryBlob.getBytes();
	
	// check XCI signature
	if (std::string(hdr->signature, 4) != xci::kXciSig)
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
	for (size_t i = 0; i < crypto::aes::kAesBlockSize; i++)
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

// variables
void nx::XciHeader::clear()
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

uint32_t nx::XciHeader::getRomAreaStartPage() const
{
	return mRomAreaStartPage;
}

void nx::XciHeader::setRomAreaStartPage(uint32_t startPage)
{
	mRomAreaStartPage = startPage;
}

uint32_t nx::XciHeader::getBackupAreaStartPage() const
{
	return mBackupAreaStartPage;
}

void nx::XciHeader::setBackupAreaStartPage(uint32_t startPage)
{
	mBackupAreaStartPage = startPage;
}

byte_t nx::XciHeader::getKekIndex() const
{
	return mKekIndex;
}

void nx::XciHeader::setKekIndex(byte_t kekIndex)
{
	mKekIndex = kekIndex;
}

byte_t nx::XciHeader::getTitleKeyDecIndex() const
{
	return mTitleKeyDecIndex;
}

void nx::XciHeader::setTitleKeyDecIndex(byte_t index)
{
	mTitleKeyDecIndex = index;
}

byte_t nx::XciHeader::getRomSizeType() const
{
	return mRomSize;
}

void nx::XciHeader::setRomSizeType(byte_t romSizeType)
{
	mRomSize = romSizeType;
}

byte_t nx::XciHeader::getCardHeaderVersion() const
{
	return mCardHeaderVersion;
}

void nx::XciHeader::setCardHeaderVersion(byte_t version)
{
	mCardHeaderVersion = version;
}

byte_t nx::XciHeader::getFlags() const
{
	return mFlags;
}

void nx::XciHeader::setFlags(byte_t flags)
{
	mFlags = flags;
}

uint64_t nx::XciHeader::getPackageId() const
{
	return mPackageId;
}

void nx::XciHeader::setPackageId(uint64_t id)
{
	mPackageId = id;
}

uint32_t nx::XciHeader::getValidDataEndPage() const
{
	return mValidDataEndPage;
}

void nx::XciHeader::setValidDataEndPage(uint32_t page)
{
	mValidDataEndPage = page;
}

const crypto::aes::sAesIvCtr& nx::XciHeader::getAesCbcIv() const
{
	return mAesCbcIv;
}

void nx::XciHeader::setAesCbcIv(const crypto::aes::sAesIvCtr& iv)
{
	mAesCbcIv = iv;
}

uint64_t nx::XciHeader::getPartitionFsAddress() const
{
	return mPartitionFsHeaderAddress;
}

void nx::XciHeader::setPartitionFsAddress(uint64_t address)
{
	mPartitionFsHeaderAddress = address;
}

uint64_t nx::XciHeader::getPartitionFsSize() const
{
	return mPartitionFsHeaderSize;
}

void nx::XciHeader::setPartitionFsSize(uint64_t size)
{
	mPartitionFsHeaderSize = size;
}

const crypto::sha::sSha256Hash& nx::XciHeader::getPartitionFsHash() const
{
	return mPartitionFsHeaderHash;
}

void nx::XciHeader::setPartitionFsHash(const crypto::sha::sSha256Hash& hash)
{
	mPartitionFsHeaderHash = hash;
}

const crypto::sha::sSha256Hash& nx::XciHeader::getInitialDataHash() const
{
	return mInitialDataHash;
}

void nx::XciHeader::setInitialDataHash(const crypto::sha::sSha256Hash& hash)
{
	mInitialDataHash = hash;
}

uint32_t nx::XciHeader::getSelSec() const
{
	return mSelSec;
}

void nx::XciHeader::setSelSec(uint32_t sel_sec)
{
	mSelSec = sel_sec;
}

uint32_t nx::XciHeader::getSelT1Key() const
{
	return mSelT1Key;
}

void nx::XciHeader::setSelT1Key(uint32_t sel_t1_key)
{
	mSelT1Key = sel_t1_key;
}

uint32_t nx::XciHeader::getSelKey() const
{
	return mSelKey;
}

void nx::XciHeader::setSelKey(uint32_t sel_key)
{
	mSelKey = sel_key;
}

uint32_t nx::XciHeader::getLimAreaPage() const
{
	return mLimAreaPage;
}

void nx::XciHeader::setLimAreaPage(uint32_t page)
{
	mLimAreaPage = page;
}


uint32_t nx::XciHeader::getFwVerMajor() const
{
	return mFwVersion[xci::FWVER_MAJOR];
}

void nx::XciHeader::setFwVerMajor(uint32_t ver)
{
	mFwVersion[xci::FWVER_MAJOR] = ver;
}

uint32_t nx::XciHeader::getFwVerMinor() const
{
	return mFwVersion[xci::FWVER_MINOR];
}

void nx::XciHeader::setFwVerMinor(uint32_t ver)
{
	mFwVersion[xci::FWVER_MINOR] = ver;
}

uint32_t nx::XciHeader::getAccCtrl1() const
{
	return mAccCtrl1;
}

void nx::XciHeader::setAccCtrl1(uint32_t acc_ctrl_1)
{
	mAccCtrl1 = acc_ctrl_1;
}

uint32_t nx::XciHeader::getWait1TimeRead() const
{
	return mWait1TimeRead;
}

void nx::XciHeader::setWait1TimeRead(uint32_t seconds)
{
	mWait1TimeRead = seconds;
}

uint32_t nx::XciHeader::getWait2TimeRead() const
{
	return mWait2TimeRead;
}

void nx::XciHeader::setWait2TimeRead(uint32_t seconds)
{
	mWait2TimeRead = seconds;
}

uint32_t nx::XciHeader::getWait1TimeWrite() const
{
	return mWait1TimeWrite;
}

void nx::XciHeader::setWait1TimeWrite(uint32_t seconds)
{
	mWait1TimeWrite = seconds;
}

uint32_t nx::XciHeader::getWait2TimeWrite() const
{
	return mWait2TimeWrite;
}

void nx::XciHeader::setWait2TimeWrite(uint32_t seconds)
{
	mWait2TimeWrite = seconds;
}

uint32_t nx::XciHeader::getFwMode() const
{
	return mFwMode;
}

void nx::XciHeader::setFwMode(uint32_t fw_mode)
{
	mFwMode = fw_mode;
}

uint32_t nx::XciHeader::getUppVersion() const
{
	return mUppVersion;
}

void nx::XciHeader::setUppVersion(uint32_t version)
{
	mUppVersion = version;
}

const byte_t* nx::XciHeader::getUppHash() const
{
	return mUppHash;
}

void nx::XciHeader::setUppHash(const byte_t* hash)
{
	memcpy(mUppHash, hash, xci::kUppHashLen);
}

uint64_t nx::XciHeader::getUppId() const
{
	return mUppId;
}

void nx::XciHeader::setUppId(uint64_t id)
{
	mUppId = id;
}

