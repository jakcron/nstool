#include <nx/InteruptEntry.h>



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
	setInterupt(0, interupt0);
	setInterupt(1, interupt1);
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

u32 nx::InteruptEntry::operator[](size_t index) const
{
	return getInterupt(index);
}

u32 nx::InteruptEntry::getInterupt(size_t index) const
{
	return mInterupt[index % kInteruptNum];
}

void nx::InteruptEntry::setInterupt(size_t index, u32 interupt)
{
	if (interupt > kInteruptMax)
	{
		throw fnd::Exception(kModuleName, "Illegal interupt value.");
	}

	mInterupt[index % kInteruptNum] = interupt;
	updateCapField();
}