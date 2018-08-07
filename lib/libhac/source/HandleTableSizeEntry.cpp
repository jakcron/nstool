#include <nn/hac/HandleTableSizeEntry.h>

nn::hac::HandleTableSizeEntry::HandleTableSizeEntry() :
	mCap(kCapId),
	mHandleTableSize(0)
{}

nn::hac::HandleTableSizeEntry::HandleTableSizeEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mHandleTableSize(0)
{
	setKernelCapability(kernel_cap);
}

nn::hac::HandleTableSizeEntry::HandleTableSizeEntry(uint16_t size) :
	mCap(kCapId),
	mHandleTableSize(0)
{
	setHandleTableSize(size);
}

void nn::hac::HandleTableSizeEntry::operator=(const HandleTableSizeEntry& other)
{
	mHandleTableSize = other.mHandleTableSize;
	updateCapField();
}

bool nn::hac::HandleTableSizeEntry::operator==(const HandleTableSizeEntry& other) const
{
	return (mHandleTableSize == other.mHandleTableSize);
}

bool nn::hac::HandleTableSizeEntry::operator!=(const HandleTableSizeEntry& other) const
{
	return !(*this == other);
}


const nn::hac::KernelCapabilityEntry & nn::hac::HandleTableSizeEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::HandleTableSizeEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'HandleTableSize'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint16_t nn::hac::HandleTableSizeEntry::getHandleTableSize() const
{
	return mHandleTableSize;
}

void nn::hac::HandleTableSizeEntry::setHandleTableSize(uint16_t size)
{
	if (size > kMaxHandleTableSize)
	{
		throw fnd::Exception(kModuleName, "Illegal HandleTableSize. (range: 0-1023 inclusive)");
	}

	mHandleTableSize = size;
	updateCapField();
}