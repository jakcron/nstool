#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class ThreadInfoEntry
	{
	public:
		ThreadInfoEntry();
		ThreadInfoEntry(const KernelCapability& kernel_cap);
		ThreadInfoEntry(u8 min_priority, u8 max_priority, u8 min_core_number, u8 max_core_number);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u8 getMinPriority() const;
		void setMinPriority(u8 priority);
		u8 getMaxPriority() const;
		void setMaxPriority(u8 priority);
		u8 getMinCoreNumber() const;
		void setMinCoreNumber(u8 core_num);
		u8 getMaxCoreNumber() const;
		void setMaxCoreNumber(u8 core_num);

	private:
		const std::string kModuleName = "THREAD_INFO_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_THREAD_INFO;
		static const u8 kValBits = 6;
		static const u8 kMaxVal = BIT(kValBits)-1;
		static const u8 kDefaultPriority = 6;
		static const u8 kDefaultCoreNumber = 8;

		KernelCapability mCap;
		u8 mMinPriority;
		u8 mMaxPriority;
		u8 mMinCoreNumber;
		u8 mMaxCoreNumber;

		inline void updateCapField()
		{
			u32 field = 0;
			field |= (u32)(mMinPriority & kMaxVal) << (kValBits * 0);
			field |= (u32)(mMaxPriority & kMaxVal) << (kValBits * 1);
			field |= (u32)(mMinCoreNumber & kMaxVal) << (kValBits * 2);
			field |= (u32)(mMaxCoreNumber & kMaxVal) << (kValBits * 3);
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mMinPriority = (field >> (kValBits * 0)) & kMaxVal;
			mMaxPriority = (field >> (kValBits * 1)) & kMaxVal;
			mMinCoreNumber = (field >> (kValBits * 2)) & kMaxVal;
			mMaxCoreNumber = (field >> (kValBits * 3)) & kMaxVal;
		}
	};
}
