#include <es/SectionHeader_V2.h>



es::SectionHeader_V2::SectionHeader_V2()
{}

es::SectionHeader_V2::SectionHeader_V2(const SectionHeader_V2 & other)
{
	*this = other;
}

void es::SectionHeader_V2::operator=(const SectionHeader_V2 & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		mRawBinary.clear();
		mSectionOffset = other.mSectionOffset;
		mRecordSize = other.mRecordSize;
		mSectionSize = other.mSectionSize;
		mRecordNum = other.mRecordNum;
		mSectionType = other.mSectionType;
	}
}

bool es::SectionHeader_V2::operator==(const SectionHeader_V2 & other) const
{
	return (mSectionOffset == other.mSectionOffset) \
		&& (mRecordSize == other.mRecordSize) \
		&& (mSectionSize == other.mSectionSize) \
		&& (mRecordNum == other.mRecordNum) \
		&& (mSectionType == other.mSectionType);
}

bool es::SectionHeader_V2::operator!=(const SectionHeader_V2 & other) const
{
	return !(*this ==other);
}

void es::SectionHeader_V2::toBytes()
{
	mRawBinary.alloc(sizeof(sSectionHeader_v2));
	sSectionHeader_v2* hdr = (sSectionHeader_v2*)mRawBinary.data();

	hdr->section_offset = (mSectionOffset);
	hdr->record_size = (mRecordSize);
	hdr->section_size = (mSectionSize);
	hdr->record_num = (mRecordNum);
	hdr->section_type = (mSectionType);
}

void es::SectionHeader_V2::fromBytes(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sSectionHeader_v2))
	{
		throw fnd::Exception(kModuleName, "Binary too small");
	}

	clear();

	mRawBinary.alloc(sizeof(sSectionHeader_v2));
	memcpy(mRawBinary.data(), bytes, mRawBinary.size());
	sSectionHeader_v2* hdr = (sSectionHeader_v2*)mRawBinary.data();

	mSectionOffset = hdr->section_offset.get();
	mRecordSize = hdr->record_size.get();
	mSectionSize = hdr->section_size.get();
	mRecordNum = hdr->record_num.get();
	mSectionType = (ticket::SectionType)hdr->section_type.get();
}

const fnd::Vec<byte_t>& es::SectionHeader_V2::getBytes() const
{
	return mRawBinary;
}

void es::SectionHeader_V2::clear()
{
	mRawBinary.clear();
	mSectionOffset = 0;
	mRecordSize = 0;
	mSectionSize = 0;
	mRecordNum = 0;
	mSectionType = ticket::SECTION_PERMANENT;
}

uint32_t es::SectionHeader_V2::getSectionOffset() const
{
	return mSectionOffset;
}

void es::SectionHeader_V2::setSectionOffset(uint32_t offset)
{
	mSectionOffset = offset;
}

uint32_t es::SectionHeader_V2::getRecordSize() const
{
	return mRecordSize;
}

void es::SectionHeader_V2::setRecordSize(uint32_t size)
{
	mRecordSize = size;
}

uint32_t es::SectionHeader_V2::getSectionSize() const
{
	return mSectionSize;
}

void es::SectionHeader_V2::getSectionSize(uint32_t size)
{
	mSectionSize = size;
}

uint16_t es::SectionHeader_V2::getRecordNum() const
{
	return mRecordNum;
}

void es::SectionHeader_V2::setRecordNum(uint16_t record_num)
{
	mRecordNum = record_num;
}

es::ticket::SectionType es::SectionHeader_V2::getSectionType() const
{
	return mSectionType;
}

void es::SectionHeader_V2::setSectionType(ticket::SectionType type)
{
	mSectionType = type;
}


