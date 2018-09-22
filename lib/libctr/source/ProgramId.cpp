#include <nn/ctr/ProgramId.h>

nn::ctr::ProgramId::ProgramId() :
	mProgramId(0)
{}

nn::ctr::ProgramId::ProgramId(const ProgramId & other) :
	mProgramId(other.getInnerValue())
{}

nn::ctr::ProgramId::ProgramId(uint64_t prog_id) :
	mProgramId(prog_id)
{}

nn::ctr::ProgramId::ProgramId(uint16_t device_group, uint16_t category, uint32_t unique_id, uint8_t variation) :
	mProgramId(0)
{
	setDeviceGroup(device_group);
	setCategory(category);
	setUniqueId(unique_id);
	setVariation(variation);
}

void nn::ctr::ProgramId::operator=(const ProgramId& other)
{
	mProgramId = other.mProgramId;
}

bool nn::ctr::ProgramId::operator==(const ProgramId& other) const
{
	return mProgramId == other.mProgramId;
}

bool nn::ctr::ProgramId::operator!=(const ProgramId& other) const
{
	return !(*this == other);
}

void nn::ctr::ProgramId::setInnerValue(uint64_t id)
{
	mProgramId = id;
}

uint64_t nn::ctr::ProgramId::getInnerValue() const
{
	return mProgramId;
}

void nn::ctr::ProgramId::setDeviceGroup(uint16_t device_group)
{
	mProgramId &= ~prog_id::kDeviceGroupMask;
	mProgramId |= ((uint64_t)device_group) << prog_id::kDeviceGroupShift;
}

uint16_t nn::ctr::ProgramId::getDeviceGroup() const
{
	return (mProgramId & prog_id::kDeviceGroupMask) >> prog_id::kDeviceGroupShift;
}

void nn::ctr::ProgramId::setCategory(uint16_t category)
{
	mProgramId &= ~prog_id::kCategoryMask;
	mProgramId |= ((uint64_t)category) << prog_id::kCategoryShift;
}

uint16_t nn::ctr::ProgramId::getCategory() const
{
	return (mProgramId & prog_id::kCategoryMask) >> prog_id::kCategoryShift;
}

void nn::ctr::ProgramId::setUniqueId(uint32_t uid)
{
	mProgramId &= ~prog_id::kUniqueIdMask;
	mProgramId |= ((uint64_t)uid) << prog_id::kUniqueIdShift;
}

uint32_t nn::ctr::ProgramId::getUniqueId() const
{
	return (mProgramId & prog_id::kUniqueIdMask) >> prog_id::kUniqueIdShift;
}

void nn::ctr::ProgramId::setVariation(uint8_t variation)
{
	mProgramId &= ~prog_id::kVariationMask;
	mProgramId |= (uint64_t)variation;
}

uint8_t nn::ctr::ProgramId::getVariation() const
{
	return mProgramId & prog_id::kVariationMask;
}
