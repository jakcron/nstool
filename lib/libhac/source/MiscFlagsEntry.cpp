#include <nn/hac/MiscFlagsEntry.h>

nn::hac::MiscFlagsEntry::MiscFlagsEntry() :
	mCap(kCapId),
	mFlags(0)
{}

nn::hac::MiscFlagsEntry::MiscFlagsEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mFlags(0)
{
	setKernelCapability(kernel_cap);
}

nn::hac::MiscFlagsEntry::MiscFlagsEntry(uint32_t flags) :
	mCap(kCapId),
	mFlags(0)
{
	setFlags(flags);
}

void nn::hac::MiscFlagsEntry::operator=(const MiscFlagsEntry& other)
{
	mFlags = other.mFlags;
	updateCapField();
}

bool nn::hac::MiscFlagsEntry::operator==(const MiscFlagsEntry& other) const
{
	return (mFlags == other.mFlags);
}

bool nn::hac::MiscFlagsEntry::operator!=(const MiscFlagsEntry& other) const
{
	return !(*this == other);
}

const nn::hac::KernelCapabilityEntry & nn::hac::MiscFlagsEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::MiscFlagsEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'MiscFlags'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint32_t nn::hac::MiscFlagsEntry::getFlags() const
{
	return mFlags;
}

void nn::hac::MiscFlagsEntry::setFlags(uint32_t flags)
{
	if ((flags & ~kMaxVal) != 0)
	{
		throw fnd::Exception(kModuleName, "Illegal MiscFlag bits set. (settable bits: 0-14 inclusive)");
	}

	mFlags = flags;
	updateCapField();
}
