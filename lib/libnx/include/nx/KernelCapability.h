#pragma once
#include <fnd/types.h>

namespace nx
{
	class KernelCapability
	{
	public:
		enum KernelCapId
		{
			KC_INVALID = 0,
			KC_THREAD_INFO = 3,
			KC_ENABLE_SYSTEM_CALLS = 4,
			KC_MEMORY_MAP = 6,
			KC_IO_MEMORY_MAP = 7,
			KC_ENABLE_INTERUPTS = 11,
			KC_MISC_PARAMS = 13,
			KC_KERNEL_VERSION = 14,
			KC_HANDLE_TABLE_SIZE = 15,
			KC_MISC_FLAGS = 16
		};

		KernelCapability();
		KernelCapability(KernelCapId type);
		KernelCapability(KernelCapId type, uint32_t field);

		const KernelCapability& operator=(const KernelCapability& other);
		bool operator==(const KernelCapability& other) const;
		bool operator!=(const KernelCapability& other) const;

		uint32_t getCap() const;
		void setCap(uint32_t cap);

		KernelCapId getType() const;
		void setType(KernelCapId type);

		uint32_t getField() const;
		void setField(uint32_t field);

	private:
		KernelCapId mType;
		uint32_t mField;

		inline uint32_t getFieldShift() const { return mType + 1; }
		inline uint32_t getFieldMask() const { return BIT(31 - mType) - 1; }
		inline uint32_t getCapMask() const { return BIT(mType) - 1; }
		inline KernelCapId getCapId(uint32_t cap) const
		{
			KernelCapId id = KC_INVALID;
			for (byte_t tmp = 0; tmp < 31; tmp++)
			{
				if (((cap >> tmp) & 1) == 0)
				{
					id = (KernelCapId)tmp;
					break;
				}
			}
			return id;
		}
	};
}

