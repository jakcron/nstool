#include <nx/HandleTableSizeHandler.h>

nx::HandleTableSizeHandler::HandleTableSizeHandler() :
	mIsSet(false),
	mEntry(0)
{}

void nx::HandleTableSizeHandler::operator=(const HandleTableSizeHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::HandleTableSizeHandler::operator==(const HandleTableSizeHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}

bool nx::HandleTableSizeHandler::operator!=(const HandleTableSizeHandler & other) const
{
	return !(*this == other);
}

void nx::HandleTableSizeHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
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

void nx::HandleTableSizeHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nx::HandleTableSizeHandler::clear()
{
	mIsSet = false;
	mEntry.setHandleTableSize(0);
}

bool nx::HandleTableSizeHandler::isSet() const
{
	return mIsSet;
}

uint16_t nx::HandleTableSizeHandler::getHandleTableSize() const
{
	return mEntry.getHandleTableSize();
}

void nx::HandleTableSizeHandler::setHandleTableSize(uint16_t size)
{
	mEntry.setHandleTableSize(size);
	mIsSet = true;
}