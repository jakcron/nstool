#include <nn/ctr/CodeSegment.h>

nn::ctr::CodeSegment::CodeSegment() :
	mAddress(0),
	mSize(0),
	mPageNum(0)
{
}

nn::ctr::CodeSegment::CodeSegment(const CodeSegment& other) :
	mAddress(other.mAddress),
	mSize(other.mSize),
	mPageNum(other.mPageNum)
{}


nn::ctr::CodeSegment::CodeSegment(uint32_t address, uint32_t size, uint32_t page_num) :
	mAddress(address),
	mSize(size),
	mPageNum(page_num)
{}

void nn::ctr::CodeSegment::operator=(const CodeSegment& other)
{
	mAddress = other.mAddress;
	mSize = other.mSize;
	mPageNum = other.mPageNum;
}

bool nn::ctr::CodeSegment::operator==(const CodeSegment& other) const
{
	return (mAddress == other.mAddress) \
		&& (mSize == other.mSize) \
		&& (mPageNum == other.mPageNum);
}

bool nn::ctr::CodeSegment::operator!=(const CodeSegment& other) const
{
	return !(*this == other);
}

void nn::ctr::CodeSegment::setAddress(uint32_t address)
{
	mAddress = address;
}

uint32_t nn::ctr::CodeSegment::getAddress() const
{
	return mAddress;
}

void nn::ctr::CodeSegment::setSize(uint32_t size)
{
	mSize = size;
}

uint32_t nn::ctr::CodeSegment::getSize() const
{
	return mSize;
}

void nn::ctr::CodeSegment::setPageNum(uint32_t num)
{
	mPageNum = num;
}

uint32_t nn::ctr::CodeSegment::getPageNum() const
{
	return mPageNum;
}
