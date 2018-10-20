#include <nn/hac/ContentInfo.h>

nn::hac::ContentInfo::ContentInfo()
{
	clear();
}

nn::hac::ContentInfo::ContentInfo(const ContentInfo& other)
{
	*this = other;
}

void nn::hac::ContentInfo::operator=(const ContentInfo& other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mHash = other.mHash;
	mContentId = other.mContentId;
	mSize = other.mSize;
	mType = other.mType;
}

bool nn::hac::ContentInfo::operator==(const ContentInfo& other) const
{
	return (mHash == other.mHash) \
		&& (mContentId == other.mContentId) \
		&& (mSize == other.mSize) \
		&& (mType == other.mType);
}

bool nn::hac::ContentInfo::operator!=(const ContentInfo& other) const
{
	return !(*this == other);
}

void nn::hac::ContentInfo::toBytes()
{
	mRawBinary.alloc(sizeof(sContentInfo));
	sContentInfo* info = (sContentInfo*)mRawBinary.data();

	info->content_hash = mHash;
	info->content_id = mContentId;
	info->size_lower = mSize & (uint32_t)(-1);
	info->size_higher = (mSize >> 32) & (uint16_t)(-1);
	info->content_type = mType;
	info->id_offset = mIdOffset;
}

void nn::hac::ContentInfo::fromBytes(const byte_t* bytes, size_t len)
{
	if (len < sizeof(sContentInfo))
	{
		throw fnd::Exception(kModuleName, "ContentInfo too small");
	}

	const sContentInfo* info = (const sContentInfo*)bytes;

	mHash = info->content_hash;
	mContentId = info->content_id;
	mSize = (uint64_t)(info->size_lower.get()) | (uint64_t)(info->size_higher.get()) << 32;
	mType = (cnmt::ContentType)info->content_type;
	mIdOffset = info->id_offset;
}

const fnd::Vec<byte_t>& nn::hac::ContentInfo::getBytes() const
{
	return mRawBinary;
}

void nn::hac::ContentInfo::clear()
{
	mRawBinary.clear();
}

const fnd::sha::sSha256Hash& nn::hac::ContentInfo::getContentHash() const
{
	return mHash;
}

void nn::hac::ContentInfo::setContentHash(const fnd::sha::sSha256Hash& hash)
{
	mHash = hash;
}

const nn::hac::cnmt::sContentId& nn::hac::ContentInfo::getContentId() const
{
	return mContentId;
}

void nn::hac::ContentInfo::setContentId(const cnmt::sContentId& content_id)
{
	mContentId = content_id;
}

size_t nn::hac::ContentInfo::getContentSize() const
{
	return mSize;
}

void nn::hac::ContentInfo::setContentSize(size_t size)
{
	mSize = size;
}

nn::hac::cnmt::ContentType nn::hac::ContentInfo::getContentType() const
{
	return mType;
}

void nn::hac::ContentInfo::setContentType(cnmt::ContentType type)
{
	mType = type;
}

byte_t nn::hac::ContentInfo::getIdOffset() const
{
	return mIdOffset;
}

void nn::hac::ContentInfo::setIdOffset(byte_t id_offset)
{
	mIdOffset = id_offset;
}