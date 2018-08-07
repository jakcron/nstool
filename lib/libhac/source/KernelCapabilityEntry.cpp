#include <nn/hac/KernelCapabilityEntry.h>

nn::hac::KernelCapabilityEntry::KernelCapabilityEntry() :
	mType(kc::KC_INVALID)
{}

nn::hac::KernelCapabilityEntry::KernelCapabilityEntry(kc::KernelCapId type) :
	mType(type),
	mField(0)
{}

nn::hac::KernelCapabilityEntry::KernelCapabilityEntry(kc::KernelCapId type, uint32_t field) :
	mType(type),
	mField(field)
{}

void nn::hac::KernelCapabilityEntry::operator=(const KernelCapabilityEntry & other)
{
	mType = other.mType;
	mField = other.mField;
}

bool nn::hac::KernelCapabilityEntry::operator==(const KernelCapabilityEntry & other) const
{
	return (mType == other.mType) \
		&& (mField == other.mField);
}

bool nn::hac::KernelCapabilityEntry::operator!=(const KernelCapabilityEntry & other) const
{
	return !operator==(other);
}

uint32_t nn::hac::KernelCapabilityEntry::getCap() const
{
	return (mField & getFieldMask()) << getFieldShift() | getCapMask();
}

void nn::hac::KernelCapabilityEntry::setCap(uint32_t cap)
{
	mType = getCapId(cap);
	mField = (cap >> getFieldShift()) & getFieldMask();
}

nn::hac::kc::KernelCapId nn::hac::KernelCapabilityEntry::getType() const
{
	return mType;
}

void nn::hac::KernelCapabilityEntry::setType(kc::KernelCapId type)
{
	mType = type;
}

uint32_t nn::hac::KernelCapabilityEntry::getField() const
{
	return mField & getFieldMask();
}

void nn::hac::KernelCapabilityEntry::setField(uint32_t field)
{
	mField = field;
}
