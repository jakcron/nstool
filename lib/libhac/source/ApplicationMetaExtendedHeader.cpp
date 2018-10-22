#include <nn/hac/ApplicationMetaExtendedHeader.h>

nn::hac::ApplicationMetaExtendedHeader::ApplicationMetaExtendedHeader()
{
	clear();
}

nn::hac::ApplicationMetaExtendedHeader::ApplicationMetaExtendedHeader(const ApplicationMetaExtendedHeader& other)
{
	*this = other;
}

void nn::hac::ApplicationMetaExtendedHeader::operator=(const ApplicationMetaExtendedHeader& other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mPatchId = other.mPatchId;
	mRequiredSystemVersion = other.mRequiredSystemVersion;
}

bool nn::hac::ApplicationMetaExtendedHeader::operator==(const ApplicationMetaExtendedHeader& other) const
{
	return (mPatchId == other.mPatchId) \
		&& (mRequiredSystemVersion == other.mRequiredSystemVersion);
}

bool nn::hac::ApplicationMetaExtendedHeader::operator!=(const ApplicationMetaExtendedHeader& other) const
{
	return !(*this == other);
}

void nn::hac::ApplicationMetaExtendedHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sApplicationMetaExtendedHeader));
	sApplicationMetaExtendedHeader* info = (sApplicationMetaExtendedHeader*)mRawBinary.data();

	info->patch_id = mPatchId;
	info->required_system_version = mRequiredSystemVersion;
}

void nn::hac::ApplicationMetaExtendedHeader::fromBytes(const byte_t* bytes, size_t len)
{
	if (len < sizeof(sApplicationMetaExtendedHeader))
	{
		throw fnd::Exception(kModuleName, "ApplicationMetaExtendedHeader too small");
	}

	const sApplicationMetaExtendedHeader* info = (const sApplicationMetaExtendedHeader*)bytes;

	mPatchId = info->patch_id.get();
	mRequiredSystemVersion = info->required_system_version.get();
}

const fnd::Vec<byte_t>& nn::hac::ApplicationMetaExtendedHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::ApplicationMetaExtendedHeader::clear()
{
	mRawBinary.clear();
	mPatchId = 0;
	mRequiredSystemVersion = 0;
}

uint64_t nn::hac::ApplicationMetaExtendedHeader::getPatchId() const
{
	return mPatchId;
}

void nn::hac::ApplicationMetaExtendedHeader::setPatchId(uint64_t application_id)
{
	mPatchId = application_id;
}

uint32_t nn::hac::ApplicationMetaExtendedHeader::getRequiredSystemVersion() const
{
	return mRequiredSystemVersion;
}

void nn::hac::ApplicationMetaExtendedHeader::setRequiredSystemVersion(uint32_t sys_ver)
{
	mRequiredSystemVersion = sys_ver;
}