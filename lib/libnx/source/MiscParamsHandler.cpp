#include <nx/MiscParamsHandler.h>



nx::MiscParamsHandler::MiscParamsHandler() :
	mIsSet(false),
	mEntry(0)
{}

bool nx::MiscParamsHandler::operator==(const MiscParamsHandler & other) const
{
	return isEqual(other);
}

bool nx::MiscParamsHandler::operator!=(const MiscParamsHandler & other) const
{
	return !isEqual(other);
}

void nx::MiscParamsHandler::operator=(const MiscParamsHandler & other)
{
	copyFrom(other);
}

void nx::MiscParamsHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
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

void nx::MiscParamsHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nx::MiscParamsHandler::clear()
{
	mIsSet = false;
	mEntry.setProgramType(0);
}

bool nx::MiscParamsHandler::isSet() const
{
	return mIsSet;
}

uint8_t nx::MiscParamsHandler::getProgramType() const
{
	return mEntry.getProgramType();
}

void nx::MiscParamsHandler::setProgramType(uint8_t type)
{
	mEntry.setProgramType(type);
	mIsSet = true;
}

void nx::MiscParamsHandler::copyFrom(const MiscParamsHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::MiscParamsHandler::isEqual(const MiscParamsHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}
