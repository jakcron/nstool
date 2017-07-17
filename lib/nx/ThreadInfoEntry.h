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
		ThreadInfoEntry(u8 min_priority, u8 max_priority, u8 min_cpu_id, u8 max_cpu_id);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u8 getMinPriority() const;
		void setMinPriority(u8 priority);
		u8 getMaxPriority() const;
		void setMaxPriority(u8 priority);
		u8 getMinCpuId() const;
		void setMinCpuId(u8 cpu_id);
		u8 getMaxCpuId() const;
		void setMaxCpuId(u8 cpu_id);

	private:
		const std::string kModuleName = "THREAD_INFO_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_THREAD_INFO;
		static const u8 kValBits = 6;
		static const u8 kMaxVal = BIT(kValBits)-1;
		static const u8 kDefaultPriority = 6;
		static const u8 kDefaultCpuId = 8;

		KernelCapability mCap;
		u8 mMinPriority;
		u8 mMaxPriority;
		u8 mMinCpuId;
		u8 mMaxCpuId;

		inline void updateCapField()
		{
			u32 field = 0;
			field |= (u32)(mMinPriority & kMaxVal) << (kValBits * 0);
			field |= (u32)(mMaxPriority & kMaxVal) << (kValBits * 1);
			field |= (u32)(mMinCpuId & kMaxVal) << (kValBits * 2);
			field |= (u32)(mMaxCpuId & kMaxVal) << (kValBits * 3);
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mMinPriority = (field >> (kValBits * 0)) & kMaxVal;
			mMaxPriority = (field >> (kValBits * 1)) & kMaxVal;
			mMinCpuId = (field >> (kValBits * 2)) & kMaxVal;
			mMaxCpuId = (field >> (kValBits * 3)) & kMaxVal;
		}
	};
}
