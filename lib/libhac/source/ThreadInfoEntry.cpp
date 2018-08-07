#include <nn/hac/ThreadInfoEntry.h>

nn::hac::ThreadInfoEntry::ThreadInfoEntry() :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCpuId(kDefaultCpuId),
	mMaxCpuId(kDefaultCpuId)
{}

nn::hac::ThreadInfoEntry::ThreadInfoEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCpuId(kDefaultCpuId),
	mMaxCpuId(kDefaultCpuId)
{
	setKernelCapability(kernel_cap);
}

nn::hac::ThreadInfoEntry::ThreadInfoEntry(uint8_t min_priority, uint8_t max_priority, uint8_t min_core_number, uint8_t max_core_number) :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCpuId(kDefaultCpuId),
	mMaxCpuId(kDefaultCpuId)
{
	setMinPriority(min_priority);
	setMaxPriority(max_priority);
	setMinCpuId(min_core_number);
	setMaxCpuId(max_core_number);
}

void nn::hac::ThreadInfoEntry::operator=(const ThreadInfoEntry& other)
{
	mMinPriority = other.mMinPriority;
	mMaxPriority = other.mMaxPriority;
	mMinCpuId = other.mMinCpuId;
	mMaxCpuId = other.mMaxCpuId;
	updateCapField();
}

bool nn::hac::ThreadInfoEntry::operator==(const ThreadInfoEntry& other) const
{
	return (mMinPriority == other.mMinPriority) \
		&& (mMaxPriority == other.mMaxPriority) \
		&& (mMinCpuId == other.mMinCpuId) \
		&& (mMaxCpuId == other.mMaxCpuId);
}

bool nn::hac::ThreadInfoEntry::operator!=(const ThreadInfoEntry& other) const
{
	return !(*this == other);
}

const nn::hac::KernelCapabilityEntry & nn::hac::ThreadInfoEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::ThreadInfoEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'ThreadInfo'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint8_t nn::hac::ThreadInfoEntry::getMinPriority() const
{
	return mMinPriority;
}

void nn::hac::ThreadInfoEntry::setMinPriority(uint8_t priority)
{
	if (priority > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinPriority (range 0-63)");
	}

	mMinPriority = priority;
	updateCapField();
}

uint8_t nn::hac::ThreadInfoEntry::getMaxPriority() const
{
	return mMaxPriority;
}

void nn::hac::ThreadInfoEntry::setMaxPriority(uint8_t priority)
{
	if (priority > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxPriority (range 0-63)");
	}

	mMaxPriority = priority;
	updateCapField();
}

uint8_t nn::hac::ThreadInfoEntry::getMinCpuId() const
{
	return mMinCpuId;
}

void nn::hac::ThreadInfoEntry::setMinCpuId(uint8_t core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinCoreNumber (range 0-63)");
	}

	mMinCpuId = core_num;
	updateCapField();
}

uint8_t nn::hac::ThreadInfoEntry::getMaxCpuId() const
{
	return mMaxCpuId;
}

void nn::hac::ThreadInfoEntry::setMaxCpuId(uint8_t core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxCoreNumber (range 0-63)");
	}

	mMaxCpuId = core_num;
	updateCapField();
}
