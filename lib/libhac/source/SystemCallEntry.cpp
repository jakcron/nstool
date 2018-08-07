#include <nn/hac/SystemCallEntry.h>

nn::hac::SystemCallEntry::SystemCallEntry() :
	mCap(kCapId),
	mSystemCallUpper(0),
	mSystemCallLower(0)
{

}

nn::hac::SystemCallEntry::SystemCallEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mSystemCallUpper(0),
	mSystemCallLower(0)
{
	setKernelCapability(kernel_cap);
}

nn::hac::SystemCallEntry::SystemCallEntry(uint32_t upper_bits, uint32_t lower_bits) :
	mCap(kCapId),
	mSystemCallUpper(0),
	mSystemCallLower(0)
{
	setSystemCallUpperBits(upper_bits);
	setSystemCallLowerBits(lower_bits);
}

void nn::hac::SystemCallEntry::operator=(const SystemCallEntry& other)
{
	mSystemCallUpper = other.mSystemCallUpper;
	mSystemCallLower = other.mSystemCallLower;
	updateCapField();
}

bool nn::hac::SystemCallEntry::operator==(const SystemCallEntry& other) const
{
	return (mSystemCallUpper == other.mSystemCallUpper) \
		&& (mSystemCallLower == other.mSystemCallLower);
}

bool nn::hac::SystemCallEntry::operator!=(const SystemCallEntry& other) const
{
	return !(*this == other);
}

const nn::hac::KernelCapabilityEntry & nn::hac::SystemCallEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::SystemCallEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'EnableSystemCalls'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint32_t nn::hac::SystemCallEntry::getSystemCallUpperBits() const
{
	return mSystemCallUpper;
}

void nn::hac::SystemCallEntry::setSystemCallUpperBits(uint32_t upper_bits)
{
	if (upper_bits > kSysCallUpperMax)
	{
		throw fnd::Exception(kModuleName, "Illegal SystemCall upper bits.");
	}

	mSystemCallUpper = upper_bits;
	updateCapField();
}

uint32_t nn::hac::SystemCallEntry::getSystemCallLowerBits() const
{
	return mSystemCallLower;
}

void nn::hac::SystemCallEntry::setSystemCallLowerBits(uint32_t lower_bits)
{
	if (lower_bits > kSysCallLowerMax)
	{
		throw fnd::Exception(kModuleName, "Illegal SystemCall upper bits.");
	}

	mSystemCallLower = lower_bits;
	updateCapField();
}
