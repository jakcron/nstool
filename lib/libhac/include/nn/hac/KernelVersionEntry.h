#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class KernelVersionEntry
	{
	public:
		KernelVersionEntry();
		KernelVersionEntry(const KernelCapabilityEntry& kernel_cap);
		KernelVersionEntry(uint16_t major, uint8_t minor);

		void operator=(const KernelVersionEntry& other);
		bool operator==(const KernelVersionEntry& other) const;
		bool operator!=(const KernelVersionEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		uint16_t getVerMajor() const;
		void setVerMajor(uint16_t major);
		uint8_t getVerMinor() const;
		void setVerMinor(uint8_t minor);
	private:
		const std::string kModuleName = "KERNEL_VERSION_ENTRY";
		static const kc::KernelCapId kCapId = kc::KC_KERNEL_VERSION;
		static const uint32_t kVerMajorBits = 13;
		static const uint32_t kVerMajorMax = BIT(kVerMajorBits) - 1;
		static const uint32_t kVerMinorBits = 4;
		static const uint32_t kVerMinorMax = BIT(kVerMinorBits) - 1;

		KernelCapabilityEntry mCap;
		uint16_t mVerMajor;
		uint8_t mVerMinor;

		inline void updateCapField()
		{
			uint32_t field = 0;
			field |= (uint32_t)(mVerMinor & kVerMinorMax) << 0;
			field |= (uint32_t)(mVerMajor & kVerMajorMax) << kVerMinorBits;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mVerMinor = (field >> 0) & kVerMinorMax;
			mVerMajor = (field >> kVerMinorBits) & kVerMajorMax;
		}
	};
}
}