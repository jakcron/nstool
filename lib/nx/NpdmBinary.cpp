#include "NpdmBinary.h"



nx::NpdmBinary::NpdmBinary() :
	mAci(),
	mAcid()
{
	clear();
}

nx::NpdmBinary::NpdmBinary(const NpdmBinary & other) :
	mAci(),
	mAcid()
{
	copyFrom(other);
}

nx::NpdmBinary::NpdmBinary(const u8 * bytes, size_t len) :
	mAci(),
	mAcid()
{
	importBinary(bytes, len);
}

bool nx::NpdmBinary::operator==(const NpdmBinary & other) const
{
	return isEqual(other);
}

bool nx::NpdmBinary::operator!=(const NpdmBinary & other) const
{
	return !isEqual(other);
}

void nx::NpdmBinary::operator=(const NpdmBinary & other)
{
	copyFrom(other);
}

void nx::NpdmBinary::exportBinary()
{
	mAci.exportBinary();
	mAcid.exportBinary();

	setAciSize(mAci.getSize());
	setAcidSize(mAcid.getSize());
}

void nx::NpdmBinary::importBinary(const u8 * bytes, size_t len)
{
	// clear
	clear();

	// import header
	NpdmHeader::importBinary(bytes, len);

	// check size
	if (getNpdmSize() > len)
	{
		throw fnd::Exception(kModuleName, "NPDM binary too small");
	}

	// save local copy
	mBinaryBlob.alloc(getNpdmSize());
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	// import Aci/Acid
	if (getAciPos().size)
	{
		mAci.importBinary(mBinaryBlob.getBytes() + getAciPos().offset, getAciPos().size);
	}
	if (getAcidPos().size)
	{
		mAcid.importBinary(mBinaryBlob.getBytes() + getAcidPos().offset, getAcidPos().size);
	}
	
}

void nx::NpdmBinary::clear()
{
	NpdmHeader::clear();
	mAci.clear();
	mAcid.clear();
}

const nx::AciBinary & nx::NpdmBinary::getAci() const
{
	return mAci;
}

void nx::NpdmBinary::setAci(const AciBinary & aci)
{
	mAci = aci;
}

const nx::AcidBinary & nx::NpdmBinary::getAcid() const
{
	return mAcid;
}

void nx::NpdmBinary::setAcid(const AcidBinary & acid)
{
	mAcid = acid;
}

bool nx::NpdmBinary::isEqual(const NpdmBinary & other) const
{
	return (NpdmHeader::operator==(other)) \
		&& (mAci == other.mAci) \
		&& (mAcid == other.mAcid);
}

void nx::NpdmBinary::copyFrom(const NpdmBinary & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		NpdmHeader::operator=(other);
		mAci = other.mAci;
		mAcid = other.mAcid;
	}
}

const u8 * nx::NpdmBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::NpdmBinary::getSize() const
{
	return mBinaryBlob.getSize();
}
