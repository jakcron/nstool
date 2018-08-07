#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class MemoryPageEntry
	{
	public:
		MemoryPageEntry();
		MemoryPageEntry(const KernelCapabilityEntry& kernel_cap);
		MemoryPageEntry(uint32_t page);
		MemoryPageEntry(uint32_t page, bool flag);

		void operator=(const MemoryPageEntry& other);
		bool operator==(const MemoryPageEntry& other) const;
		bool operator!=(const MemoryPageEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		uint32_t getPage() const;
		void setPage(uint32_t page);
		bool getFlag() const;
		void setFlag(bool flag);

		bool isMultiplePages() const;
		void setMapMultiplePages(bool useFlag);
	private:
		const std::string kModuleName = "MEMORY_PAGE_ENTRY";
		static const uint32_t kPageBits = 24;
		static const uint32_t kMaxPage = BIT(kPageBits) - 1;

		KernelCapabilityEntry mCap;
		uint32_t mPage;
		bool mFlag;
		bool mUseFlag;

		inline void updateCapField()
		{
			uint32_t field = 0;
			field |= (uint32_t)(mPage & kMaxPage) << 0;
			field |= (uint32_t)(mFlag) << kPageBits;
			mCap.setField(field);
			mCap.setType(mUseFlag ? kc::KC_MEMORY_MAP : kc::KC_IO_MEMORY_MAP);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mPage = (field >> 0) & kMaxPage;
			mFlag = (field >> kPageBits);
			mUseFlag = mCap.getType() == kc::KC_MEMORY_MAP;
		}
	};
}
}