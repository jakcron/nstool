#include <nx/MiscFlagsHandler.h>

nx::MiscFlagsHandler::MiscFlagsHandler() :
	mIsSet(false)
{}

void nx::MiscFlagsHandler::operator=(const MiscFlagsHandler & other)
{
	mIsSet = other.mIsSet;
	mFlags = other.mFlags;
}

bool nx::MiscFlagsHandler::operator==(const MiscFlagsHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mFlags == other.mFlags);
}

bool nx::MiscFlagsHandler::operator!=(const MiscFlagsHandler & other) const
{
	return !(*this == other);
}

void nx::MiscFlagsHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
{
	if (caps.size() > kMaxKernelCapNum)
	{
		throw fnd::Exception(kModuleName, "Too many kernel capabilities");
	}

	if (caps.size() == 0)
		return;

	MiscFlagsEntry entry;
	entry.setKernelCapability(caps[0]);
	
	clear();
	for (uint32_t i = 0; i < FLAG_NUM; i++)
	{
		if ((entry.getFlags() & BIT(i)) == BIT(i))
		{
			mFlags.addElement((Flags)i);
		}
	}

	mIsSet = true;
}

void nx::MiscFlagsHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
{
	if (isSet() == false)
		return;

	// convert list to word flags
	uint32_t flag = 0;
	for (size_t i = 0; i < mFlags.size(); i++)
	{
		flag |= BIT(mFlags[i]);
	}

	// create MiscFlag entry
	MiscFlagsEntry entry;
	entry.setFlags(flag);

	// add to list
	caps.addElement(entry.getKernelCapability());
}

void nx::MiscFlagsHandler::clear()
{
	mIsSet = false;
	mFlags.clear();
}

bool nx::MiscFlagsHandler::isSet() const
{
	return mIsSet;
}

const fnd::List<nx::MiscFlagsHandler::Flags>& nx::MiscFlagsHandler::getFlagList() const
{
	return mFlags;
}

void nx::MiscFlagsHandler::setFlagList(fnd::List<Flags> flags)
{
	mFlags = flags;
	mIsSet = true;
}