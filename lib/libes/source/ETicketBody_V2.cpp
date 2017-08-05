#include <es/ETicketBody_V2.h>



es::ETicketBody_V2::ETicketBody_V2()
{
	clear();
}

es::ETicketBody_V2::ETicketBody_V2(const ETicketBody_V2 & other)
{
	copyFrom(other);
}

es::ETicketBody_V2::ETicketBody_V2(const u8 * bytes, size_t len)
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

const u8 * es::ETicketBody_V2::getBytes() const
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

	body->set_format_version(kFormatVersion);

	body->set_issuer(mIssuer.c_str());
	body->set_enc_title_key(mEncTitleKey, kEncTitleKeyLen);
	body->set_title_key_enc_type(mEncType);
	body->set_ticket_version(mTicketVersion);
	u8 property_mask = 0;
	property_mask |= mPreInstall ? BIT(FLAG_PRE_INSTALL) : 0;
	property_mask |= mSharedTitle ? BIT(FLAG_SHARED_TITLE) : 0;
	property_mask |= mAllowAllContent ? BIT(FLAG_ALLOW_ALL_CONTENT) : 0;
	body->set_property_mask(property_mask);
	body->set_reserved_region(mReservedRegion, kReservedRegionLen);
	body->set_ticket_id(mTicketId);
	body->set_device_id(mDeviceId);
	body->set_rights_id(mRightsId);
	body->set_account_id(mAccountId);
	body->set_sect_total_size(mSectTotalSize);
	body->set_sect_header_offset(mSectHeaderOffset);
	body->set_sect_num(mSectNum);
	body->set_sect_entry_size(mSectEntrySize);
}

void es::ETicketBody_V2::importBinary(const u8 * bytes, size_t len)
{
	if (len < sizeof(sTicketBody_v2))
	{
		throw fnd::Exception(kModuleName, "Header size too small");
	}

	clear();

	mBinaryBlob.alloc(sizeof(sTicketBody_v2));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sTicketBody_v2* body = (sTicketBody_v2*)mBinaryBlob.getBytes();

	if (body->format_version() != kFormatVersion)
	{
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}

	mIssuer.append(body->issuer(), kIssuerLen);
	memcpy(mEncTitleKey, body->enc_title_key(), kEncTitleKeyLen);
	mEncType = (TitleKeyEncType)body->title_key_enc_type();
	mTicketVersion = body->ticket_version();
	mLicenseType = (LicenseType)body->license_type();
	mPreInstall = (body->property_mask() & BIT(FLAG_PRE_INSTALL)) == BIT(FLAG_PRE_INSTALL);
	mSharedTitle = (body->property_mask() & BIT(FLAG_SHARED_TITLE)) == BIT(FLAG_SHARED_TITLE);
	mAllowAllContent = (body->property_mask() & BIT(FLAG_ALLOW_ALL_CONTENT)) == BIT(FLAG_ALLOW_ALL_CONTENT);
	memcpy(mReservedRegion, body->reserved_region(), kReservedRegionLen);
	mTicketId = body->ticket_id();
	mDeviceId = body->device_id();
	memcpy(mRightsId, body->rights_id(), kRightsIdLen);
	mAccountId = body->account_id();
	mSectTotalSize = body->sect_total_size();
	mSectHeaderOffset = body->sect_header_offset();
	mSectNum = body->sect_num();
	mSectEntrySize = body->sect_entry_size();
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

const u8 * es::ETicketBody_V2::getEncTitleKey() const
{
	return mEncTitleKey;
}

void es::ETicketBody_V2::setEncTitleKey(const u8 * data, size_t len)
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

u16 es::ETicketBody_V2::getTicketVersion() const
{
	return mTicketVersion;
}

void es::ETicketBody_V2::setTicketVersion(u16 version)
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

u8 es::ETicketBody_V2::getCommonKeyId() const
{
	return mCommonKeyId;
}

void es::ETicketBody_V2::setCommonKeyId(u8 id)
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

const u8 * es::ETicketBody_V2::getReservedRegion() const
{
	return mReservedRegion;
}

void es::ETicketBody_V2::setReservedRegion(const u8 * data, size_t len)
{
	memset(mReservedRegion, 0, kReservedRegionLen);
	memcpy(mReservedRegion, data, MIN(len, kReservedRegionLen));
}

u64 es::ETicketBody_V2::getTicketId() const
{
	return mTicketId;
}

void es::ETicketBody_V2::setTicketId(u64 id)
{
	mTicketId = id;
}

u64 es::ETicketBody_V2::getDeviceId() const
{
	return mDeviceId;
}

void es::ETicketBody_V2::setDeviceId(u64 id)
{
	mDeviceId = id;
}

const u8 * es::ETicketBody_V2::getRightsId() const
{
	return mRightsId;
}

void es::ETicketBody_V2::setRightsId(const u8 * id)
{
	memcpy(mRightsId, id, kRightsIdLen);
}

u32 es::ETicketBody_V2::getAccountId() const
{
	return mAccountId;
}

void es::ETicketBody_V2::setAccountId(u32 id)
{
	mAccountId = id;
}

u32 es::ETicketBody_V2::getSectionTotalSize() const
{
	return mSectTotalSize;
}

void es::ETicketBody_V2::setSectionTotalSize(u32 size)
{
	mSectTotalSize = size;
}

u32 es::ETicketBody_V2::getSectionHeaderOffset() const
{
	return mSectHeaderOffset;
}

void es::ETicketBody_V2::setSectionHeaderOffset(u32 offset)
{
	mSectHeaderOffset = offset;
}

u16 es::ETicketBody_V2::getSectionNum() const
{
	return mSectNum;
}

void es::ETicketBody_V2::setSectionNum(u16 num)
{
	mSectNum = num;
}

u16 es::ETicketBody_V2::getSectionEntrySize() const
{
	return mSectEntrySize;
}

void es::ETicketBody_V2::setSectionEntrySize(u16 size)
{
	mSectEntrySize = size;
}
