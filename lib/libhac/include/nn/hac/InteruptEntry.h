#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class InteruptEntry
	{
	public:
		static const uint32_t kInteruptBits = 10;
		static const uint32_t kInteruptMax = BIT(kInteruptBits) - 1;
		static const uint32_t kInteruptNum = 2;

		InteruptEntry();
		InteruptEntry(const KernelCapabilityEntry& kernel_cap);
		InteruptEntry(uint32_t interupt0, uint32_t interupt1);

		void operator=(const InteruptEntry& other);
		bool operator==(const InteruptEntry& other) const;
		bool operator!=(const InteruptEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		uint32_t operator[](size_t index) const;

		uint32_t getInterupt(size_t index) const;
		void setInterupt(size_t index, uint32_t interupt);

	private:
		const std::string kModuleName = "INTERUPT_ENTRY";
		static const kc::KernelCapId kCapId = kc::KC_ENABLE_INTERUPTS;

		KernelCapabilityEntry mCap;
		uint32_t mInterupt[kInteruptNum];

		inline void updateCapField()
		{
			uint32_t field = 0;
			field |= (uint32_t)(mInterupt[0] & kInteruptMax) << 0;
			field |= (uint32_t)(mInterupt[1] & kInteruptMax) << kInteruptBits;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mInterupt[0] = (field >> 0) & kInteruptMax;
			mInterupt[1] = (field >> kInteruptBits) & kInteruptMax;
		}
	};
}
}
