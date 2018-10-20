#include <nn/hac/ContentMetaInfo.h>

nn::hac::ContentMetaInfo::ContentMetaInfo()
{
	clear();
}

nn::hac::ContentMetaInfo::ContentMetaInfo(const ContentMetaInfo& other)
{
	*this = other;
}

void nn::hac::ContentMetaInfo::operator=(const ContentMetaInfo& other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mTitleId = other.mTitleId;
	mVersion = other.mVersion;
	mType = other.mType;
	mAttributes = other.mAttributes;
}

bool nn::hac::ContentMetaInfo::operator==(const ContentMetaInfo& other) const
{
	return (mTitleId == other.mTitleId) \
		&& (mVersion == other.mVersion) \
		&& (mType == other.mType) \
		&& (mAttributes == other.mAttributes);
}

bool nn::hac::ContentMetaInfo::operator!=(const ContentMetaInfo& other) const
{
	return !(*this == other);
}

void nn::hac::ContentMetaInfo::toBytes()
{
	mRawBinary.alloc(sizeof(sContentMetaInfo));
	sContentMetaInfo* info = (sContentMetaInfo*)mRawBinary.data();

	info->id = mTitleId;
	info->version = mVersion;
	info->type = mType;
	info->attributes = mAttributes;
}

void nn::hac::ContentMetaInfo::fromBytes(const byte_t* bytes, size_t len)
{
	if (len < sizeof(sContentMetaInfo))
	{
		throw fnd::Exception(kModuleName, "ContentMetaInfo too small");
	}

	const sContentMetaInfo* info = (const sContentMetaInfo*)bytes;

	mTitleId = info->id.get();
	mVersion = info->version.get();
	mType = (cnmt::ContentMetaType)info->type;
	mAttributes = info->attributes;
}

const fnd::Vec<byte_t>& nn::hac::ContentMetaInfo::getBytes() const
{
	return mRawBinary;
}

void nn::hac::ContentMetaInfo::clear()
{
	mRawBinary.clear();
	mTitleId = 0;
	mVersion = 0;
	mType = cnmt::ContentMetaType::METATYPE_APPLICATION;
	mAttributes = 0;
}

uint64_t nn::hac::ContentMetaInfo::getTitleId() const
{
	return mTitleId;
}

void nn::hac::ContentMetaInfo::setTitleId(uint64_t title_id)
{
	mTitleId = title_id;
}

uint32_t nn::hac::ContentMetaInfo::getVersion() const
{
	return mVersion;
}

void nn::hac::ContentMetaInfo::setVersion(uint32_t ver)
{
	mVersion = ver;
}	

nn::hac::cnmt::ContentMetaType nn::hac::ContentMetaInfo::getContentMetaType() const
{
	return mType;
}

void nn::hac::ContentMetaInfo::setContentMetaType(cnmt::ContentMetaType type)
{
	mType = type;
}	

byte_t nn::hac::ContentMetaInfo::getAttributes() const
{
	return mAttributes;
}

void nn::hac::ContentMetaInfo::setAttributes(byte_t attr)
{
	mAttributes = attr;
}
