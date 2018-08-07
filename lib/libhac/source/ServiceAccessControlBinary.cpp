#include <cstring>
#include <nn/hac/ServiceAccessControlBinary.h>

nn::hac::ServiceAccessControlBinary::ServiceAccessControlBinary()
{
	clear();
}

nn::hac::ServiceAccessControlBinary::ServiceAccessControlBinary(const ServiceAccessControlBinary & other)
{
	*this = other;
}

void nn::hac::ServiceAccessControlBinary::operator=(const ServiceAccessControlBinary & other)
{
	mRawBinary = other.mRawBinary;
	mServices = other.mServices;
}

bool nn::hac::ServiceAccessControlBinary::operator==(const ServiceAccessControlBinary & other) const
{
	return (mServices == other.mServices);
}

bool nn::hac::ServiceAccessControlBinary::operator!=(const ServiceAccessControlBinary & other) const
{
	return !(*this == other);
}

void nn::hac::ServiceAccessControlBinary::toBytes()
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

void nn::hac::ServiceAccessControlBinary::fromBytes(const byte_t* data, size_t len)
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

const fnd::Vec<byte_t>& nn::hac::ServiceAccessControlBinary::getBytes() const
{
	return mRawBinary;
}

void nn::hac::ServiceAccessControlBinary::clear()
{
	mRawBinary.clear();
	mServices.clear();
}

const fnd::List<nn::hac::ServiceAccessControlEntry>& nn::hac::ServiceAccessControlBinary::getServiceList() const
{
	return mServices;
}

void nn::hac::ServiceAccessControlBinary::addService(const ServiceAccessControlEntry& service)
{
	mServices.addElement(service);
}