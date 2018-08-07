#include <nn/hac/InteruptEntry.h>

nn::hac::InteruptEntry::InteruptEntry() :
	mCap(kCapId),
	mInterupt{0,0}
{
	
}

nn::hac::InteruptEntry::InteruptEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mInterupt{ 0,0 }
{
	setKernelCapability(kernel_cap);
}

nn::hac::InteruptEntry::InteruptEntry(uint32_t interupt0, uint32_t interupt1) :
	mCap(kCapId),
	mInterupt{ 0,0 }
{
	setInterupt(0, interupt0);
	setInterupt(1, interupt1);
}

void nn::hac::InteruptEntry::operator=(const InteruptEntry& other)
{
	mInterupt[0] = other.mInterupt[0];
	mInterupt[1] = other.mInterupt[1];
	updateCapField();
}

bool nn::hac::InteruptEntry::operator==(const InteruptEntry& other) const
{
	return (mInterupt[0] == other.mInterupt[0]) \
		&& (mInterupt[1] == other.mInterupt[1]);
}

bool nn::hac::InteruptEntry::operator!=(const InteruptEntry& other) const
{
	return !(*this == other);
}

const nn::hac::KernelCapabilityEntry & nn::hac::InteruptEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::InteruptEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'EnableInterupts'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint32_t nn::hac::InteruptEntry::operator[](size_t index) const
{
	return getInterupt(index);
}

uint32_t nn::hac::InteruptEntry::getInterupt(size_t index) const
{
	return mInterupt[index % kInteruptNum];
}

void nn::hac::InteruptEntry::setInterupt(size_t index, uint32_t interupt)
{
	if (interupt > kInteruptMax)
	{
		throw fnd::Exception(kModuleName, "Illegal interupt value.");
	}

	mInterupt[index % kInteruptNum] = interupt;
	updateCapField();
}