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
		KernelCapability(KernelCapId type, u32 field);

		const KernelCapability& operator=(const KernelCapability& other);
		bool operator==(const KernelCapability& other) const;
		bool operator!=(const KernelCapability& other) const;

		u32 getCap() const;
		void setCap(u32 cap);

		KernelCapId getType() const;
		void setType(KernelCapId type);

		u32 getField() const;
		void setField(u32 field);

	private:
		KernelCapId mType;
		u32 mField;

		inline u32 getFieldShift() const { return mType + 1; }
		inline u32 getFieldMask() const { return BIT(31 - mType) - 1; }
		inline u32 getCapMask() const { return BIT(mType) - 1; }
		inline KernelCapId getCapId(u32 cap) const
		{
			KernelCapId id = KC_INVALID;
			for (u8 tmp = 0; tmp < 31; tmp++)
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

