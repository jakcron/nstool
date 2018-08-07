#include <nx/KernelCapabilityEntry.h>

nx::KernelCapabilityEntry::KernelCapabilityEntry() :
	mType(kc::KC_INVALID)
{}

nx::KernelCapabilityEntry::KernelCapabilityEntry(kc::KernelCapId type) :
	mType(type),
	mField(0)
{}

nx::KernelCapabilityEntry::KernelCapabilityEntry(kc::KernelCapId type, uint32_t field) :
	mType(type),
	mField(field)
{}

void nx::KernelCapabilityEntry::operator=(const KernelCapabilityEntry & other)
{
	mType = other.mType;
	mField = other.mField;
}

bool nx::KernelCapabilityEntry::operator==(const KernelCapabilityEntry & other) const
{
	return (mType == other.mType) \
		&& (mField == other.mField);
}

bool nx::KernelCapabilityEntry::operator!=(const KernelCapabilityEntry & other) const
{
	return !operator==(other);
}

uint32_t nx::KernelCapabilityEntry::getCap() const
{
	return (mField & getFieldMask()) << getFieldShift() | getCapMask();
}

void nx::KernelCapabilityEntry::setCap(uint32_t cap)
{
	mType = getCapId(cap);
	mField = (cap >> getFieldShift()) & getFieldMask();
}

nx::kc::KernelCapId nx::KernelCapabilityEntry::getType() const
{
	return mType;
}

void nx::KernelCapabilityEntry::setType(kc::KernelCapId type)
{
	mType = type;
}

uint32_t nx::KernelCapabilityEntry::getField() const
{
	return mField & getFieldMask();
}

void nx::KernelCapabilityEntry::setField(uint32_t field)
{
	mField = field;
}
