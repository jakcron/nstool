#pragma once
#include <string>
#include <fnd/types.h>
#include <nn/hac/KernelCapabilityEntry.h>

namespace nn
{
namespace hac
{
	class HandleTableSizeEntry
	{
	public:
		HandleTableSizeEntry();
		HandleTableSizeEntry(const KernelCapabilityEntry& kernel_cap);
		HandleTableSizeEntry(uint16_t size);

		void operator=(const HandleTableSizeEntry& other);
		bool operator==(const HandleTableSizeEntry& other) const;
		bool operator!=(const HandleTableSizeEntry& other) const;

		// kernel capability
		const KernelCapabilityEntry& getKernelCapability() const;
		void setKernelCapability(const KernelCapabilityEntry& kernel_cap);

		// variables
		uint16_t getHandleTableSize() const;
		void setHandleTableSize(uint16_t size);
	private:
		const std::string kModuleName = "HANDLE_TABLE_SIZE_ENTRY";
		static const kc::KernelCapId kCapId = kc::KC_HANDLE_TABLE_SIZE;
		static const uint16_t kValBits = 10;
		static const uint16_t kMaxHandleTableSize = BIT(kValBits) - 1;

		KernelCapabilityEntry mCap;
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
}