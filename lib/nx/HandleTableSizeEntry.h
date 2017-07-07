#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class HandleTableSizeEntry
	{
	public:
		HandleTableSizeEntry();
		HandleTableSizeEntry(const KernelCapability& kernel_cap);
		HandleTableSizeEntry(u16 size);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		u16 getHandleTableSize() const;
		void setHandleTableSize(u16 size);
	private:
		const std::string kModuleName = "HANDLE_TABLE_SIZE_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_HANDLE_TABLE_SIZE;
		static const u16 kValBits = 10;
		static const u16 kMaxHandleTableSize = BIT(kValBits) - 1;

		KernelCapability mCap;
		u16 mHandleTableSize;

		inline void updateCapField()
		{
			u32 field = mHandleTableSize & kMaxHandleTableSize;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			u32 field = mCap.getField();
			mHandleTableSize = field & kMaxHandleTableSize;
		}
	};
}