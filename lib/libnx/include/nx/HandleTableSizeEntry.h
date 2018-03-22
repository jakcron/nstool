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
		HandleTableSizeEntry(uint16_t size);

		// kernel capability
		const KernelCapability& getKernelCapability() const;
		void setKernelCapability(const KernelCapability& kernel_cap);

		// variables
		uint16_t getHandleTableSize() const;
		void setHandleTableSize(uint16_t size);
	private:
		const std::string kModuleName = "HANDLE_TABLE_SIZE_ENTRY";
		static const KernelCapability::KernelCapId kCapId = KernelCapability::KC_HANDLE_TABLE_SIZE;
		static const uint16_t kValBits = 10;
		static const uint16_t kMaxHandleTableSize = BIT(kValBits) - 1;

		KernelCapability mCap;
		uint16_t mHandleTableSize;

		inline void updateCapField()
		{
			uint32_t field = mHandleTableSize & kMaxHandleTableSize;
			mCap.setField(field);
		}

		inline void processCapField()
		{
			uint32_t field = mCap.getField();
			mHandleTableSize = field & kMaxHandleTableSize;
		}
	};
}