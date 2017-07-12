#include "MiscFlagsHandler.h"



nx::MiscFlagsHandler::MiscFlagsHandler() :
	mIsSet(false)
{}

void nx::MiscFlagsHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
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

void nx::MiscFlagsHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nx::MiscFlagsHandler::clear()
{
	mIsSet = false;
	mEntry.setFlags(0);
}

bool nx::MiscFlagsHandler::isSet() const
{
	return mIsSet;
}

u32 nx::MiscFlagsHandler::getFlags() const
{
	return mEntry.getFlags();
}

void nx::MiscFlagsHandler::setFlags(u32 flags)
{
	mEntry.setFlags(flags);
	mIsSet = true;
}

void nx::MiscFlagsHandler::copyFrom(const MiscFlagsHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::MiscFlagsHandler::isEqual(const MiscFlagsHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}
