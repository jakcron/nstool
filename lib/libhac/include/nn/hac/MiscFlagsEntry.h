#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class MiscFlagsEntry
	{
	public:
		MiscFlagsEntry();
		MiscFlagsEntry(const KernelCapabilityEntry& kernel_cap);
		MiscFlagsEntry(uint32_t flags);

		void operator=(const MiscFlagsEntry& other);
		bool operator==(const MiscFlagsEntry& other) const;
		bool operator!=(const MiscFlagsEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		uint32_t getFlags() const;
		void setFlags(uint32_t flags);
	private:
		const std::string kModuleName = "MISC_FLAG_ENTRY";
		static const kc::KernelCapId kCapId = kc::KC_MISC_FLAGS;
		static const uint32_t kValueBits = 15;
		static const uint32_t kMaxVal = BIT(kValueBits)-1;

		KernelCapabilityEntry mCap;
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
}