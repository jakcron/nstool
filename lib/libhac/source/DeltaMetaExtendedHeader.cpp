#include <nn/hac/DeltaMetaExtendedHeader.h>

nn::hac::DeltaMetaExtendedHeader::DeltaMetaExtendedHeader()
{
	clear();
}

nn::hac::DeltaMetaExtendedHeader::DeltaMetaExtendedHeader(const DeltaMetaExtendedHeader& other)
{
	*this = other;
}

void nn::hac::DeltaMetaExtendedHeader::operator=(const DeltaMetaExtendedHeader& other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mApplicationId = other.mApplicationId;
	mExtendedDataSize = other.mExtendedDataSize;
}

bool nn::hac::DeltaMetaExtendedHeader::operator==(const DeltaMetaExtendedHeader& other) const
{
	return (mApplicationId == other.mApplicationId) \
		&& (mExtendedDataSize == other.mExtendedDataSize);
}

bool nn::hac::DeltaMetaExtendedHeader::operator!=(const DeltaMetaExtendedHeader& other) const
{
	return !(*this == other);
}

void nn::hac::DeltaMetaExtendedHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sDeltaMetaExtendedHeader));
	sDeltaMetaExtendedHeader* info = (sDeltaMetaExtendedHeader*)mRawBinary.data();

	info->application_id = mApplicationId;
	info->extended_data_size = mExtendedDataSize;
}

void nn::hac::DeltaMetaExtendedHeader::fromBytes(const byte_t* bytes, size_t len)
{
	if (len < sizeof(sDeltaMetaExtendedHeader))
	{
		throw fnd::Exception(kModuleName, "DeltaMetaExtendedHeader too small");
	}

	const sDeltaMetaExtendedHeader* info = (const sDeltaMetaExtendedHeader*)bytes;

	mApplicationId = info->application_id.get();
	mExtendedDataSize = info->extended_data_size.get();
}

const fnd::Vec<byte_t>& nn::hac::DeltaMetaExtendedHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::DeltaMetaExtendedHeader::clear()
{
	mRawBinary.clear();
	mApplicationId = 0;
	mExtendedDataSize = 0;
}

uint64_t nn::hac::DeltaMetaExtendedHeader::getApplicationId() const
{
	return mApplicationId;
}

void nn::hac::DeltaMetaExtendedHeader::setApplicationId(uint64_t application_id)
{
	mApplicationId = application_id;
}

uint32_t nn::hac::DeltaMetaExtendedHeader::getExtendedDataSize() const
{
	return mExtendedDataSize;
}

void nn::hac::DeltaMetaExtendedHeader::setExtendedDataSize(uint32_t size)
{
	mExtendedDataSize = size;
}