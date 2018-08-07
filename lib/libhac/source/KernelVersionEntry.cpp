#include <nn/hac/KernelVersionEntry.h>

nn::hac::KernelVersionEntry::KernelVersionEntry() :
	mCap(kCapId),
	mVerMajor(0),
	mVerMinor(0)
{}

nn::hac::KernelVersionEntry::KernelVersionEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mVerMajor(0),
	mVerMinor(0)
{
	setKernelCapability(kernel_cap);
}

nn::hac::KernelVersionEntry::KernelVersionEntry(uint16_t major, uint8_t minor) :
	mCap(kCapId),
	mVerMajor(0),
	mVerMinor(0)
{
	setVerMajor(major);
	setVerMinor(minor);
}

void nn::hac::KernelVersionEntry::operator=(const KernelVersionEntry& other)
{
	mVerMajor = other.mVerMajor;
	mVerMinor = other.mVerMinor;
	updateCapField();
}

bool nn::hac::KernelVersionEntry::operator==(const KernelVersionEntry& other) const
{
	return (mVerMajor == other.mVerMajor) \
		&& (mVerMinor == other.mVerMinor);
}

bool nn::hac::KernelVersionEntry::operator!=(const KernelVersionEntry& other) const
{
	return !(*this == other);
}

const nn::hac::KernelCapabilityEntry & nn::hac::KernelVersionEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::KernelVersionEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'KernelVersion'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint16_t nn::hac::KernelVersionEntry::getVerMajor() const
{
	return mVerMajor;
}

void nn::hac::KernelVersionEntry::setVerMajor(uint16_t major)
{
	if (major > kVerMajorMax)
	{
		throw fnd::Exception(kModuleName, "Illegal KernelVersionMajor. (range: 0-0x1fff)");
	}

	mVerMajor = major;
	updateCapField();
}

uint8_t nn::hac::KernelVersionEntry::getVerMinor() const
{
	return mVerMinor;
}

void nn::hac::KernelVersionEntry::setVerMinor(uint8_t minor)
{
	if (minor > kVerMinorMax)
	{
		throw fnd::Exception(kModuleName, "Illegal KernelVersionMinor. (range: 0-0xf)");
	}

	mVerMinor = minor;
	updateCapField();
}
