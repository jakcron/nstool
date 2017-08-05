#include <nx/KernelCapability.h>

using namespace nx;

KernelCapability::KernelCapability() :
	mType(KC_INVALID)
{}

KernelCapability::KernelCapability(KernelCapId type) :
	mType(type),
	mField(0)
{}

KernelCapability::KernelCapability(KernelCapId type, u32 field) :
	mType(type),
	mField(field)
{}

const KernelCapability & nx::KernelCapability::operator=(const KernelCapability & other)
{
	mType = other.mType;
	mField = other.mField;
	return *this;
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

u32 KernelCapability::getCap() const
{
	return (mField & getFieldMask()) << getFieldShift() | getCapMask();
}

void KernelCapability::setCap(u32 cap)
{
	mType = getCapId(cap);
	mField = (cap >> getFieldShift()) & getFieldMask();
}

KernelCapability::KernelCapId KernelCapability::getType() const
{
	return mType;
}

void KernelCapability::setType(KernelCapId type)
{
	mType = type;
}

u32 KernelCapability::getField() const
{
	return mField & getFieldMask();
}

void KernelCapability::setField(u32 field)
{
	mField = field;
}
