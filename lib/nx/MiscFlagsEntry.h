#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class MiscFlagsEntry
	{
	public:
		// consider different definition location, perhaps in MiscFlagsHandler?
		enum Flags
		{
			FLAG_ENABLE_DEBUG = BIT(0),
			FLAG_FORCE_DEBUG = BIT(1),
			FLAG_UNK02 = BIT(2),
			FLAG_UNK03 = BIT(3),
			FLAG_UNK04 = BIT(4),
			FLAG_UNK05 = BIT(5),
			FLAG_UNK06 = BIT(6),
			FLAG_UNK07 = BIT(7),
			FLAG_UNK08 = BIT(8),
			FLAG_UNK09 = BIT(9),
			FLAG_UNK10 = BIT(10),
			FLAG_UNK11 = BIT(11),
			FLAG_UNK12 = BIT(12),
			FLAG_UNK13 = BIT(13),
			FLAG_UNK14 = BIT(14),
		};

		MiscFlagsEntry();
		MiscFlagsEntry(const KernelCapability& kernel_cap);
		MiscFlagsEntry(u32 flags);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u32 getFlags() const;
		void setFlags(u32 flags);
	private:
		const std::string kModuleName = "MISC_FLAG_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_MISC_FLAGS;
		static const u32 kValueBits = 15;
		static const u32 kMaxVal = BIT(kValueBits)-1;

		KernelCapability mCap;
		u32 mFlags;

		inline void updateCapField()
		{
			u32 field = mFlags & kMaxVal;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mFlags = field & kMaxVal;
		}
	};

}
