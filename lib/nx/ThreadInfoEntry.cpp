#include "ThreadInfoEntry.h"



nx::ThreadInfoEntry::ThreadInfoEntry() :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCoreNumber(kDefaultCoreNumber),
	mMaxCoreNumber(kDefaultCoreNumber)
{}

nx::ThreadInfoEntry::ThreadInfoEntry(const KernelCapability & kernel_cap) :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCoreNumber(kDefaultCoreNumber),
	mMaxCoreNumber(kDefaultCoreNumber)
{
	setKernelCapability(kernel_cap);
}

nx::ThreadInfoEntry::ThreadInfoEntry(u8 min_priority, u8 max_priority, u8 min_core_number, u8 max_core_number) :
	mCap(kCapId),
	mMinPriority(kDefaultPriority),
	mMaxPriority(kDefaultPriority),
	mMinCoreNumber(kDefaultCoreNumber),
	mMaxCoreNumber(kDefaultCoreNumber)
{
	setMinPriority(min_priority);
	setMaxPriority(max_priority);
	setMinCoreNumber(min_core_number);
	setMaxCoreNumber(max_core_number);
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

u8 nx::ThreadInfoEntry::getMinCoreNumber() const
{
	return mMinCoreNumber;
}

void nx::ThreadInfoEntry::setMinCoreNumber(u8 core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MinCoreNumber (range 0-63)");
	}

	mMinCoreNumber = core_num;
	updateCapField();
}

u8 nx::ThreadInfoEntry::getMaxCoreNumber() const
{
	return mMaxCoreNumber;
}

void nx::ThreadInfoEntry::setMaxCoreNumber(u8 core_num)
{
	if (core_num > kMaxVal)
	{
		throw fnd::Exception(kModuleName, "Illegal MaxCoreNumber (range 0-63)");
	}

	mMaxCoreNumber = core_num;
	updateCapField();
}
