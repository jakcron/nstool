#include "SacBinary.h"

using namespace nx;

SacBinary::SacBinary()
{
}

SacBinary::SacBinary(const SacBinary & other)
{
	copyFrom(other);
}

SacBinary::SacBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool SacBinary::operator==(const SacBinary & other) const
{
	return isEqual(other);
}

bool SacBinary::operator!=(const SacBinary & other) const
{
	return !isEqual(other);
}

void SacBinary::operator=(const SacBinary & other)
{
	copyFrom(other);
}

const u8 * SacBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t SacBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void SacBinary::exportBinary()
{
	size_t totalSize = 0;
	for (size_t i = 0; i < mServices.getSize(); i++)
	{
		mServices[i].exportBinary();
		totalSize += mServices[i].getSize();
	}

	mBinaryBlob.alloc(totalSize);
	for (size_t i = 0, pos = 0; i < mServices.getSize(); pos += mServices[i].getSize(), i++)
	{
		memcpy((mBinaryBlob.getBytes() + pos), mServices[i].getBytes(), mServices[i].getSize());
	}
}

void SacBinary::importBinary(const u8 * bytes, size_t len)
{
	clear();
	mBinaryBlob.alloc(len);
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	SacEntry svc;
	for (size_t pos = 0; pos < len; pos += mServices.atBack().getSize())
	{
		svc.importBinary((const u8*)(mBinaryBlob.getBytes() + pos), len - pos);
		mServices.addElement(svc);
	}
}

void nx::SacBinary::clear()
{
	mBinaryBlob.clear();
	mServices.clear();
}

const fnd::List<SacEntry>& SacBinary::getServiceList() const
{
	return mServices;
}

void SacBinary::addService(const SacEntry& service)
{
	mServices.addElement(service);
}

bool SacBinary::isEqual(const SacBinary & other) const
{
	return mServices == other.mServices;
}

void SacBinary::copyFrom(const SacBinary & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		this->mBinaryBlob.clear();
		this->mServices = other.mServices;
	}
}
