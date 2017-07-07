#include "InteruptEntry.h"



nx::InteruptEntry::InteruptEntry() :
	mCap(kCapId),
	mInterupt{0,0}
{
	
}

nx::InteruptEntry::InteruptEntry(const KernelCapability & kernel_cap) :
	mCap(kCapId),
	mInterupt{ 0,0 }
{
	setKernelCapability(kernel_cap);
}

nx::InteruptEntry::InteruptEntry(u32 interupt0, u32 interupt1) :
	mCap(kCapId),
	mInterupt{ 0,0 }
{
	setInterupt0(interupt0);
	setInterupt1(interupt1);
}

const nx::KernelCapability & nx::InteruptEntry::getKernelCapability() const
{
	return mCap;
}

void nx::InteruptEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'EnableInterupts'");
	}

	mCap = kernel_cap;
	processCapField();
}

u32 nx::InteruptEntry::getInterupt0() const
{
	return mInterupt[0];
}

void nx::InteruptEntry::setInterupt0(u32 interupt)
{
	if (interupt > kInteruptMax)
	{
		throw fnd::Exception(kModuleName, "Illegal interupt value.");
	}

	mInterupt[0] = interupt;
	updateCapField();
}

u32 nx::InteruptEntry::getInterupt1() const
{
	return mInterupt[1];
}

void nx::InteruptEntry::setInterupt1(u32 interupt)
{
	if (interupt > kInteruptMax)
	{
		throw fnd::Exception(kModuleName, "Illegal interupt value.");
	}

	mInterupt[1] = interupt;
	updateCapField();
}
