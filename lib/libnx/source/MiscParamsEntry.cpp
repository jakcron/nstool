#include <nx/MiscParamsEntry.h>



nx::MiscParamsEntry::MiscParamsEntry() :
	mCap(kCapId),
	mProgramType(0)
{}

nx::MiscParamsEntry::MiscParamsEntry(const KernelCapability & kernel_cap) :
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

const nx::KernelCapability & nx::MiscParamsEntry::getKernelCapability() const
{
	return mCap;
}

void nx::MiscParamsEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != kCapId)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'ThreadInfo'");
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
