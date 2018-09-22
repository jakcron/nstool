#include <nn/ctr/NcchHeader.h>

nn::ctr::NcchHeader::NcchHeader() :
	mRawBinary(),
	mNcchBinarySize(),
	mExHeaderLayout(),
	mPlainRegionLayout(),
	mLogoLayout(),
	mExefsLayout(),
	mRomfsLayout(),
	mCompanyCode(),
	mProductCode(),
	mSeedChecksum(),
	mNcchType(),
	mTitleId(),
	mProgramId(),
	mNewCryptoFlag(),
	mPlatform(),
	mFormType(),
	mContentType(),
	mBlockSize(),
	mOtherFlagList()
{
}

nn::ctr::NcchHeader::NcchHeader(const NcchHeader& other) :
	NcchHeader()
{
	*this = other;
}

void nn::ctr::NcchHeader::operator=(const NcchHeader& other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mNcchBinarySize = other.mNcchBinarySize;
	mExHeaderLayout = other.mExHeaderLayout;
	mPlainRegionLayout = other.mPlainRegionLayout;
	mLogoLayout = other.mLogoLayout;
	mExefsLayout = other.mExefsLayout;
	mRomfsLayout = other.mRomfsLayout;
	mCompanyCode = other.mCompanyCode;
	mProductCode = other.mProductCode;
	mSeedChecksum = other.mSeedChecksum;
	mNcchType = other.mNcchType;
	mTitleId = other.mTitleId;
	mProgramId = other.mProgramId;
	mNewCryptoFlag = other.mNewCryptoFlag;
	mPlatform = other.mPlatform;
	mFormType = other.mFormType;
	mContentType = other.mContentType;
	mBlockSize = other.mBlockSize;
	mOtherFlagList = other.mOtherFlagList;
}

bool nn::ctr::NcchHeader::operator==(const NcchHeader& other) const
{
	return (mRawBinary == other.mRawBinary) \
		&& (mNcchBinarySize == other.mNcchBinarySize) \
		&& (mExHeaderLayout == other.mExHeaderLayout) \
		&& (mPlainRegionLayout == other.mPlainRegionLayout) \
		&& (mLogoLayout == other.mLogoLayout) \
		&& (mExefsLayout == other.mExefsLayout) \
		&& (mRomfsLayout == other.mRomfsLayout) \
		&& (mCompanyCode == other.mCompanyCode) \
		&& (mProductCode == other.mProductCode) \
		&& (mSeedChecksum == other.mSeedChecksum) \
		&& (mNcchType == other.mNcchType) \
		&& (mTitleId == other.mTitleId) \
		&& (mProgramId == other.mProgramId) \
		&& (mNewCryptoFlag == other.mNewCryptoFlag) \
		&& (mPlatform == other.mPlatform) \
		&& (mFormType == other.mFormType) \
		&& (mContentType == other.mContentType) \
		&& (mBlockSize == other.mBlockSize) \
		&& (mOtherFlagList == other.mOtherFlagList);
}

bool nn::ctr::NcchHeader::operator!=(const NcchHeader& other) const
{
	return !(*this == other);
}

