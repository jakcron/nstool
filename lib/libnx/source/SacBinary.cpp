#include <nx/SacBinary.h>

nx::SacBinary::SacBinary()
{
	clear();
}

nx::SacBinary::SacBinary(const SacBinary & other)
{
	*this = other;
}

void nx::SacBinary::operator=(const SacBinary & other)
{
	if (other.getBytes().data())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mServices = other.mServices;
	}
}

bool nx::SacBinary::operator==(const SacBinary & other) const
{
	return (mServices == other.mServices);
}

bool nx::SacBinary::operator!=(const SacBinary & other) const
{
	return !(*this == other);
}

void nx::SacBinary::toBytes()
{
	size_t totalSize = 0;
	for (size_t i = 0; i < mServices.size(); i++)
	{
		mServices[i].toBytes();
		totalSize += mServices[i].getBytes().size();
	}

	mRawBinary.alloc(totalSize);
	for (size_t i = 0, pos = 0; i < mServices.size(); pos += mServices[i].getBytes().size(), i++)
	{
		memcpy((mRawBinary.data() + pos), mServices[i].getBytes().data(), mServices[i].getBytes().size());
	}
}

void nx::SacBinary::fromBytes(const byte_t* data, size_t len)
{
	clear();
	mRawBinary.alloc(len);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	SacEntry sac;
	for (size_t pos = 0; pos < len; pos += mServices.atBack().getBytes().size())
	{
		sac.fromBytes((const byte_t*)(mRawBinary.data() + pos), len - pos);
		mServices.addElement(sac);
	}
}

const fnd::Vec<byte_t>& nx::SacBinary::getBytes() const
{
	return mRawBinary;
}

void nx::SacBinary::clear()
{
	mRawBinary.clear();
	mServices.clear();
}

const fnd::List<nx::SacEntry>& nx::SacBinary::getServiceList() const
{
	return mServices;
}

void nx::SacBinary::addService(const SacEntry& service)
{
	mServices.addElement(service);
}