#include <nx/KernelCapability.h>

nx::KernelCapability::KernelCapability() :
	mType(KC_INVALID)
{}

nx::KernelCapability::KernelCapability(KernelCapId type) :
	mType(type),
	mField(0)
{}

nx::KernelCapability::KernelCapability(KernelCapId type, uint32_t field) :
	mType(type),
	mField(field)
{}

void nx::KernelCapability::operator=(const KernelCapability & other)
{
	mType = other.mType;
	mField = other.mField;
}

bool nx::KernelCapability::operator==(const KernelCapability & other) const
{
	return (mType == other.mType) \
		&& (mField == other.mField);
}

bool nx::KernelCapability::operator!=(const KernelCapability & other) const
{
	return !operator==(other);
}

uint32_t nx::KernelCapability::getCap() const
{
	return (mField & getFieldMask()) << getFieldShift() | getCapMask();
}

void nx::KernelCapability::setCap(uint32_t cap)
{
	mType = getCapId(cap);
	mField = (cap >> getFieldShift()) & getFieldMask();
}

nx::KernelCapability::KernelCapId nx::KernelCapability::getType() const
{
	return mType;
}

void nx::KernelCapability::setType(KernelCapId type)
{
	mType = type;
}

uint32_t nx::KernelCapability::getField() const
{
	return mField & getFieldMask();
}

void nx::KernelCapability::setField(uint32_t field)
{
	mField = field;
}
