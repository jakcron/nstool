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
		SystemCallEntry(uint32_t upper_bits, uint32_t lower_bits);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		uint32_t getSystemCallUpperBits() const;
		void setSystemCallUpperBits(uint32_t upper_bits);
		uint32_t getSystemCallLowerBits() const;
		void setSystemCallLowerBits(uint32_t lower_bits);
		
	private:
		const std::string kModuleName = "SYSTEM_CALL_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_ENABLE_SYSTEM_CALLS;
		static const uint32_t kSysCallUpperBits = 3;
		static const uint32_t kSysCallLowerBits = 24;
		static const uint32_t kSysCallUpperMax = BIT(kSysCallUpperBits) - 1;
		static const uint32_t kSysCallLowerMax = BIT(kSysCallLowerBits) - 1;

		KernelCapability mCap;
		uint32_t mSystemCallUpper;
		uint32_t mSystemCallLower;

		inline void updateCapField()
		{
			uint32_t field = 0;
			field |= (uint32_t)(mSystemCallLower & kSysCallLowerMax) << 0;
			field |= (uint32_t)(mSystemCallUpper & kSysCallUpperMax) << kSysCallLowerBits;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mSystemCallLower = (field >> 0) & kSysCallLowerMax;
			mSystemCallUpper = (field >> kSysCallLowerBits) & kSysCallUpperMax;
		}
	};
}

