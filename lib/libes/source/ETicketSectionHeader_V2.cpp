#include <es/ETicketSectionHeader_V2.h>



es::ETicketSectionHeader_V2::ETicketSectionHeader_V2()
{}

es::ETicketSectionHeader_V2::ETicketSectionHeader_V2(const ETicketSectionHeader_V2 & other)
{
	copyFrom(other);
}

es::ETicketSectionHeader_V2::ETicketSectionHeader_V2(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool es::ETicketSectionHeader_V2::operator==(const ETicketSectionHeader_V2 & other) const
{
	return isEqual(other);
}

bool es::ETicketSectionHeader_V2::operator!=(const ETicketSectionHeader_V2 & other) const
{
	return !isEqual(other);
}

void es::ETicketSectionHeader_V2::operator=(const ETicketSectionHeader_V2 & other)
{
	copyFrom(other);
}

const byte_t * es::ETicketSectionHeader_V2::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t es::ETicketSectionHeader_V2::getSize() const
{
	return mBinaryBlob.getSize();
}

void es::ETicketSectionHeader_V2::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sSectionHeader_v2));
	sSectionHeader_v2* hdr = (sSectionHeader_v2*)mBinaryBlob.getBytes();

	hdr->set_section_offset(mSectionOffset);
	hdr->set_record_size(mRecordSize);
	hdr->set_section_size(mSectionSize);
	hdr->set_record_num(mRecordNum);
	hdr->set_section_type(mSectionType);
}

void es::ETicketSectionHeader_V2::importBinary(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sSectionHeader_v2))
	{
		throw fnd::Exception(kModuleName, "Binary too small");
	}

	clear();

	mBinaryBlob.alloc(sizeof(sSectionHeader_v2));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sSectionHeader_v2* hdr = (sSectionHeader_v2*)mBinaryBlob.getBytes();

	mSectionOffset = hdr->section_offset();
	mRecordSize = hdr->record_size();
	mSectionSize = hdr->section_size();
	mRecordNum = hdr->record_num();
	mSectionType = (SectionType)hdr->section_type();
}

bool es::ETicketSectionHeader_V2::isEqual(const ETicketSectionHeader_V2 & other) const
{
	return (mSectionOffset == other.mSectionOffset) \
		&& (mRecordSize == other.mRecordSize) \
		&& (mSectionSize == other.mSectionSize) \
		&& (mRecordNum == other.mRecordNum) \
		&& (mSectionType == other.mSectionType);
}

void es::ETicketSectionHeader_V2::copyFrom(const ETicketSectionHeader_V2 & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mBinaryBlob.clear();
		mSectionOffset = other.mSectionOffset;
		mRecordSize = other.mRecordSize;
		mSectionSize = other.mSectionSize;
		mRecordNum = other.mRecordNum;
		mSectionType = other.mSectionType;
	}
}

void es::ETicketSectionHeader_V2::clear()
{
	mBinaryBlob.clear();
	mSectionOffset = 0;
	mRecordSize = 0;
	mSectionSize = 0;
	mRecordNum = 0;
	mSectionType = PERMANENT;
}

uint32_t es::ETicketSectionHeader_V2::getSectionOffset() const
{
	return mSectionOffset;
}

void es::ETicketSectionHeader_V2::setSectionOffset(uint32_t offset)
{
	mSectionOffset = offset;
}

uint32_t es::ETicketSectionHeader_V2::getRecordSize() const
{
	return mRecordSize;
}

void es::ETicketSectionHeader_V2::setRecordSize(uint32_t size)
{
	mRecordSize = size;
}

uint32_t es::ETicketSectionHeader_V2::getSectionSize() const
{
	return mSectionSize;
}

void es::ETicketSectionHeader_V2::getSectionSize(uint32_t size)
{
	mSectionSize = size;
}

uint16_t es::ETicketSectionHeader_V2::getRecordNum() const
{
	return mRecordNum;
}

void es::ETicketSectionHeader_V2::setRecordNum(uint16_t record_num)
{
	mRecordNum = record_num;
}

es::ETicketSectionHeader_V2::SectionType es::ETicketSectionHeader_V2::getSectionType() const
{
	return mSectionType;
}

void es::ETicketSectionHeader_V2::setSectionType(SectionType type)
{
	mSectionType = type;
}