void nn::ctr::NcchHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sNcchHeader));
	nn::ctr::sNcchHeader* hdr = (nn::ctr::sNcchHeader*)mRawBinary.data();

	// set header identifers
	hdr->st_magic = ncch::kNcchStructMagic;
	hdr->ncch_type = mNcchType;

	// variable to store flag before commiting to header
	byte_t otherflag = 0;
	byte_t contentflag = 0;
	for (size_t i = 0; i < mOtherFlagList.size(); i++)
	{
		_SET_BIT(otherflag, mOtherFlagList[i]);
	}
	contentflag |= (mContentType & _BITMASK(ncch::kContentTypeBitWidth)) << ncch::kFormTypeBitWidth;
	contentflag |= (mFormType & _BITMASK(ncch::kFormTypeBitWidth));

	// set flags
	hdr->flags[ncch::CRYPTO_FLAG_INDEX] = mNewCryptoFlag;
	hdr->flags[ncch::PLATFORM_INDEX] = mPlatform;
	hdr->flags[ncch::CONTENT_TYPE_INDEX] = contentflag;
	hdr->flags[ncch::BLOCK_SIZE_INDEX] = mBlockSize;
	hdr->flags[ncch::OTHER_FLAG_INDEX] = otherflag;

	// set strings
	strncpy(hdr->product_code, mProductCode.c_str(), ncch::kProductCodeLen);
	strncpy(hdr->company_code, mCompanyCode.c_str(), ncch::kCompanyCodeLen);

	// misc
	hdr->title_id = mTitleId.getInnerValue();
	hdr->program_id = mProgramId.getInnerValue();
	hdr->seed_checksum = mSeedChecksum;

	// exheader
	hdr->exheader_size = mExHeaderLayout.getSize();
	hdr->exheader_hash = mExHeaderLayout.getHash();

	// block size
	size_t block_size = 0;
	if (mBlockSize == ncch::BLOCK_SIZE_512)
		block_size = 512;
	else
		throw fnd::Exception(kModuleName, "Illegal block size flag");

	// plain region
	if (mPlainRegionLayout.getOffset() % block_size || mPlainRegionLayout.getSize() % block_size)
		throw fnd::Exception(kModuleName, "Plain region geometry not aligned to block size");
	hdr->plain_region.block_offset = mPlainRegionLayout.getOffset() / block_size;
	hdr->plain_region.block_num = mPlainRegionLayout.getSize() / block_size;

	// logo
	if (mLogoLayout.getOffset() % block_size || mLogoLayout.getSize() % block_size)
		throw fnd::Exception(kModuleName, "Logo geometry not aligned to block size");
	hdr->logo.block_offset = mLogoLayout.getOffset() / block_size;
	hdr->logo.block_num = mLogoLayout.getSize() / block_size;
	hdr->logo_hash = mLogoLayout.getHash();

	// exefs
	if (mExefsLayout.getOffset() % block_size || mExefsLayout.getSize() % block_size || mExefsLayout.getHashedSize() % block_size)
		throw fnd::Exception(kModuleName, "ExeFs geometry not aligned to block size");
	hdr->exefs.block_offset = mExefsLayout.getOffset() / block_size;
	hdr->exefs.block_num = mExefsLayout.getSize() / block_size;
	hdr->exefs.block_num_hash_protected = mExefsLayout.getHashedSize() / block_size;
	hdr->exefs_hash = mExefsLayout.getHash();

	// romfs
	if (mRomfsLayout.getOffset() % block_size || mRomfsLayout.getSize() % block_size || mRomfsLayout.getHashedSize() % block_size)
		throw fnd::Exception(kModuleName, "RomFs geometry not aligned to block size");
	hdr->romfs.block_offset = mRomfsLayout.getOffset() / block_size;
	hdr->romfs.block_num = mRomfsLayout.getSize() / block_size;
	hdr->romfs.block_num_hash_protected = mRomfsLayout.getHashedSize() / block_size;
	hdr->romfs_hash = mRomfsLayout.getHash();
}

