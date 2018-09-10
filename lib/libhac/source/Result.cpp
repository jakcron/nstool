#include <nn/hac/Result.h>

nn::hac::Result::Result() :
	mResult(0)
{}

nn::hac::Result::Result(uint32_t result) :
	mResult(result)
{}

nn::hac::Result::Result(uint32_t module_num, uint32_t desc, uint32_t sub_desc) :
	mResult(0)
{
	mResult |= (module_num & bitWidthToMask(kModuleNumBitWidth)) << kModuleNumBitPos;
	mResult |= (desc & bitWidthToMask(kDescriptionBitWidth)) << kDescriptionBitPos;
	mResult |= (sub_desc & bitWidthToMask(kSubDescriptionBitWidth)) << kSubDescriptionBitPos;
}

void nn::hac::Result::operator=(const Result & other)
{
	mResult = other.mResult;
}

bool nn::hac::Result::operator==(const Result & other) const
{
	return mResult == other.mResult;
}

bool nn::hac::Result::operator!=(const Result & other) const
{
	return !(*this == other);
}

bool nn::hac::Result::isSuccess() const
{
	return mResult == 0;
}

bool nn::hac::Result::isFailure() const
{
	return !isSuccess();
}

uint32_t nn::hac::Result::getInnerValue() const
{
	return mResult;
}

uint32_t nn::hac::Result::getModuleNum() const
{
	return (mResult >> kModuleNumBitPos) & bitWidthToMask(kModuleNumBitWidth);
}

uint32_t nn::hac::Result::getDescription() const
{
	return (mResult >> kDescriptionBitPos) & bitWidthToMask(kDescriptionBitWidth);
}

uint32_t nn::hac::Result::getSubDescription() const
{
	return (mResult >> kSubDescriptionBitPos) & bitWidthToMask(kSubDescriptionBitWidth);
}
