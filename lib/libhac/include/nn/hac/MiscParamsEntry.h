#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class MiscParamsEntry
	{
	public:
		MiscParamsEntry();
		MiscParamsEntry(const KernelCapabilityEntry& kernel_cap);
		MiscParamsEntry(byte_t program_type);

		void operator=(const MiscParamsEntry& other);
		bool operator==(const MiscParamsEntry& other) const;
		bool operator!=(const MiscParamsEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		byte_t getProgramType() const;
		void setProgramType(byte_t type);
	private:
		const std::string kModuleName = "MISC_PARAMS_ENTRY";
		static const kc::KernelCapId kCapId = kc::KC_MISC_PARAMS;
		static const byte_t kValBits = 3;
		static const byte_t kMaxProgramType = BIT(kValBits)-1;

		KernelCapabilityEntry mCap;
		byte_t mProgramType;

		inline void updateCapField()
		{
			uint32_t field = mProgramType & kMaxProgramType;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mProgramType = field & kMaxProgramType;
		}
	};
}
}