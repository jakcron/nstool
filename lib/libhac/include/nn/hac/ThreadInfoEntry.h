#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class ThreadInfoEntry
	{
	public:
		ThreadInfoEntry();
		ThreadInfoEntry(const KernelCapabilityEntry& kernel_cap);
		ThreadInfoEntry(uint8_t min_priority, uint8_t max_priority, uint8_t min_cpu_id, uint8_t max_cpu_id);

		void operator=(const ThreadInfoEntry& other);
		bool operator==(const ThreadInfoEntry& other) const;
		bool operator!=(const ThreadInfoEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		uint8_t getMinPriority() const;
		void setMinPriority(uint8_t priority);
		uint8_t getMaxPriority() const;
		void setMaxPriority(uint8_t priority);
		uint8_t getMinCpuId() const;
		void setMinCpuId(uint8_t cpu_id);
		uint8_t getMaxCpuId() const;
		void setMaxCpuId(uint8_t cpu_id);

	private:
		const std::string kModuleName = "THREAD_INFO_ENTRY";
		static const kc::KernelCapId kCapId = kc::KC_THREAD_INFO;
		static const uint8_t kValBits = 6;
		static const uint8_t kMaxVal = BIT(kValBits)-1;
		static const uint8_t kDefaultPriority = 6;
		static const uint8_t kDefaultCpuId = 8;

		KernelCapabilityEntry mCap;
		uint8_t mMinPriority;
		uint8_t mMaxPriority;
		uint8_t mMinCpuId;
		uint8_t mMaxCpuId;

		inline void updateCapField()
		{
			uint32_t field = 0;
			field |= (uint32_t)(mMinPriority & kMaxVal) << (kValBits * 0);
			field |= (uint32_t)(mMaxPriority & kMaxVal) << (kValBits * 1);
			field |= (uint32_t)(mMinCpuId & kMaxVal) << (kValBits * 2);
			field |= (uint32_t)(mMaxCpuId & kMaxVal) << (kValBits * 3);
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mMinPriority = (field >> (kValBits * 0)) & kMaxVal;
			mMaxPriority = (field >> (kValBits * 1)) & kMaxVal;
			mMinCpuId = (field >> (kValBits * 2)) & kMaxVal;
			mMaxCpuId = (field >> (kValBits * 3)) & kMaxVal;
		}
	};
}
}