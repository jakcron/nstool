#include "ThreadInfoHandler.h"



nx::ThreadInfoHandler::ThreadInfoHandler() :
	mIsSet(false),
	mEntry(0,0,0,0)
{}

bool nx::ThreadInfoHandler::operator==(const ThreadInfoHandler & other) const
{
	return isEqual(other);
}

bool nx::ThreadInfoHandler::operator!=(const ThreadInfoHandler & other) const
{
	return !isEqual(other);
}

void nx::ThreadInfoHandler::operator=(const ThreadInfoHandler & other)
{
	copyFrom(other);
}

void nx::ThreadInfoHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
{
	if (caps.getSize() > kMaxKernelCapNum)
	{
		throw fnd::Exception(kModuleName, "Too many kernel capabilities");
	}

	if (caps.getSize() == 0)
		return;

	mEntry.setKernelCapability(caps[0]);
	mIsSet = true;
}

void nx::ThreadInfoHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nx::ThreadInfoHandler::clear()
{
	mIsSet = false;
	mEntry.setMaxPriority(0);
	mEntry.setMinPriority(0);
	mEntry.setMaxCpuId(0);
	mEntry.setMinCpuId(0);
}

bool nx::ThreadInfoHandler::isSet() const
{
	return mIsSet;
}

u8 nx::ThreadInfoHandler::getMinPriority() const
{
	return mEntry.getMinPriority();
}

void nx::ThreadInfoHandler::setMinPriority(u8 priority)
{
	mEntry.setMinPriority(priority);
	mIsSet = true;
}

u8 nx::ThreadInfoHandler::getMaxPriority() const
{
	return mEntry.getMaxPriority();
}

void nx::ThreadInfoHandler::setMaxPriority(u8 priority)
{
	mEntry.setMaxPriority(priority);
	mIsSet = true;
}

u8 nx::ThreadInfoHandler::getMinCpuId() const
{
	return mEntry.getMinCpuId();
}

void nx::ThreadInfoHandler::setMinCpuId(u8 core_num)
{
	mEntry.setMinCpuId(core_num);
	mIsSet = true;
}

u8 nx::ThreadInfoHandler::getMaxCpuId() const
{
	return mEntry.getMaxCpuId();
}

void nx::ThreadInfoHandler::setMaxCpuId(u8 core_num)
{
	mEntry.setMaxCpuId(core_num);
	mIsSet = true;
}

void nx::ThreadInfoHandler::copyFrom(const ThreadInfoHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::ThreadInfoHandler::isEqual(const ThreadInfoHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}