void nn::ctr::NcchHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input data size
	if (len < sizeof(sNcchHeader))
	{
		throw fnd::Exception(kModuleName, "NCCH header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sNcchHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// get ptr
	const nn::ctr::sNcchHeader* hdr = (const nn::ctr::sNcchHeader*)mRawBinary.data();
 
	// check NCCH signature
	if (hdr->st_magic.get() != ncch::kNcchStructMagic)
	{
		throw fnd::Exception(kModuleName, "NCCH header corrupt (unrecognised header signature)");
	}
	
	// check NCCH type
	if (hdr->ncch_type.get() != ncch::TYPE_CFA && hdr->ncch_type.get() != ncch::TYPE_CXI)
	{
		throw fnd::Exception(kModuleName, "NCCH header corrupt (unsupported NCCH type)");
	}

	// save flags
	mNewCryptoFlag = (ncch::NewCryptoFlag)hdr->flags[ncch::CRYPTO_FLAG_INDEX];
	mPlatform = (ncch::Platform)hdr->flags[ncch::PLATFORM_INDEX];
	mBlockSize = (ncch::BlockSizeFlag)hdr->flags[ncch::BLOCK_SIZE_INDEX];
	mContentType = (ncch::ContentType)((hdr->flags[ncch::CONTENT_TYPE_INDEX] >> ncch::kFormTypeBitWidth) & (byte_t)_BITMASK(ncch::kContentTypeBitWidth));
	mFormType = (ncch::FormType)(hdr->flags[ncch::CONTENT_TYPE_INDEX] & (byte_t)_BITMASK(ncch::kFormTypeBitWidth));
	for (size_t i = 0; i < 8; i++)
	{
		if (_HAS_BIT(hdr->flags[ncch::OTHER_FLAG_INDEX], i))
			mOtherFlagList.addElement((ncch::OtherFlag)i);
	}

	// save strings
	if (hdr->company_code[0] != '\0')
		mCompanyCode = std::string(hdr->company_code, _MIN(strlen(hdr->company_code), ncch::kCompanyCodeLen));
	if (hdr->product_code[0] != '\0')
		mCompanyCode = std::string(hdr->product_code, _MIN(strlen(hdr->product_code), ncch::kProductCodeLen));

	// save misc
	mTitleId = ProgramId(hdr->title_id.get());
	mProgramId = ProgramId(hdr->program_id.get());
	mSeedChecksum = hdr->seed_checksum.get();

	
	// block size
	size_t block_size = 0;
	if (mBlockSize == ncch::BLOCK_SIZE_512)
		block_size = 512;
	else
		throw fnd::Exception(kModuleName, "NCCH header corrupt (illegal block size flag)");

	// save layouts
	mNcchBinarySize = (size_t)hdr->size.get() * block_size;
	
	// exheader
	if (hdr->exheader_size.get() != 0)
	{
		mExHeaderLayout.setSize(hdr->exheader_size.get());
		mExHeaderLayout.setHash(hdr->exheader_hash);
	}
	
	// plain region
	if (hdr->plain_region.block_num.get() != 0)
	{
		mPlainRegionLayout.setOffset((size_t)hdr->plain_region.block_offset.get() * block_size);
		mPlainRegionLayout.setSize((size_t)hdr->plain_region.block_num.get() * block_size);
	}
	
	// logo region
	if (hdr->logo.block_num.get() != 0)
	{
		mLogoLayout.setOffset((size_t)hdr->logo.block_offset.get() * block_size);
		mLogoLayout.setSize((size_t)hdr->logo.block_num.get() * block_size);
		mLogoLayout.setHash(hdr->logo_hash);
	}
	

	// exefs region
	if (hdr->exefs.block_num.get() != 0)
	{
		mExefsLayout.setOffset((size_t)hdr->exefs.block_offset.get() * block_size);
		mExefsLayout.setSize((size_t)hdr->exefs.block_num.get() * block_size);
		mExefsLayout.setHashedSize((size_t)hdr->exefs.block_num_hash_protected.get() * block_size);
		mExefsLayout.setHash(hdr->exefs_hash);
	}

	// romfs region
	if (hdr->romfs.block_num.get() != 0)
	{
		mRomfsLayout.setOffset((size_t)hdr->romfs.block_offset.get() * block_size);
		mRomfsLayout.setSize((size_t)hdr->romfs.block_num.get() * block_size);
		mRomfsLayout.setHashedSize((size_t)hdr->romfs.block_num_hash_protected.get() * block_size);
		mRomfsLayout.setHash(hdr->romfs_hash);
	}
}

const fnd::Vec<byte_t>& nn::ctr::NcchHeader::getBytes() const
{
	return mRawBinary;
}

void nn::ctr::NcchHeader::clear()
{
	mRawBinary.clear();
	mNcchBinarySize = 0;
	mExHeaderLayout = Layout();
	mPlainRegionLayout = Layout();
	mLogoLayout = Layout();
	mExefsLayout = Layout();
	mRomfsLayout = Layout();
	mCompanyCode = std::string();
	mProductCode = std::string();
	mSeedChecksum = 0;
	mNcchType = ncch::NcchType::TYPE_CFA;
	mTitleId = ProgramId();
	mProgramId = ProgramId();
	mNewCryptoFlag = ncch::NewCryptoFlag::CRYPTO_DEFAULT;
	mPlatform = ncch::Platform::PLATFORM_CTR;
	mFormType = ncch::FormType::UNASSIGNED;
	mContentType = ncch::ContentType::APPLICATION;
	mBlockSize = ncch::BlockSizeFlag::BLOCK_SIZE_512;
	mOtherFlagList.clear();
}

void nn::ctr::NcchHeader::setNcchBinarySize(size_t size)
{
	mNcchBinarySize = size;
}

size_t nn::ctr::NcchHeader::getNcchBinarySize() const
{
	return mNcchBinarySize;
}

void nn::ctr::NcchHeader::setExtendedHeaderLayout(const Layout layout)
{
	mExHeaderLayout = layout;
}

const nn::ctr::NcchHeader::Layout& nn::ctr::NcchHeader::getExtendedHeaderLayout() const
{
	return mExHeaderLayout;
}

void nn::ctr::NcchHeader::setPlainRegionLayout(const Layout layout)
{
	mPlainRegionLayout = layout;
}

const nn::ctr::NcchHeader::Layout& nn::ctr::NcchHeader::getPlainRegionLayout() const
{
	return mPlainRegionLayout;
}

void nn::ctr::NcchHeader::setLogoLayout(const Layout layout)
{
	mLogoLayout = layout;
}

const nn::ctr::NcchHeader::Layout& nn::ctr::NcchHeader::getLogoLayout() const
{
	return mLogoLayout;
}

void nn::ctr::NcchHeader::setExefsLayout(const Layout layout)
{
	mExefsLayout = layout;
}

const nn::ctr::NcchHeader::Layout& nn::ctr::NcchHeader::getExefsLayout() const
{
	return mExefsLayout;
}

void nn::ctr::NcchHeader::setRomfsLayout(const Layout layout)
{
	mRomfsLayout = layout;
}

const nn::ctr::NcchHeader::Layout& nn::ctr::NcchHeader::getRomfsLayout() const
{
	return mRomfsLayout;
}

// property mutators
void nn::ctr::NcchHeader::setNcchType(ncch::NcchType type)
{
	mNcchType = type;
}

nn::ctr::ncch::NcchType nn::ctr::NcchHeader::getNcchType() const
{
	return mNcchType;
}

void nn::ctr::NcchHeader::setNewCryptoFlag(ncch::NewCryptoFlag flag)
{
	mNewCryptoFlag = flag;
}

nn::ctr::ncch::NewCryptoFlag nn::ctr::NcchHeader::getNewCryptoFlag() const
{
	return mNewCryptoFlag;
}

void nn::ctr::NcchHeader::setPlatform(ncch::Platform platform)
{
	mPlatform = platform;
}

nn::ctr::ncch::Platform nn::ctr::NcchHeader::getPlatform() const
{
	return mPlatform;
}

void nn::ctr::NcchHeader::setFormType(ncch::FormType type)
{
	mFormType = type;
}

nn::ctr::ncch::FormType nn::ctr::NcchHeader::getFormType() const
{
	return mFormType;
}

void nn::ctr::NcchHeader::setContentType(ncch::ContentType type)
{
	mContentType = type;
}

nn::ctr::ncch::ContentType nn::ctr::NcchHeader::getContentType() const
{
	return mContentType;
}

void nn::ctr::NcchHeader::setBlockSize(ncch::BlockSizeFlag size)
{
	mBlockSize = size;
}

nn::ctr::ncch::BlockSizeFlag nn::ctr::NcchHeader::getBlockSize() const
{
	return mBlockSize;
}

void nn::ctr::NcchHeader::setOtherFlagList(const fnd::List<ncch::OtherFlag>& list)
{
	mOtherFlagList = list;
}

const fnd::List<nn::ctr::ncch::OtherFlag>& nn::ctr::NcchHeader::getOtherFlagList() const
{
	return mOtherFlagList;
}

void nn::ctr::NcchHeader::setCompanyCode(const std::string& company_code)
{
	mCompanyCode = company_code;
}

const std::string& nn::ctr::NcchHeader::getCompanyCode() const
{
	return mCompanyCode;
}

void nn::ctr::NcchHeader::setTitleId(const ctr::ProgramId& id)
{
	mTitleId = id;
}

const nn::ctr::ProgramId& nn::ctr::NcchHeader::getTitleId() const
{
	return mTitleId;
}

void nn::ctr::NcchHeader::setProgramId(const ctr::ProgramId& id)
{
	mProgramId = id;
}

const nn::ctr::ProgramId& nn::ctr::NcchHeader::getProgramId() const
{
	return mProgramId;
}

void nn::ctr::NcchHeader::setProductCode(const std::string& product_code)
{
	mProductCode = product_code;
}

const std::string& nn::ctr::NcchHeader::getProductCode() const
{
	return mProductCode;
}

void nn::ctr::NcchHeader::setSeedChecksum(uint32_t checksum)
{
	mSeedChecksum = checksum;
}

uint32_t nn::ctr::NcchHeader::getSeedChecksum() const
{
	return mSeedChecksum;
}