#include <nn/hac/ThreadInfoHandler.h>

nn::hac::ThreadInfoHandler::ThreadInfoHandler() :
	mIsSet(false),
	mEntry(0,0,0,0)
{}

void nn::hac::ThreadInfoHandler::operator=(const ThreadInfoHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nn::hac::ThreadInfoHandler::operator==(const ThreadInfoHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}

bool nn::hac::ThreadInfoHandler::operator!=(const ThreadInfoHandler & other) const
{
	return !(*this == other);
}

void nn::hac::ThreadInfoHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
{
	if (caps.size() > kMaxKernelCapNum)
	{
		throw fnd::Exception(kModuleName, "Too many kernel capabilities");
	}

	if (caps.size() == 0)
		return;

	mEntry.setKernelCapability(caps[0]);
	mIsSet = true;
}

void nn::hac::ThreadInfoHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nn::hac::ThreadInfoHandler::clear()
{
	mIsSet = false;
	mEntry.setMaxPriority(0);
	mEntry.setMinPriority(0);
	mEntry.setMaxCpuId(0);
	mEntry.setMinCpuId(0);
}

bool nn::hac::ThreadInfoHandler::isSet() const
{
	return mIsSet;
}

uint8_t nn::hac::ThreadInfoHandler::getMinPriority() const
{
	return mEntry.getMinPriority();
}

void nn::hac::ThreadInfoHandler::setMinPriority(uint8_t priority)
{
	mEntry.setMinPriority(priority);
	mIsSet = true;
}

uint8_t nn::hac::ThreadInfoHandler::getMaxPriority() const
{
	return mEntry.getMaxPriority();
}

void nn::hac::ThreadInfoHandler::setMaxPriority(uint8_t priority)
{
	mEntry.setMaxPriority(priority);
	mIsSet = true;
}

uint8_t nn::hac::ThreadInfoHandler::getMinCpuId() const
{
	return mEntry.getMinCpuId();
}

void nn::hac::ThreadInfoHandler::setMinCpuId(uint8_t core_num)
{
	mEntry.setMinCpuId(core_num);
	mIsSet = true;
}

uint8_t nn::hac::ThreadInfoHandler::getMaxCpuId() const
{
	return mEntry.getMaxCpuId();
}

void nn::hac::ThreadInfoHandler::setMaxCpuId(uint8_t core_num)
{
	mEntry.setMaxCpuId(core_num);
	mIsSet = true;
}