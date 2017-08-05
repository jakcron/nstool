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

nx::MiscParamsEntry::MiscParamsEntry(u8 program_type) :
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

u8 nx::MiscParamsEntry::getProgramType() const
{
	return mProgramType;
}

void nx::MiscParamsEntry::setProgramType(u8 type)
{
	if (type > kMaxProgramType)
	{
		throw fnd::Exception(kModuleName, "Illegal ProgramType. (range: 0-7 inclusive)");
	}

	mProgramType = type;
	updateCapField();
}
