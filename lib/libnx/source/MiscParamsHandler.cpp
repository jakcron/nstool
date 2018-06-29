#include <nx/MiscParamsHandler.h>

nx::MiscParamsHandler::MiscParamsHandler() :
	mIsSet(false),
	mEntry(0)
{}

void nx::MiscParamsHandler::operator=(const MiscParamsHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::MiscParamsHandler::operator==(const MiscParamsHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}

bool nx::MiscParamsHandler::operator!=(const MiscParamsHandler & other) const
{
	return !(*this == other);
}

void nx::MiscParamsHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
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

void nx::MiscParamsHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
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