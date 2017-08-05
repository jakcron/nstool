#include <nx/AciBinary.h>



nx::AciBinary::AciBinary()
{
	clear();
}

nx::AciBinary::AciBinary(const AciBinary & other)
{
	copyFrom(other);
}

nx::AciBinary::AciBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::AciBinary::operator==(const AciBinary & other) const
{
	return isEqual(other);
}

bool nx::AciBinary::operator!=(const AciBinary & other) const
{
	return !isEqual(other);
}

void nx::AciBinary::operator=(const AciBinary & other)
{
	copyFrom(other);
}

const u8 * nx::AciBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::AciBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::AciBinary::exportBinary()
{
	// export components
	mFac.exportBinary();
	mSac.exportBinary();
	mKc.exportBinary();

	// set sizes
	setFacSize(mFac.getSize());
	setSacSize(mSac.getSize());
	setKcSize(mKc.getSize());

	// export header
	AciHeader::exportBinary();

	// allocate binary
	mBinaryBlob.alloc(getAciSize());

	// copy header
	memcpy(mBinaryBlob.getBytes(), AciHeader::getBytes(), AciHeader::getSize());

	// copy components
	memcpy(mBinaryBlob.getBytes() + getFacPos().offset, mFac.getBytes(), mFac.getSize());
	memcpy(mBinaryBlob.getBytes() + getSacPos().offset, mSac.getBytes(), mSac.getSize());
	memcpy(mBinaryBlob.getBytes() + getKcPos().offset, mKc.getBytes(), mKc.getSize());
}

void nx::AciBinary::importBinary(const u8 * bytes, size_t len)
{
	AciHeader::importBinary(bytes, len);

	if (getAciSize() > len)
	{
		throw fnd::Exception(kModuleName, "ACI binary too small");
	}

	mBinaryBlob.alloc(getAciSize());
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	if (getFacPos().size > 0)
	{
		mFac.importBinary(mBinaryBlob.getBytes() + getFacPos().offset, getFacPos().size);
	}
	if (getSacPos().size > 0)
	{
		mSac.importBinary(mBinaryBlob.getBytes() + getSacPos().offset, getSacPos().size);
	}
	if (getKcPos().size > 0)
	{
		mKc.importBinary(mBinaryBlob.getBytes() + getKcPos().offset, getKcPos().size);
	}

}	

void nx::AciBinary::clear()
{
	AciHeader::clear();
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

bool nx::AciBinary::isEqual(const AciBinary & other) const
{
	return (AciHeader::operator==(other)) \
		&& (mFac == other.mFac) \
		&& (mSac == other.mSac) \
		&& (mKc == other.mKc);
}

void nx::AciBinary::copyFrom(const AciBinary & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		AciHeader::operator=(other);
		mFac = other.mFac;
		mSac = other.mSac;
		mKc = other.mKc;
	}
}
