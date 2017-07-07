#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class MiscParamsEntry
	{
	public:
		MiscParamsEntry();
		MiscParamsEntry(const KernelCapability& kernel_cap);
		MiscParamsEntry(u8 program_type);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u8 getProgramType() const;
		void setProgramType(u8 type);
	private:
		const std::string kModuleName = "MISC_PARAMS_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_MISC_PARAMS;
		static const u8 kValBits = 3;
		static const u8 kMaxProgramType = BIT(kValBits)-1;

		KernelCapability mCap;
		u8 mProgramType;

		inline void updateCapField()
		{
			u32 field = mProgramType & kMaxProgramType;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mProgramType = field & kMaxProgramType;
		}
	};


}