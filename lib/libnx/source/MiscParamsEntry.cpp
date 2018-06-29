#include <nx/MiscParamsEntry.h>

nx::MiscParamsEntry::MiscParamsEntry() :
	mCap(kCapId),
	mProgramType(0)
{}

nx::MiscParamsEntry::MiscParamsEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kCapId),
	mProgramType(0)
{
	setKernelCapability(kernel_cap);
}

nx::MiscParamsEntry::MiscParamsEntry(uint8_t program_type) :
	mCap(kCapId),
	mProgramType(0)
{
	setProgramType(program_type);
}

void nx::MiscParamsEntry::operator=(const MiscParamsEntry& other)
{
	mProgramType = other.mProgramType;
	updateCapField();
}

bool nx::MiscParamsEntry::operator==(const MiscParamsEntry& other) const
{
	return (mProgramType == other.mProgramType);
}

bool nx::MiscParamsEntry::operator!=(const MiscParamsEntry& other) const
{
	return !(*this == other);
}

const nx::KernelCapabilityEntry & nx::MiscParamsEntry::getKernelCapability() const
{
	return mCap;
}

void nx::MiscParamsEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'ThreadInfo'");
	}

	mCap = kernel_cap;
	processCapField();
}

uint8_t nx::MiscParamsEntry::getProgramType() const
{
	return mProgramType;
}

void nx::MiscParamsEntry::setProgramType(uint8_t type)
{
	if (type > kMaxProgramType)
	{
		throw fnd::Exception(kModuleName, "Illegal ProgramType. (range: 0-7 inclusive)");
	}

	mProgramType = type;
	updateCapField();
}
