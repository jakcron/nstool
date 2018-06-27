#include <nx/NpdmBinary.h>

#include <fnd/SimpleTextOutput.h>

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
	*this = other;
}

void nx::NpdmBinary::operator=(const NpdmBinary & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		NpdmHeader::operator=(other);
		mAci = other.mAci;
		mAcid = other.mAcid;
	}
}

bool nx::NpdmBinary::operator==(const NpdmBinary & other) const
{
	return (NpdmHeader::operator==(other)) \
		&& (mAci == other.mAci) \
		&& (mAcid == other.mAcid);
}

bool nx::NpdmBinary::operator!=(const NpdmBinary & other) const
{
	return !(*this == other);
}

void nx::NpdmBinary::toBytes()
{
	throw fnd::Exception(kModuleName, "toBytes() not implemented.");
}

void nx::NpdmBinary::fromBytes(const byte_t* data, size_t len)
{
	// clear
	clear();

	// import header
	NpdmHeader::fromBytes(data, len);

	// check size
	if (getNpdmSize() > len)
	{
		throw fnd::Exception(kModuleName, "NPDM binary too small");
	}

	// save local copy
	mRawBinary.alloc(getNpdmSize());
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// import Aci/Acid
	if (getAciPos().size)
	{
		mAci.fromBytes(mRawBinary.data() + getAciPos().offset, getAciPos().size);
	}
	if (getAcidPos().size)
	{
		mAcid.fromBytes(mRawBinary.data() + getAcidPos().offset, getAcidPos().size);
	}	
}

const fnd::Vec<byte_t>& nx::NpdmBinary::getBytes() const
{
	return mRawBinary;
}

void nx::NpdmBinary::clear()
{
	NpdmHeader::clear();
	mAci.clear();
	mAcid.clear();
}

const nx::AccessControlInfoBinary & nx::NpdmBinary::getAci() const
{
	return mAci;
}

void nx::NpdmBinary::setAci(const AccessControlInfoBinary & aci)
{
	mAci = aci;
}

const nx::AccessControlInfoDescBinary & nx::NpdmBinary::getAcid() const
{
	return mAcid;
}

void nx::NpdmBinary::setAcid(const AccessControlInfoDescBinary & acid)
{
	mAcid = acid;
}