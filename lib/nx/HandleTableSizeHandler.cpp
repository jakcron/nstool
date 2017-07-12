#include "HandleTableSizeHandler.h"



nx::HandleTableSizeHandler::HandleTableSizeHandler() :
	mIsSet(false),
	mEntry(0)
{}

bool nx::HandleTableSizeHandler::operator==(const HandleTableSizeHandler & other) const
{
	return isEqual(other);
}

bool nx::HandleTableSizeHandler::operator!=(const HandleTableSizeHandler & other) const
{
	return !isEqual(other);
}

void nx::HandleTableSizeHandler::operator=(const HandleTableSizeHandler & other)
{
	copyFrom(other);
}

void nx::HandleTableSizeHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
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

void nx::HandleTableSizeHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
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

u16 nx::HandleTableSizeHandler::getHandleTableSize() const
{
	return mEntry.getHandleTableSize();
}

void nx::HandleTableSizeHandler::setHandleTableSize(u16 size)
{
	mEntry.setHandleTableSize(size);
	mIsSet = true;
}

void nx::HandleTableSizeHandler::copyFrom(const HandleTableSizeHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::HandleTableSizeHandler::isEqual(const HandleTableSizeHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}
