#include <es/TicketBody_V2.h>



es::TicketBody_V2::TicketBody_V2()
{
	clear();
}

es::TicketBody_V2::TicketBody_V2(const TicketBody_V2 & other)
{
	*this = other;
}

void es::TicketBody_V2::operator=(const TicketBody_V2 & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mIssuer = other.mIssuer;
		memcpy(mEncTitleKey, other.mEncTitleKey, ticket::kEncTitleKeySize);
		mEncType = other.mEncType;
		mTicketVersion = other.mTicketVersion;
		mLicenseType = other.mLicenseType;
		mPreInstall = other.mPreInstall;
		mSharedTitle = other.mSharedTitle;
		mAllowAllContent = other.mAllowAllContent;
		memcpy(mReservedRegion, other.mReservedRegion, ticket::kReservedRegionSize);
		mTicketId = other.mTicketId;
		mDeviceId = other.mDeviceId;
		memcpy(mRightsId, other.mRightsId, ticket::kRightsIdSize);
		mAccountId = other.mAccountId;
		mSectTotalSize = other.mSectTotalSize;
		mSectHeaderOffset = other.mSectHeaderOffset;
		mSectNum = other.mSectNum;
		mSectEntrySize = other.mSectEntrySize;
	}
}

bool es::TicketBody_V2::operator==(const TicketBody_V2 & other) const
{
	return (mIssuer == other.mIssuer) \
		&& (memcmp(mEncTitleKey, other.mEncTitleKey, ticket::kEncTitleKeySize) == 0) \
		&& (mEncType == other.mEncType) \
		&& (mTicketVersion == other.mTicketVersion) \
		&& (mLicenseType == other.mLicenseType) \
		&& (mPreInstall == other.mPreInstall) \
		&& (mSharedTitle == other.mSharedTitle) \
		&& (mAllowAllContent == other.mAllowAllContent) \
		&& (memcmp(mReservedRegion, other.mReservedRegion, ticket::kReservedRegionSize) == 0) \
		&& (mTicketId == other.mTicketId) \
		&& (mDeviceId == other.mDeviceId) \
		&& (memcmp(mRightsId, other.mRightsId, ticket::kRightsIdSize) == 0) \
		&& (mAccountId == other.mAccountId) \
		&& (mSectTotalSize == other.mSectTotalSize) \
		&& (mSectHeaderOffset == other.mSectHeaderOffset) \
		&& (mSectNum == other.mSectNum) \
		&& (mSectEntrySize == other.mSectEntrySize);
}

bool es::TicketBody_V2::operator!=(const TicketBody_V2 & other) const
{
	return !(*this == other);
}

void es::TicketBody_V2::toBytes()
{
	mRawBinary.alloc(sizeof(sTicketBody_v2));
	sTicketBody_v2* body = (sTicketBody_v2*)mRawBinary.data();

	body->format_version = (ticket::kFormatVersion);

	strncmp(body->issuer, mIssuer.c_str(), ticket::kIssuerSize);
	memcpy(body->enc_title_key, mEncTitleKey, ticket::kEncTitleKeySize);
	body->title_key_enc_type = (mEncType);
	body->ticket_version = (mTicketVersion);
	byte_t property_mask = 0;
	property_mask |= mPreInstall ? _BIT(ticket::FLAG_PRE_INSTALL) : 0;
	property_mask |= mSharedTitle ? _BIT(ticket::FLAG_SHARED_TITLE) : 0;
	property_mask |= mAllowAllContent ? _BIT(ticket::FLAG_ALLOW_ALL_CONTENT) : 0;
	body->property_mask = (property_mask);
	memcpy(body->reserved_region, mReservedRegion, ticket::kReservedRegionSize);
	body->ticket_id = (mTicketId);
	body->device_id = (mDeviceId);
	memcpy(body->rights_id, mRightsId, ticket::kRightsIdSize);
	body->account_id = (mAccountId);
	body->sect_total_size = (mSectTotalSize);
	body->sect_header_offset = (mSectHeaderOffset);
	body->sect_num = (mSectNum);
	body->sect_entry_size = (mSectEntrySize);
}

void es::TicketBody_V2::fromBytes(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sTicketBody_v2))
	{
		throw fnd::Exception(kModuleName, "Header size too small");
	}

	clear();

	mRawBinary.alloc(sizeof(sTicketBody_v2));
	memcpy(mRawBinary.data(), bytes, mRawBinary.size());
	sTicketBody_v2* body = (sTicketBody_v2*)mRawBinary.data();

	if (body->format_version != ticket::kFormatVersion)
	{
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}

	mIssuer.append(body->issuer, ticket::kIssuerSize);
	memcpy(mEncTitleKey, body->enc_title_key, ticket::kEncTitleKeySize);
	mEncType = (ticket::TitleKeyEncType)body->title_key_enc_type;
	mTicketVersion = body->ticket_version.get();
	mLicenseType = (ticket::LicenseType)body->license_type;
	mPreInstall = _HAS_BIT(body->property_mask, ticket::FLAG_PRE_INSTALL);
	mSharedTitle = _HAS_BIT(body->property_mask, ticket::FLAG_SHARED_TITLE);
	mAllowAllContent = _HAS_BIT(body->property_mask, ticket::FLAG_ALLOW_ALL_CONTENT);
	memcpy(mReservedRegion, body->reserved_region, ticket::kReservedRegionSize);
	mTicketId = body->ticket_id.get();
	mDeviceId = body->device_id.get();
	memcpy(mRightsId, body->rights_id, ticket::kRightsIdSize);
	mAccountId = body->account_id.get();
	mSectTotalSize = body->sect_total_size.get();
	mSectHeaderOffset = body->sect_header_offset.get();
	mSectNum = body->sect_num.get();
	mSectEntrySize = body->sect_entry_size.get();
}

