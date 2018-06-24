#include <nx/AciBinary.h>

nx::AciBinary::AciBinary()
{
	clear();
}

nx::AciBinary::AciBinary(const AciBinary & other)
{
	*this = other;
}

void nx::AciBinary::operator=(const AciBinary & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		AciHeader::operator=(other);
		mFac = other.mFac;
		mSac = other.mSac;
		mKc = other.mKc;
	}
}

bool nx::AciBinary::operator==(const AciBinary & other) const
{
	return (AciHeader::operator==(other)) \
		&& (mFac == other.mFac) \
		&& (mSac == other.mSac) \
		&& (mKc == other.mKc);
}

bool nx::AciBinary::operator!=(const AciBinary & other) const
{
	return !(*this == other);
}

void nx::AciBinary::toBytes()
{
	// export components
	mFac.toBytes();
	mSac.toBytes();
	mKc.toBytes();

	// set sizes
	setFacSize(mFac.getBytes().size());
	setSacSize(mSac.getBytes().size());
	setKcSize(mKc.getBytes().size());

	// export header
	AciHeader::toBytes();

	// allocate binary
	mRawBinary.alloc(getAciSize());

	// copy header
	memcpy(mRawBinary.data(), AciHeader::getBytes().data(), AciHeader::getBytes().size());

	// copy components
	memcpy(mRawBinary.data() + getFacPos().offset, mFac.getBytes().data(), mFac.getBytes().size());
	memcpy(mRawBinary.data() + getSacPos().offset, mSac.getBytes().data(), mSac.getBytes().size());
	memcpy(mRawBinary.data() + getKcPos().offset, mKc.getBytes().data(), mKc.getBytes().size());
}

void nx::AciBinary::fromBytes(const byte_t * bytes, size_t len)
{
	clear();

	AciHeader::fromBytes(bytes, len);

	if (getAciSize() > len)
	{
		throw fnd::Exception(kModuleName, "ACI binary too small");
	}

	mRawBinary.alloc(getAciSize());
	memcpy(mRawBinary.data(), bytes, mRawBinary.size());

	if (getFacPos().size > 0)
	{
		mFac.fromBytes(mRawBinary.data() + getFacPos().offset, getFacPos().size);
	}
	if (getSacPos().size > 0)
	{
		mSac.fromBytes(mRawBinary.data() + getSacPos().offset, getSacPos().size);
	}
	if (getKcPos().size > 0)
	{
		mKc.fromBytes(mRawBinary.data() + getKcPos().offset, getKcPos().size);
	}

}	

const fnd::Vec<byte_t>& nx::AciBinary::getBytes() const
{
	return mRawBinary;
}

void nx::AciBinary::clear()
{
	AciHeader::clear();
	mRawBinary.clear();
	mFac.clear();
	mSac.clear();
	mKc.clear();
}

const nx::FacBinary & nx::AciBinary::getFac() const
{
	return mFac;
}

void nx::AciBinary::setFac(const FacBinary & fac)
{
	mFac = fac;
}

const nx::SacBinary & nx::AciBinary::getSac() const
{
	return mSac;
}

void nx::AciBinary::setSac(const SacBinary & sac)
{
	mSac = sac;
}

const nx::KcBinary & nx::AciBinary::getKc() const
{
	return mKc;
}

void nx::AciBinary::setKc(const KcBinary & kc)
{
	mKc = kc;
}