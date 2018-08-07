#include <nn/hac/HandleTableSizeHandler.h>

nn::hac::HandleTableSizeHandler::HandleTableSizeHandler() :
	mIsSet(false),
	mEntry(0)
{}

void nn::hac::HandleTableSizeHandler::operator=(const HandleTableSizeHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nn::hac::HandleTableSizeHandler::operator==(const HandleTableSizeHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}

bool nn::hac::HandleTableSizeHandler::operator!=(const HandleTableSizeHandler & other) const
{
	return !(*this == other);
}

void nn::hac::HandleTableSizeHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
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

void nn::hac::HandleTableSizeHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nn::hac::HandleTableSizeHandler::clear()
{
	mIsSet = false;
	mEntry.setHandleTableSize(0);
}

bool nn::hac::HandleTableSizeHandler::isSet() const
{
	return mIsSet;
}

uint16_t nn::hac::HandleTableSizeHandler::getHandleTableSize() const
{
	return mEntry.getHandleTableSize();
}

void nn::hac::HandleTableSizeHandler::setHandleTableSize(uint16_t size)
{
	mEntry.setHandleTableSize(size);
	mIsSet = true;
}