#pragma once
#include <fnd/types.h>
#include <nn/hac/kc.h>

namespace nn
{
namespace hac
{
	class KernelCapabilityEntry
	{
	public:

		KernelCapabilityEntry();
		KernelCapabilityEntry(kc::KernelCapId type);
		KernelCapabilityEntry(kc::KernelCapId type, uint32_t field);

		void operator=(const KernelCapabilityEntry& other);
		bool operator==(const KernelCapabilityEntry& other) const;
		bool operator!=(const KernelCapabilityEntry& other) const;

		uint32_t getCap() const;
		void setCap(uint32_t cap);

		kc::KernelCapId getType() const;
		void setType(kc::KernelCapId type);

		uint32_t getField() const;
		void setField(uint32_t field);

	private:
		kc::KernelCapId mType;
		uint32_t mField;

		inline uint32_t getFieldShift() const { return mType + 1; }
		inline uint32_t getFieldMask() const { return BIT(31 - mType) - 1; }
		inline uint32_t getCapMask() const { return BIT(mType) - 1; }
		inline kc::KernelCapId getCapId(uint32_t cap) const
		{
			kc::KernelCapId id = kc::KC_INVALID;
			for (byte_t tmp = 0; tmp < 31; tmp++)
			{
				if (((cap >> tmp) & 1) == 0)
				{
					id = (kc::KernelCapId)tmp;
					break;
				}
			}
			return id;
		}
	};
}
}