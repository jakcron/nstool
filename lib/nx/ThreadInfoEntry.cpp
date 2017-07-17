#include "ThreadInfoEntry.h"



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

nx::ThreadInfoEntry::ThreadInfoEntry(u8 min_priority, u8 max_priority, u8 min_core_number, u8 max_core_number) :
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

u8 nx::ThreadInfoEntry::getMinPriority() const
{
	return mMinPriority;
}

void nx::ThreadInfoEntry::setMinPriority(u8 priority)
{
	if (priority > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinPriority (range 0-63)");
	}

	mMinPriority = priority;
	updateCapField();
}

u8 nx::ThreadInfoEntry::getMaxPriority() const
{
	return mMaxPriority;
}

void nx::ThreadInfoEntry::setMaxPriority(u8 priority)
{
	if (priority > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxPriority (range 0-63)");
	}

	mMaxPriority = priority;
	updateCapField();
}

u8 nx::ThreadInfoEntry::getMinCpuId() const
{
	return mMinCpuId;
}

void nx::ThreadInfoEntry::setMinCpuId(u8 core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinCoreNumber (range 0-63)");
	}

	mMinCpuId = core_num;
	updateCapField();
}

u8 nx::ThreadInfoEntry::getMaxCpuId() const
{
	return mMaxCpuId;
}

void nx::ThreadInfoEntry::setMaxCpuId(u8 core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxCoreNumber (range 0-63)");
	}

	mMaxCpuId = core_num;
	updateCapField();
}
