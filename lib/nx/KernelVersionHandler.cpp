#include "KernelVersionHandler.h"



nx::KernelVersionHandler::KernelVersionHandler() :
	mIsSet(false),
	mEntry(0,0)
{}

bool nx::KernelVersionHandler::operator==(const KernelVersionHandler & other) const
{
	return isEqual(other);
}

bool nx::KernelVersionHandler::operator!=(const KernelVersionHandler & other) const
{
	return !isEqual(other);
}

void nx::KernelVersionHandler::operator=(const KernelVersionHandler & other)
{
	copyFrom(other);
}

void nx::KernelVersionHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
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

void nx::KernelVersionHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nx::KernelVersionHandler::clear()
{
	mIsSet = false;
	mEntry.setVerMajor(0);
	mEntry.setVerMinor(0);
}

bool nx::KernelVersionHandler::isSet() const
{
	return mIsSet;
}

u16 nx::KernelVersionHandler::getVerMajor() const
{
	return mEntry.getVerMajor();
}

void nx::KernelVersionHandler::setVerMajor(u16 major)
{
	mEntry.setVerMajor(major);
	mIsSet = true;
}

u8 nx::KernelVersionHandler::getVerMinor() const
{
	return mEntry.getVerMinor();
}

void nx::KernelVersionHandler::setVerMinor(u8 minor)
{
	mEntry.setVerMinor(minor);
	mIsSet = true;
}

void nx::KernelVersionHandler::copyFrom(const KernelVersionHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::KernelVersionHandler::isEqual(const KernelVersionHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}
