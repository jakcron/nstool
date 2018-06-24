#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class MiscParamsEntry
	{
	public:
		MiscParamsEntry();
		MiscParamsEntry(const KernelCapability& kernel_cap);
		MiscParamsEntry(byte_t program_type);

		void operator=(const MiscParamsEntry& other);
		bool operator==(const MiscParamsEntry& other) const;
		bool operator!=(const MiscParamsEntry& other) const;

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		byte_t getProgramType() const;
		void setProgramType(byte_t type);
	private:
		const std::string kModuleName = "MISC_PARAMS_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_MISC_PARAMS;
		static const byte_t kValBits = 3;
		static const byte_t kMaxProgramType = BIT(kValBits)-1;

		KernelCapability mCap;
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