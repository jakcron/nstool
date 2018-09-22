#include <nn/ctr/CodeBinaryHeader.h>
#include <cstring>

nn::ctr::CodeBinaryHeader::CodeBinaryHeader() :
	mRawBinary(),
	mCodeBinSize(0),
	mStackSize(0),
	mBssSize(0),
	mSegmentList()
{
}

nn::ctr::CodeBinaryHeader::CodeBinaryHeader(const CodeBinaryHeader& other) :
	CodeBinaryHeader()
{
	*this = other;
}

void nn::ctr::CodeBinaryHeader::operator=(const CodeBinaryHeader& other)
{
	clear();
	mCodeBinOffset = other.mCodeBinOffset;
	mCodeBinSize = other.mCodeBinSize;
	mStackSize = other.mStackSize;
	mBssSize = other.mBssSize;
	mSegmentList[code::TEXT_SEGMENT] = other.mSegmentList[code::TEXT_SEGMENT];
	mSegmentList[code::RODATA_SEGMENT] = other.mSegmentList[code::RODATA_SEGMENT];
	mSegmentList[code::DATA_SEGMENT] = other.mSegmentList[code::DATA_SEGMENT];
}

bool nn::ctr::CodeBinaryHeader::operator==(const CodeBinaryHeader& other) const
{
	return (mCodeBinOffset == other.mCodeBinOffset) \
		&& (mCodeBinSize == other.mCodeBinSize) \
		&& (mStackSize == other.mStackSize) \
		&& (mBssSize == other.mBssSize) \
		&& (mSegmentList[code::TEXT_SEGMENT] == other.mSegmentList[code::TEXT_SEGMENT]) \
		&& (mSegmentList[code::RODATA_SEGMENT] == other.mSegmentList[code::RODATA_SEGMENT]) \
		&& (mSegmentList[code::DATA_SEGMENT] == other.mSegmentList[code::DATA_SEGMENT]);
}

bool nn::ctr::CodeBinaryHeader::operator!=(const CodeBinaryHeader& other) const
{
	return !(*this == other);
}

void nn::ctr::CodeBinaryHeader::toBytes()
{
	// allocate memory for header
	mRawBinary.clear();
	mRawBinary.alloc(sizeof(sCodeBinaryHeader));

	// get ptr to header struct
	sCodeBinaryHeader* hdr = (sCodeBinaryHeader*)mRawBinary.data();

	// write variables
	hdr->st_magic = code::kStructMagic;
	hdr->code_bin_size = le_word(mCodeBinSize);
	hdr->stack_size = le_word(mStackSize);
	hdr->bss_size = le_word(mBssSize);
	for (size_t i = 0; i < code::kCodeSegmentNum; i++)
	{
		hdr->segments[i].v_addr = le_word(mSegmentList[i].getAddress());
		hdr->segments[i].size = le_word(mSegmentList[i].getSize());
		hdr->segments[i].page_num = le_word(mSegmentList[i].getPageNum());
	}
}

void nn::ctr::CodeBinaryHeader::fromBytes(const byte_t* data, size_t len)
{
	// check size
	if (len < sizeof(sCodeBinaryHeader))
	{
		throw fnd::Exception(kModuleName, "CODE header corrupt (binary is too small)");
	}

	// clear variables
	clear();

	// save a copy of the header
	sCodeBinaryHeader hdr;
	memcpy((void*)&hdr, data, sizeof(sCodeBinaryHeader));

	// check magic
	if (hdr.st_magic.get() != code::kStructMagic)
	{
		throw fnd::Exception(kModuleName, "CODE header corrupt (unrecognised header signature)");
	}

	if (hdr.format_ident.get() != code::kCtrFormatIdent)
	{
		throw fnd::Exception(kModuleName, "CODE header corrupt (unsupported format version)");
	}

	// allocate memory for header
	mRawBinary.alloc(sizeof(sCodeBinaryHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// save variables
	mCodeBinOffset = hdr.code_bin_offset.get();
	mCodeBinSize = hdr.code_bin_size.get();
	mStackSize = hdr.stack_size.get();
	mBssSize = hdr.bss_size.get();
	for (size_t i = 0; i < code::kCodeSegmentNum; i++)
	{
		mSegmentList[i].setAddress(hdr.segments[i].v_addr.get());
		mSegmentList[i].setSize(hdr.segments[i].size.get());
		mSegmentList[i].setPageNum(hdr.segments[i].page_num.get());
	}
}

const fnd::Vec<byte_t>& nn::ctr::CodeBinaryHeader::getBytes() const
{
	return mRawBinary;
}


void nn::ctr::CodeBinaryHeader::clear()
{
	mRawBinary.clear();
	mCodeBinOffset = 0;
	mCodeBinSize = 0;
	mStackSize = 0;
	mBssSize = 0;
	for (size_t i = 0; i < code::kCodeSegmentNum; i++)
	{
		mSegmentList[i].setAddress(0);
		mSegmentList[i].setSize(0);
		mSegmentList[i].setPageNum(0);
	}
}

void nn::ctr::CodeBinaryHeader::setCodeBinOffset(uint32_t offset)
{
	mCodeBinOffset = offset;
}

uint32_t nn::ctr::CodeBinaryHeader::getCodeBinOffset() const
{
	return mCodeBinOffset;
}

void nn::ctr::CodeBinaryHeader::setCodeBinSize(uint32_t size)
{
	mCodeBinSize = size;
}

uint32_t nn::ctr::CodeBinaryHeader::getCodeBinSize() const
{
	return mCodeBinSize;
}

void nn::ctr::CodeBinaryHeader::setStackSize(uint32_t size)
{
	mStackSize = size;
}

uint32_t nn::ctr::CodeBinaryHeader::getStackSize() const
{
	return mStackSize;
}

void nn::ctr::CodeBinaryHeader::setBssSize(uint32_t size)
{
	mBssSize = size;
}

uint32_t nn::ctr::CodeBinaryHeader::getBssSize() const
{
	return mBssSize;
}

void nn::ctr::CodeBinaryHeader::setTextSegment(const nn::ctr::CodeSegment & segment)
{
	mSegmentList[code::TEXT_SEGMENT] = segment;
}

const nn::ctr::CodeSegment & nn::ctr::CodeBinaryHeader::getTextSegment() const
{
	return mSegmentList[code::TEXT_SEGMENT];
}

void nn::ctr::CodeBinaryHeader::setRODataSegment(const nn::ctr::CodeSegment & segment)
{
	mSegmentList[code::RODATA_SEGMENT] = segment;
}

const nn::ctr::CodeSegment & nn::ctr::CodeBinaryHeader::getRODataSegment() const
{
	return mSegmentList[code::RODATA_SEGMENT];
}

void nn::ctr::CodeBinaryHeader::setDataSegment(const nn::ctr::CodeSegment & segment)
{
	mSegmentList[code::DATA_SEGMENT] = segment;
}

const nn::ctr::CodeSegment & nn::ctr::CodeBinaryHeader::getDataSegment() const
{
	return mSegmentList[code::DATA_SEGMENT];
}
