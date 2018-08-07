#include <nn/hac/MiscParamsEntry.h>

nn::hac::MiscParamsEntry::MiscParamsEntry() :
	mCap(kCapId),
	mProgramType(0)
{}

nn::hac::MiscParamsEntry::MiscParamsEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mProgramType(0)
{
	setKernelCapability(kernel_cap);
}

nn::hac::MiscParamsEntry::MiscParamsEntry(uint8_t program_type) :
	mCap(kCapId),
	mProgramType(0)
{
	setProgramType(program_type);
}

void nn::hac::MiscParamsEntry::operator=(const MiscParamsEntry& other)
{
	mProgramType = other.mProgramType;
	updateCapField();
}

bool nn::hac::MiscParamsEntry::operator==(const MiscParamsEntry& other) const
{
	return (mProgramType == other.mProgramType);
}

bool nn::hac::MiscParamsEntry::operator!=(const MiscParamsEntry& other) const
{
	return !(*this == other);
}

const nn::hac::KernelCapabilityEntry & nn::hac::MiscParamsEntry::getKernelCapability() const
{
	return mCap;
}

void nn::hac::MiscParamsEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'ThreadInfo'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint8_t nn::hac::MiscParamsEntry::getProgramType() const
{
	return mProgramType;
}

void nn::hac::MiscParamsEntry::setProgramType(uint8_t type)
{
	if (type > kMaxProgramType)
	{
		throw fnd::Exception(kModuleName, "Illegal ProgramType. (range: 0-7 inclusive)");
	}

	mProgramType = type;
	updateCapField();
}
