#include <nx/InteruptHandler.h>

nx::InteruptHandler::InteruptHandler() :
	mIsSet(false),
	mInterupts()
{}

void nx::InteruptHandler::operator=(const InteruptHandler & other)
{
	mIsSet = other.mIsSet;
	mInterupts = other.mInterupts;
}

bool nx::InteruptHandler::operator==(const InteruptHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mInterupts == other.mInterupts);
}

bool nx::InteruptHandler::operator!=(const InteruptHandler & other) const
{
	return !(*this == other);
}

void nx::InteruptHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
{
	if (caps.size() == 0)
		return;

	// convert to interupts
	fnd::List<InteruptEntry> interupts;
	for (size_t i = 0; i < caps.size(); i++)
	{
		interupts[i].setKernelCapability(caps[i]);
	}

	mInterupts.clear();
	for (size_t i = 0; i < interupts.size(); i++)
	{
		// weird condition for first interupt
		if (interupts[i][1] == 0 && i == 0)
		{
			mInterupts.addElement(interupts[i][0]);
			continue;
		}

		// we found a stub
		if (interupts[i][1] == InteruptEntry::kInteruptMax && interupts[i][1] == InteruptEntry::kInteruptMax)
		{
			continue;
		}

		// add interupts
		mInterupts.hasElement(interupts[i][0]) == false ? mInterupts.addElement(interupts[i][0]) : throw fnd::Exception(kModuleName, "Interupt already added");
		mInterupts.hasElement(interupts[i][1]) == false ? mInterupts.addElement(interupts[i][1]) : throw fnd::Exception(kModuleName, "Interupt already added");
	}

	mIsSet = true;
}

void nx::InteruptHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	size_t i = 0;
	if (mInterupts.size() % 2)
	{
		caps.addElement(InteruptEntry(mInterupts[i], 0).getKernelCapability());
		i++;
	}
	for (; i < mInterupts.size(); i += 2)
	{
		if (mInterupts[i] == InteruptEntry::kInteruptMax)
		{
			caps.addElement(InteruptEntry(InteruptEntry::kInteruptMax, InteruptEntry::kInteruptMax).getKernelCapability());
		}
		caps.addElement(InteruptEntry(mInterupts[i], mInterupts[i+1]).getKernelCapability());
		if (mInterupts[i+1] == InteruptEntry::kInteruptMax)
		{
			caps.addElement(InteruptEntry(InteruptEntry::kInteruptMax, InteruptEntry::kInteruptMax).getKernelCapability());
		}

	}
}

void nx::InteruptHandler::clear()
{
	mIsSet = false;
	mInterupts.clear();
}

bool nx::InteruptHandler::isSet() const
{
	return mIsSet;
}

const fnd::List<uint16_t>& nx::InteruptHandler::getInteruptList() const
{
	return mInterupts;
}

void nx::InteruptHandler::setInteruptList(const fnd::List<uint16_t>& interupts)
{
	mInterupts.clear();
	for (size_t i = 0; i < interupts.size(); i++)
	{
		mInterupts.hasElement(interupts[i]) == false ? mInterupts.addElement(interupts[i]) : throw fnd::Exception(kModuleName, "Interupt already added");
	}

	mIsSet = true;
}