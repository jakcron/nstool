#include <nx/KernelVersionEntry.h>



nx::KernelVersionEntry::KernelVersionEntry() :
	mCap(kCapId),
	mVerMajor(0),
	mVerMinor(0)
{}

nx::KernelVersionEntry::KernelVersionEntry(const KernelCapability & kernel_cap) :
	mCap(kCapId),
	mVerMajor(0),
	mVerMinor(0)
{
	setKernelCapability(kernel_cap);
}

nx::KernelVersionEntry::KernelVersionEntry(uint16_t major, uint8_t minor) :
	mCap(kCapId),
	mVerMajor(0),
	mVerMinor(0)
{
	setVerMajor(major);
	setVerMinor(minor);
}

const nx::KernelCapability & nx::KernelVersionEntry::getKernelCapability() const
{
	return mCap;
}

void nx::KernelVersionEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'KernelVersion'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint16_t nx::KernelVersionEntry::getVerMajor() const
{
	return mVerMajor;
}

void nx::KernelVersionEntry::setVerMajor(uint16_t major)
{
	if (major > kVerMajorMax)
	{
		throw fnd::Exception(kModuleName, "Illegal KernelVersionMajor. (range: 0-0x1fff)");
	}

	mVerMajor = major;
	updateCapField();
}

uint8_t nx::KernelVersionEntry::getVerMinor() const
{
	return mVerMinor;
}

void nx::KernelVersionEntry::setVerMinor(uint8_t minor)
{
	if (minor > kVerMinorMax)
	{
		throw fnd::Exception(kModuleName, "Illegal KernelVersionMinor. (range: 0-0xf)");
	}

	mVerMinor = minor;
	updateCapField();
}
