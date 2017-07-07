#include "SystemCallEntry.h"



nx::SystemCallEntry::SystemCallEntry() :
	mCap(kCapId),
	mSystemCallUpper(0),
	mSystemCallLower(0)
{

}

nx::SystemCallEntry::SystemCallEntry(const KernelCapability & kernel_cap) :
	mCap(kCapId),
	mSystemCallUpper(0),
	mSystemCallLower(0)
{
	setKernelCapability(kernel_cap);
}

nx::SystemCallEntry::SystemCallEntry(u32 upper_bits, u32 lower_bits) :
	mCap(kCapId),
	mSystemCallUpper(0),
	mSystemCallLower(0)
{
	setSystemCallUpperBits(upper_bits);
	setSystemCallLowerBits(lower_bits);
}

const nx::KernelCapability & nx::SystemCallEntry::getKernelCapability() const
{
	return mCap;
}

void nx::SystemCallEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'EnableSystemCalls'");
	}

	mCap = kernel_cap;
	processCapField();
}

u32 nx::SystemCallEntry::getSystemCallUpperBits() const
{
	return mSystemCallUpper;
}

void nx::SystemCallEntry::setSystemCallUpperBits(u32 upper_bits)
{
	if (upper_bits > kSysCallUpperMax)
	{
		throw fnd::Exception(kModuleName, "Illegal SystemCall upper bits.");
	}

	mSystemCallUpper = upper_bits;
	updateCapField();
}

u32 nx::SystemCallEntry::getSystemCallLowerBits() const
{
	return mSystemCallLower;
}

void nx::SystemCallEntry::setSystemCallLowerBits(u32 lower_bits)
{
	if (lower_bits > kSysCallLowerMax)
	{
		throw fnd::Exception(kModuleName, "Illegal SystemCall upper bits.");
	}

	mSystemCallLower = lower_bits;
	updateCapField();
}
