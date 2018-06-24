#include <nx/ThreadInfoEntry.h>

nx::ThreadInfoEntry::ThreadInfoEntry() :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCpuId(kDefaultCpuId),
	mMaxCpuId(kDefaultCpuId)
{}

nx::ThreadInfoEntry::ThreadInfoEntry(const KernelCapability & kernel_cap) :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCpuId(kDefaultCpuId),
	mMaxCpuId(kDefaultCpuId)
{
	setKernelCapability(kernel_cap);
}

nx::ThreadInfoEntry::ThreadInfoEntry(uint8_t min_priority, uint8_t max_priority, uint8_t min_core_number, uint8_t max_core_number) :
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

void nx::ThreadInfoEntry::operator=(const ThreadInfoEntry& other)
{
	mMinPriority = other.mMinPriority;
	mMaxPriority = other.mMaxPriority;
	mMinCpuId = other.mMinCpuId;
	mMaxCpuId = other.mMaxCpuId;
	updateCapField();
}

bool nx::ThreadInfoEntry::operator==(const ThreadInfoEntry& other) const
{
	return (mMinPriority == other.mMinPriority) \
		&& (mMaxPriority == other.mMaxPriority) \
		&& (mMinCpuId == other.mMinCpuId) \
		&& (mMaxCpuId == other.mMaxCpuId);
}

bool nx::ThreadInfoEntry::operator!=(const ThreadInfoEntry& other) const
{
	return !(*this == other);
}

const nx::KernelCapability & nx::ThreadInfoEntry::getKernelCapability() const
{
	return mCap;
}

void nx::ThreadInfoEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'ThreadInfo'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint8_t nx::ThreadInfoEntry::getMinPriority() const
{
	return mMinPriority;
}

void nx::ThreadInfoEntry::setMinPriority(uint8_t priority)
{
	if (priority > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinPriority (range 0-63)");
	}

	mMinPriority = priority;
	updateCapField();
}

uint8_t nx::ThreadInfoEntry::getMaxPriority() const
{
	return mMaxPriority;
}

void nx::ThreadInfoEntry::setMaxPriority(uint8_t priority)
{
	if (priority > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxPriority (range 0-63)");
	}

	mMaxPriority = priority;
	updateCapField();
}

uint8_t nx::ThreadInfoEntry::getMinCpuId() const
{
	return mMinCpuId;
}

void nx::ThreadInfoEntry::setMinCpuId(uint8_t core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinCoreNumber (range 0-63)");
	}

	mMinCpuId = core_num;
	updateCapField();
}

uint8_t nx::ThreadInfoEntry::getMaxCpuId() const
{
	return mMaxCpuId;
}

void nx::ThreadInfoEntry::setMaxCpuId(uint8_t core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxCoreNumber (range 0-63)");
	}

	mMaxCpuId = core_num;
	updateCapField();
}
