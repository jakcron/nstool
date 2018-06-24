#include <nx/HandleTableSizeEntry.h>

nx::HandleTableSizeEntry::HandleTableSizeEntry() :
	mCap(kCapId),
	mHandleTableSize(0)
{}

nx::HandleTableSizeEntry::HandleTableSizeEntry(const KernelCapability & kernel_cap) :
	mCap(kCapId),
	mHandleTableSize(0)
{
	setKernelCapability(kernel_cap);
}

nx::HandleTableSizeEntry::HandleTableSizeEntry(uint16_t size) :
	mCap(kCapId),
	mHandleTableSize(0)
{
	setHandleTableSize(size);
}

void nx::HandleTableSizeEntry::operator=(const HandleTableSizeEntry& other)
{
	mHandleTableSize = other.mHandleTableSize;
	updateCapField();
}

bool nx::HandleTableSizeEntry::operator==(const HandleTableSizeEntry& other) const
{
	return (mHandleTableSize == other.mHandleTableSize);
}

bool nx::HandleTableSizeEntry::operator!=(const HandleTableSizeEntry& other) const
{
	return !(*this == other);
}


const nx::KernelCapability & nx::HandleTableSizeEntry::getKernelCapability() const
{
	return mCap;
}

void nx::HandleTableSizeEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'HandleTableSize'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint16_t nx::HandleTableSizeEntry::getHandleTableSize() const
{
	return mHandleTableSize;
}

void nx::HandleTableSizeEntry::setHandleTableSize(uint16_t size)
{
	if (size > kMaxHandleTableSize)
	{
		throw fnd::Exception(kModuleName, "Illegal HandleTableSize. (range: 0-1023 inclusive)");
	}

	mHandleTableSize = size;
	updateCapField();
}