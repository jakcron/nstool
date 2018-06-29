#include <nx/ServiceAccessControlBinary.h>

nx::ServiceAccessControlBinary::ServiceAccessControlBinary()
{
	clear();
}

nx::ServiceAccessControlBinary::ServiceAccessControlBinary(const ServiceAccessControlBinary & other)
{
	*this = other;
}

void nx::ServiceAccessControlBinary::operator=(const ServiceAccessControlBinary & other)
{
	mRawBinary = other.mRawBinary;
	mServices = other.mServices;
}

bool nx::ServiceAccessControlBinary::operator==(const ServiceAccessControlBinary & other) const
{
	return (mServices == other.mServices);
}

bool nx::ServiceAccessControlBinary::operator!=(const ServiceAccessControlBinary & other) const
{
	return !(*this == other);
}

void nx::ServiceAccessControlBinary::toBytes()
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

void nx::ServiceAccessControlBinary::fromBytes(const byte_t* data, size_t len)
{
	clear();
	mRawBinary.alloc(len);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	ServiceAccessControlEntry sac;
	for (size_t pos = 0; pos < len; pos += mServices.atBack().getBytes().size())
	{
		sac.fromBytes((const byte_t*)(mRawBinary.data() + pos), len - pos);
		mServices.addElement(sac);
	}
}

const fnd::Vec<byte_t>& nx::ServiceAccessControlBinary::getBytes() const
{
	return mRawBinary;
}

void nx::ServiceAccessControlBinary::clear()
{
	mRawBinary.clear();
	mServices.clear();
}

const fnd::List<nx::ServiceAccessControlEntry>& nx::ServiceAccessControlBinary::getServiceList() const
{
	return mServices;
}

void nx::ServiceAccessControlBinary::addService(const ServiceAccessControlEntry& service)
{
	mServices.addElement(service);
}