const fnd::Vec<byte_t>& es::TicketBody_V2::getBytes() const
{
	return mRawBinary;
}

void es::TicketBody_V2::clear()
{
	mRawBinary.clear();
	mIssuer.clear();
	memset(mEncTitleKey, 0, ticket::kEncTitleKeySize);
	mEncType = ticket::AES128_CBC;
	mTicketVersion = 0;
	mLicenseType = ticket::LICENSE_PERMANENT;
	mPreInstall = false;
	mSharedTitle = false;
	mAllowAllContent = false;
	memset(mReservedRegion, 0, ticket::kReservedRegionSize);
	mTicketId = 0;
	mDeviceId = 0;
	memset(mRightsId, 0, ticket::kRightsIdSize);
	mAccountId = 0;
	mSectTotalSize = 0;
	mSectHeaderOffset = 0;
	mSectNum = 0;
	mSectEntrySize = 0;
}

const std::string & es::TicketBody_V2::getIssuer() const
{
	return mIssuer;
}

void es::TicketBody_V2::setIssuer(const std::string & issuer)
{
	if (issuer.length() > ticket::kIssuerSize)
	{
		throw fnd::Exception(kModuleName, "Issuer is too long");
	}

	mIssuer = issuer;
}

const byte_t * es::TicketBody_V2::getEncTitleKey() const
{
	return mEncTitleKey;
}

void es::TicketBody_V2::setEncTitleKey(const byte_t * data, size_t len)
{
	memset(mEncTitleKey, 0, ticket::kEncTitleKeySize);
	memcpy(mEncTitleKey, data, _MIN(len, ticket::kEncTitleKeySize));
}

es::ticket::TitleKeyEncType es::TicketBody_V2::getTitleKeyEncType() const
{
	return mEncType;
}

void es::TicketBody_V2::setTitleKeyEncType(ticket::TitleKeyEncType type)
{
	mEncType = type;
}

uint16_t es::TicketBody_V2::getTicketVersion() const
{
	return mTicketVersion;
}

void es::TicketBody_V2::setTicketVersion(uint16_t version)
{
	mTicketVersion = version;
}

es::ticket::LicenseType es::TicketBody_V2::getLicenseType() const
{
	return mLicenseType;
}

void es::TicketBody_V2::setLicenseType(ticket::LicenseType type)
{
	mLicenseType = type;
}

byte_t es::TicketBody_V2::getCommonKeyId() const
{
	return mCommonKeyId;
}

void es::TicketBody_V2::setCommonKeyId(byte_t id)
{
	mCommonKeyId = id;
}

bool es::TicketBody_V2::isPreInstall() const
{
	return mPreInstall;
}

void es::TicketBody_V2::setIsPreInstall(bool isPreInstall)
{
	mPreInstall = isPreInstall;
}

bool es::TicketBody_V2::isSharedTitle() const
{
	return mSharedTitle;
}

void es::TicketBody_V2::setIsSharedTitle(bool isSharedTitle)
{
	mSharedTitle = isSharedTitle;
}

bool es::TicketBody_V2::allowAllContent() const
{
	return mAllowAllContent;
}

void es::TicketBody_V2::setAllowAllContent(bool allowAllContent)
{
	mAllowAllContent = allowAllContent;
}

const byte_t * es::TicketBody_V2::getReservedRegion() const
{
	return mReservedRegion;
}

void es::TicketBody_V2::setReservedRegion(const byte_t * data, size_t len)
{
	memset(mReservedRegion, 0, ticket::kReservedRegionSize);
	memcpy(mReservedRegion, data, _MIN(len, ticket::kReservedRegionSize));
}

uint64_t es::TicketBody_V2::getTicketId() const
{
	return mTicketId;
}

void es::TicketBody_V2::setTicketId(uint64_t id)
{
	mTicketId = id;
}

uint64_t es::TicketBody_V2::getDeviceId() const
{
	return mDeviceId;
}

void es::TicketBody_V2::setDeviceId(uint64_t id)
{
	mDeviceId = id;
}

const byte_t * es::TicketBody_V2::getRightsId() const
{
	return mRightsId;
}

void es::TicketBody_V2::setRightsId(const byte_t * id)
{
	memcpy(mRightsId, id, ticket::kRightsIdSize);
}

uint32_t es::TicketBody_V2::getAccountId() const
{
	return mAccountId;
}

void es::TicketBody_V2::setAccountId(uint32_t id)
{
	mAccountId = id;
}

uint32_t es::TicketBody_V2::getSectionTotalSize() const
{
	return mSectTotalSize;
}

void es::TicketBody_V2::setSectionTotalSize(uint32_t size)
{
	mSectTotalSize = size;
}

uint32_t es::TicketBody_V2::getSectionHeaderOffset() const
{
	return mSectHeaderOffset;
}

void es::TicketBody_V2::setSectionHeaderOffset(uint32_t offset)
{
	mSectHeaderOffset = offset;
}

uint16_t es::TicketBody_V2::getSectionNum() const
{
	return mSectNum;
}

void es::TicketBody_V2::setSectionNum(uint16_t num)
{
	mSectNum = num;
}

uint16_t es::TicketBody_V2::getSectionEntrySize() const
{
	return mSectEntrySize;
}

void es::TicketBody_V2::setSectionEntrySize(uint16_t size)
{
	mSectEntrySize = size;
}