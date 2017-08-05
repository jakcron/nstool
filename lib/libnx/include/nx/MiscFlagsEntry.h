#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class MiscFlagsEntry
	{
	public:
		MiscFlagsEntry();
		MiscFlagsEntry(const KernelCapability& kernel_cap);
		MiscFlagsEntry(u32 flags);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u32 getFlags() const;
		void setFlags(u32 flags);
	private:
		const std::string kModuleName = "MISC_FLAG_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_MISC_FLAGS;
		static const u32 kValueBits = 15;
		static const u32 kMaxVal = BIT(kValueBits)-1;

		KernelCapability mCap;
		u32 mFlags;

		inline void updateCapField()
		{
			u32 field = mFlags & kMaxVal;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mFlags = field & kMaxVal;
		}
	};

}
