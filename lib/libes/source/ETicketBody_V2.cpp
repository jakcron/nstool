#include <es/ETicketBody_V2.h>



es::ETicketBody_V2::ETicketBody_V2()
{
	clear();
}

es::ETicketBody_V2::ETicketBody_V2(const ETicketBody_V2 & other)
{
	copyFrom(other);
}

es::ETicketBody_V2::ETicketBody_V2(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool es::ETicketBody_V2::operator==(const ETicketBody_V2 & other) const
{
	return isEqual(other);
}

bool es::ETicketBody_V2::operator!=(const ETicketBody_V2 & other) const
{
	return !isEqual(other);
}

void es::ETicketBody_V2::operator=(const ETicketBody_V2 & other)
{
	copyFrom(other);
}

const byte_t * es::ETicketBody_V2::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t es::ETicketBody_V2::getSize() const
{
	return mBinaryBlob.getSize();
}

bool es::ETicketBody_V2::isEqual(const ETicketBody_V2 & other) const
{
	return (mIssuer == other.mIssuer) \
		&& (memcmp(mEncTitleKey, other.mEncTitleKey, kEncTitleKeyLen) == 0) \
		&& (mEncType == other.mEncType) \
		&& (mTicketVersion == other.mTicketVersion) \
		&& (mLicenseType == other.mLicenseType) \
		&& (mPreInstall == other.mPreInstall) \
		&& (mSharedTitle == other.mSharedTitle) \
		&& (mAllowAllContent == other.mAllowAllContent) \
		&& (memcmp(mReservedRegion, other.mReservedRegion, kReservedRegionLen) == 0) \
		&& (mTicketId == other.mTicketId) \
		&& (mDeviceId == other.mDeviceId) \
		&& (memcmp(mRightsId, other.mRightsId, kRightsIdLen) == 0) \
		&& (mAccountId == other.mAccountId) \
		&& (mSectTotalSize == other.mSectTotalSize) \
		&& (mSectHeaderOffset == other.mSectHeaderOffset) \
		&& (mSectNum == other.mSectNum) \
		&& (mSectEntrySize == other.mSectEntrySize);
}

void es::ETicketBody_V2::copyFrom(const ETicketBody_V2 & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		clear();
		mIssuer = other.mIssuer;
		memcpy(mEncTitleKey, other.mEncTitleKey, kEncTitleKeyLen);
		mEncType = other.mEncType;
		mTicketVersion = other.mTicketVersion;
		mLicenseType = other.mLicenseType;
		mPreInstall = other.mPreInstall;
		mSharedTitle = other.mSharedTitle;
		mAllowAllContent = other.mAllowAllContent;
		memcpy(mReservedRegion, other.mReservedRegion, kReservedRegionLen);
		mTicketId = other.mTicketId;
		mDeviceId = other.mDeviceId;
		memcpy(mRightsId, other.mRightsId, kRightsIdLen);
		mAccountId = other.mAccountId;
		mSectTotalSize = other.mSectTotalSize;
		mSectHeaderOffset = other.mSectHeaderOffset;
		mSectNum = other.mSectNum;
		mSectEntrySize = other.mSectEntrySize;
	}
}

void es::ETicketBody_V2::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sTicketBody_v2));
	sTicketBody_v2* body = (sTicketBody_v2*)mBinaryBlob.getBytes();

	body->format_version = (kFormatVersion);

	strncmp(body->issuer, mIssuer.c_str(), kIssuerLen);
	memcpy(body->enc_title_key, mEncTitleKey, kEncTitleKeyLen);
	body->title_key_enc_type = (mEncType);
	body->ticket_version = (mTicketVersion);
	byte_t property_mask = 0;
	property_mask |= mPreInstall ? BIT(FLAG_PRE_INSTALL) : 0;
	property_mask |= mSharedTitle ? BIT(FLAG_SHARED_TITLE) : 0;
	property_mask |= mAllowAllContent ? BIT(FLAG_ALLOW_ALL_CONTENT) : 0;
	body->property_mask = (property_mask);
	memcpy(body->reserved_region, mReservedRegion, kReservedRegionLen);
	body->ticket_id = (mTicketId);
	body->device_id = (mDeviceId);
	memcmp(body->rights_id, mRightsId, kRightsIdLen);
	body->account_id = (mAccountId);
	body->sect_total_size = (mSectTotalSize);
	body->sect_header_offset = (mSectHeaderOffset);
	body->sect_num = (mSectNum);
	body->sect_entry_size = (mSectEntrySize);
}

void es::ETicketBody_V2::importBinary(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sTicketBody_v2))
	{
		throw fnd::Exception(kModuleName, "Header size too small");
	}

	clear();

	mBinaryBlob.alloc(sizeof(sTicketBody_v2));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sTicketBody_v2* body = (sTicketBody_v2*)mBinaryBlob.getBytes();

	if (body->format_version != kFormatVersion)
	{
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}

	mIssuer.append(body->issuer, kIssuerLen);
	memcpy(mEncTitleKey, body->enc_title_key, kEncTitleKeyLen);
	mEncType = (TitleKeyEncType)body->title_key_enc_type;
	mTicketVersion = body->ticket_version.get();
	mLicenseType = (LicenseType)body->license_type;
	mPreInstall = (body->property_mask & BIT(FLAG_PRE_INSTALL)) == BIT(FLAG_PRE_INSTALL);
	mSharedTitle = (body->property_mask & BIT(FLAG_SHARED_TITLE)) == BIT(FLAG_SHARED_TITLE);
	mAllowAllContent = (body->property_mask & BIT(FLAG_ALLOW_ALL_CONTENT)) == BIT(FLAG_ALLOW_ALL_CONTENT);
	memcpy(mReservedRegion, body->reserved_region, kReservedRegionLen);
	mTicketId = body->ticket_id.get();
	mDeviceId = body->device_id.get();
	memcpy(mRightsId, body->rights_id, kRightsIdLen);
	mAccountId = body->account_id.get();
	mSectTotalSize = body->sect_total_size.get();
	mSectHeaderOffset = body->sect_header_offset.get();
	mSectNum = body->sect_num.get();
	mSectEntrySize = body->sect_entry_size.get();
}

