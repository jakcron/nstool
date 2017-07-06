#include "memory_blob.h"

using namespace fnd;

MemoryBlob::MemoryBlob() :
	mData(),
	mSize(0),
	mVisableSize(0)
{

}

fnd::MemoryBlob::MemoryBlob(const byte_t * bytes, size_t len) :
	mData(),
	mSize(0),
	mVisableSize(0)
{
	alloc(len);
	memcpy(getBytes(), bytes, getSize());
}

bool fnd::MemoryBlob::operator==(const MemoryBlob & other) const
{
	bool isEqual = true;

	if (this->getSize() == other.getSize())
	{
		isEqual = memcmp(this->getBytes(), other.getBytes(), this->getSize()) == 0;
	}
	else
	{
		isEqual = false;
	}
		

	return isEqual;
}

bool fnd::MemoryBlob::operator!=(const MemoryBlob & other) const
{
	return !operator==(other);
}

void fnd::MemoryBlob::operator=(const MemoryBlob & other)
{
	alloc(other.getSize());
	memcpy(getBytes(), other.getBytes(), getSize());
}

void MemoryBlob::alloc(size_t size)
{
	if (size > mSize)
	{
		allocateMemory(size);
	}
	else
	{
		mVisableSize = size;
		clearMemory();
	}
}

void MemoryBlob::extend(size_t new_size)
{
	try {
		mData.resize(new_size);
	}
	catch (...) {
		throw fnd::Exception(kModuleName, "extend() failed to allocate memory");
	}
}

void fnd::MemoryBlob::clear()
{
	mVisableSize = 0;
}

void MemoryBlob::allocateMemory(size_t size)
{
	mSize = (size_t)align(size, kAllocBlockSize);
	mVisableSize = size;
	extend(mSize);
	clearMemory();
}

void MemoryBlob::clearMemory()
{
	memset(mData.data(), 0, mSize);
}
