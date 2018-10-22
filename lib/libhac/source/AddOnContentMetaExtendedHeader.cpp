#include <nn/hac/AddOnContentMetaExtendedHeader.h>

nn::hac::AddOnContentMetaExtendedHeader::AddOnContentMetaExtendedHeader()
{
	clear();
}

nn::hac::AddOnContentMetaExtendedHeader::AddOnContentMetaExtendedHeader(const AddOnContentMetaExtendedHeader& other)
{
	*this = other;
}

void nn::hac::AddOnContentMetaExtendedHeader::operator=(const AddOnContentMetaExtendedHeader& other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mApplicationId = other.mApplicationId;
	mRequiredApplicationVersion = other.mRequiredApplicationVersion;
}

bool nn::hac::AddOnContentMetaExtendedHeader::operator==(const AddOnContentMetaExtendedHeader& other) const
{
	return (mApplicationId == other.mApplicationId) \
		&& (mRequiredApplicationVersion == other.mRequiredApplicationVersion);
}

bool nn::hac::AddOnContentMetaExtendedHeader::operator!=(const AddOnContentMetaExtendedHeader& other) const
{
	return !(*this == other);
}

void nn::hac::AddOnContentMetaExtendedHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sAddOnContentMetaExtendedHeader));
	sAddOnContentMetaExtendedHeader* info = (sAddOnContentMetaExtendedHeader*)mRawBinary.data();

	info->application_id = mApplicationId;
	info->required_application_version = mRequiredApplicationVersion;
}

void nn::hac::AddOnContentMetaExtendedHeader::fromBytes(const byte_t* bytes, size_t len)
{
	if (len < sizeof(sAddOnContentMetaExtendedHeader))
	{
		throw fnd::Exception(kModuleName, "AddOnContentMetaExtendedHeader too small");
	}

	const sAddOnContentMetaExtendedHeader* info = (const sAddOnContentMetaExtendedHeader*)bytes;

	mApplicationId = info->application_id.get();
	mRequiredApplicationVersion = info->required_application_version.get();
}

const fnd::Vec<byte_t>& nn::hac::AddOnContentMetaExtendedHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::AddOnContentMetaExtendedHeader::clear()
{
	mRawBinary.clear();
	mApplicationId = 0;
	mRequiredApplicationVersion = 0;
}

uint64_t nn::hac::AddOnContentMetaExtendedHeader::getApplicationId() const
{
	return mApplicationId;
}

void nn::hac::AddOnContentMetaExtendedHeader::setApplicationId(uint64_t application_id)
{
	mApplicationId = application_id;
}

uint32_t nn::hac::AddOnContentMetaExtendedHeader::getRequiredApplicationVersion() const
{
	return mRequiredApplicationVersion;
}

void nn::hac::AddOnContentMetaExtendedHeader::setRequiredApplicationVersion(uint32_t sys_ver)
{
	mRequiredApplicationVersion = sys_ver;
}