#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class MemoryPageEntry
	{
	public:
		MemoryPageEntry();
		MemoryPageEntry(const KernelCapability& kernel_cap);
		MemoryPageEntry(u32 page);
		MemoryPageEntry(u32 page, bool flag);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u32 getPage() const;
		void setPage(u32 page);
		bool getFlag() const;
		void setFlag(bool flag);

		bool hasFlag() const;
		void useFlag(bool useFlag);
	private:
		const std::string kModuleName = "MEMORY_PAGE_ENTRY";
		static const u32 kPageBits = 24;
		static const u32 kMaxPage = BIT(kPageBits) - 1;

		KernelCapability mCap;
		u32 mPage;
		bool mFlag;
		bool mUseFlag;

		inline void updateCapField()
		{
			u32 field = 0;
			field |= (u32)(mPage & kMaxPage) << 0;
			field |= (u32)(mFlag) << kPageBits;
			mCap.setField(field);
			mCap.setType(mUseFlag ? KernelCapability::KC_MEMORY_MAP : KernelCapability::KC_IO_MEMORY_MAP);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mPage = (field >> 0) & kMaxPage;
			mFlag = (field >> kPageBits);
			mUseFlag = mCap.getType() == KernelCapability::KC_MEMORY_MAP;
		}
	};

}
