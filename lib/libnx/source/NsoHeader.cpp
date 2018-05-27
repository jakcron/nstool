#include <nx/NsoHeader.h>

nx::NsoHeader::NsoHeader()
{

}

nx::NsoHeader::NsoHeader(const NsoHeader& other)
{
	copyFrom(other);
}

nx::NsoHeader::NsoHeader(const byte_t* bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::NsoHeader::operator==(const NsoHeader& other) const
{
	return isEqual(other);
}

bool nx::NsoHeader::operator!=(const NsoHeader& other) const
{
	return !(*this == other);
}

void nx::NsoHeader::operator=(const NsoHeader& other)
{
	copyFrom(other);
}

const byte_t* nx::NsoHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::NsoHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::NsoHeader::exportBinary()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nx::NsoHeader::importBinary(const byte_t* bytes, size_t len)
{
	throw fnd::Exception(kModuleName, "importBinary() not implemented");
}

void nx::NsoHeader::clear()
{

}

bool nx::NsoHeader::isEqual(const NsoHeader& other) const
{
	return false;
}
void nx::NsoHeader::copyFrom(const NsoHeader& other)
{

}