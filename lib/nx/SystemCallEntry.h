#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class SystemCallEntry
	{
	public:
		SystemCallEntry();
		SystemCallEntry(const KernelCapability& kernel_cap);
		SystemCallEntry(u32 upper_bits, u32 lower_bits);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u32 getSystemCallUpperBits() const;
		void setSystemCallUpperBits(u32 upper_bits);
		u32 getSystemCallLowerBits() const;
		void setSystemCallLowerBits(u32 lower_bits);
		
	private:
		const std::string kModuleName = "SYSTEM_CALL_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_ENABLE_SYSTEM_CALLS;
		static const u32 kSysCallUpperBits = 3;
		static const u32 kSysCallLowerBits = 23;
		static const u32 kSysCallUpperMax = BIT(kSysCallUpperBits) - 1;
		static const u32 kSysCallLowerMax = BIT(kSysCallLowerBits) - 1;

		KernelCapability mCap;
		u32 mSystemCallUpper;
		u32 mSystemCallLower;

		inline void updateCapField()
		{
			u32 field = 0;
			field |= (u32)(mSystemCallLower & kSysCallLowerMax) << 0;
			field |= (u32)(mSystemCallUpper & kSysCallUpperMax) << kSysCallLowerBits;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mSystemCallLower = (field >> 0) & kSysCallLowerMax;
			mSystemCallUpper = (field >> kSysCallLowerBits) & kSysCallUpperMax;
		}
	};
}

