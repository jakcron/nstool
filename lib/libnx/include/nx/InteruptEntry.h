#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class InteruptEntry
	{
	public:
		static const u32 kInteruptBits = 10;
		static const u32 kInteruptMax = BIT(kInteruptBits) - 1;
		static const u32 kInteruptNum = 2;

		InteruptEntry();
		InteruptEntry(const KernelCapability& kernel_cap);
		InteruptEntry(u32 interupt0, u32 interupt1);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u32 operator[](size_t index) const;

		u32 getInterupt(size_t index) const;
		void setInterupt(size_t index, u32 interupt);

	private:
		const std::string kModuleName = "INTERUPT_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_ENABLE_INTERUPTS;

		KernelCapability mCap;
		u32 mInterupt[kInteruptNum];

		inline void updateCapField()
		{
			u32 field = 0;
			field |= (u32)(mInterupt[0] & kInteruptMax) << 0;
			field |= (u32)(mInterupt[1] & kInteruptMax) << kInteruptBits;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mInterupt[0] = (field >> 0) & kInteruptMax;
			mInterupt[1] = (field >> kInteruptBits) & kInteruptMax;
		}
	};

}