void es::ETicketBody_V2::clear()
{
	mBinaryBlob.clear();
	mIssuer.clear();
	memset(mEncTitleKey, 0, kEncTitleKeyLen);
	mEncType = AES128_CBC;
	mTicketVersion = 0;
	mLicenseType = ES_LICENSE_PERMANENT;
	mPreInstall = false;
	mSharedTitle = false;
	mAllowAllContent = false;
	memset(mReservedRegion, 0, kReservedRegionLen);
	mTicketId = 0;
	mDeviceId = 0;
	memset(mRightsId, 0, kRightsIdLen);
	mAccountId = 0;
	mSectTotalSize = 0;
	mSectHeaderOffset = 0;
	mSectNum = 0;
	mSectEntrySize = 0;
}

const std::string & es::ETicketBody_V2::getIssuer() const
{
	return mIssuer;
}

void es::ETicketBody_V2::setIssuer(const std::string & issuer)
{
	if (issuer.length() > kIssuerLen)
	{
		throw fnd::Exception(kModuleName, "Issuer is too long");
	}

	mIssuer = issuer;
}

const byte_t * es::ETicketBody_V2::getEncTitleKey() const
{
	return mEncTitleKey;
}

void es::ETicketBody_V2::setEncTitleKey(const byte_t * data, size_t len)
{
	memset(mEncTitleKey, 0, kEncTitleKeyLen);
	memcpy(mEncTitleKey, data, MIN(len, kEncTitleKeyLen));
}

es::ETicketBody_V2::TitleKeyEncType es::ETicketBody_V2::getTitleKeyEncType() const
{
	return mEncType;
}

void es::ETicketBody_V2::setTitleKeyEncType(TitleKeyEncType type)
{
	mEncType = type;
}

uint16_t es::ETicketBody_V2::getTicketVersion() const
{
	return mTicketVersion;
}

void es::ETicketBody_V2::setTicketVersion(uint16_t version)
{
	mTicketVersion = version;
}

es::ETicketBody_V2::LicenseType es::ETicketBody_V2::getLicenseType() const
{
	return mLicenseType;
}

void es::ETicketBody_V2::setLicenseType(LicenseType type)
{
	mLicenseType = type;
}

byte_t es::ETicketBody_V2::getCommonKeyId() const
{
	return mCommonKeyId;
}

void es::ETicketBody_V2::setCommonKeyId(byte_t id)
{
	mCommonKeyId = id;
}

bool es::ETicketBody_V2::isPreInstall() const
{
	return mPreInstall;
}

void es::ETicketBody_V2::setIsPreInstall(bool isPreInstall)
{
	mPreInstall = isPreInstall;
}

bool es::ETicketBody_V2::isSharedTitle() const
{
	return mSharedTitle;
}

void es::ETicketBody_V2::setIsSharedTitle(bool isSharedTitle)
{
	mSharedTitle = isSharedTitle;
}

bool es::ETicketBody_V2::allowAllContent() const
{
	return mAllowAllContent;
}

void es::ETicketBody_V2::setAllowAllContent(bool allowAllContent)
{
	mAllowAllContent = allowAllContent;
}

const byte_t * es::ETicketBody_V2::getReservedRegion() const
{
	return mReservedRegion;
}

void es::ETicketBody_V2::setReservedRegion(const byte_t * data, size_t len)
{
	memset(mReservedRegion, 0, kReservedRegionLen);
	memcpy(mReservedRegion, data, MIN(len, kReservedRegionLen));
}

uint64_t es::ETicketBody_V2::getTicketId() const
{
	return mTicketId;
}

void es::ETicketBody_V2::setTicketId(uint64_t id)
{
	mTicketId = id;
}

uint64_t es::ETicketBody_V2::getDeviceId() const
{
	return mDeviceId;
}

void es::ETicketBody_V2::setDeviceId(uint64_t id)
{
	mDeviceId = id;
}

const byte_t * es::ETicketBody_V2::getRightsId() const
{
	return mRightsId;
}

void es::ETicketBody_V2::setRightsId(const byte_t * id)
{
	memcpy(mRightsId, id, kRightsIdLen);
}

uint32_t es::ETicketBody_V2::getAccountId() const
{
	return mAccountId;
}

void es::ETicketBody_V2::setAccountId(uint32_t id)
{
	mAccountId = id;
}

uint32_t es::ETicketBody_V2::getSectionTotalSize() const
{
	return mSectTotalSize;
}

void es::ETicketBody_V2::setSectionTotalSize(uint32_t size)
{
	mSectTotalSize = size;
}

uint32_t es::ETicketBody_V2::getSectionHeaderOffset() const
{
	return mSectHeaderOffset;
}

void es::ETicketBody_V2::setSectionHeaderOffset(uint32_t offset)
{
	mSectHeaderOffset = offset;
}

uint16_t es::ETicketBody_V2::getSectionNum() const
{
	return mSectNum;
}

void es::ETicketBody_V2::setSectionNum(uint16_t num)
{
	mSectNum = num;
}

uint16_t es::ETicketBody_V2::getSectionEntrySize() const
{
	return mSectEntrySize;
}

void es::ETicketBody_V2::setSectionEntrySize(uint16_t size)
{
	mSectEntrySize = size;
}
