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
		MiscFlagsEntry(uint32_t flags);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		uint32_t getFlags() const;
		void setFlags(uint32_t flags);
	private:
		const std::string kModuleName = "MISC_FLAG_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_MISC_FLAGS;
		static const uint32_t kValueBits = 15;
		static const uint32_t kMaxVal = BIT(kValueBits)-1;

		KernelCapability mCap;
		uint32_t mFlags;

		inline void updateCapField()
		{
			uint32_t field = mFlags & kMaxVal;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mFlags = field & kMaxVal;
		}
	};

}
