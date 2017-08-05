#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class KernelVersionEntry
	{
	public:
		KernelVersionEntry();
		KernelVersionEntry(const KernelCapability& kernel_cap);
		KernelVersionEntry(u16 major, u8 minor);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u16 getVerMajor() const;
		void setVerMajor(u16 major);
		u8 getVerMinor() const;
		void setVerMinor(u8 minor);
	private:
		const std::string kModuleName = "KERNEL_VERSION_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_KERNEL_VERSION;
		static const u32 kVerMajorBits = 13;
		static const u32 kVerMajorMax = BIT(kVerMajorBits) - 1;
		static const u32 kVerMinorBits = 4;
		static const u32 kVerMinorMax = BIT(kVerMinorBits) - 1;

		KernelCapability mCap;
		u16 mVerMajor;
		u8 mVerMinor;

		inline void updateCapField()
		{
			u32 field = 0;
			field |= (u32)(mVerMinor & kVerMinorMax) << 0;
			field |= (u32)(mVerMajor & kVerMajorMax) << kVerMinorBits;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mVerMinor = (field >> 0) & kVerMinorMax;
			mVerMajor = (field >> kVerMinorBits) & kVerMajorMax;
		}
	};
}

