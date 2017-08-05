#include <nx/AciHeader.h>

using namespace nx;

void AciHeader::calculateSectionOffsets()
{
	mFac.offset = align(mHeaderOffset, kAciAlignSize) + align(sizeof(sAciHeader), kAciAlignSize);
	mSac.offset = mFac.offset + align(mFac.size, kAciAlignSize);
	mKc.offset = mSac.offset + align(mSac.size, kAciAlignSize);
}

bool AciHeader::isEqual(const AciHeader & other) const
{
	return (mHeaderOffset == other.mHeaderOffset) \
		&& (mType == other.mType) \
		&& (mAcidSize == other.mAcidSize) \
		&& (mProgramId == other.mProgramId) \
		&& (mFac == other.mFac) \
		&& (mSac == other.mSac) \
		&& (mKc == other.mKc);
}

void AciHeader::copyFrom(const AciHeader & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mHeaderOffset = other.mHeaderOffset;
		mType = other.mType;
		mAcidSize = other.mAcidSize;
		mProgramId = other.mProgramId;
		mFac = other.mFac;
		mSac = other.mSac;
		mKc = other.mKc;
	}
}

AciHeader::AciHeader()
{
	clear();
}

AciHeader::AciHeader(const AciHeader & other)
{
	importBinary(other.getBytes(), other.getSize());
}

AciHeader::AciHeader(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool AciHeader::operator==(const AciHeader & other) const
{
	return isEqual(other);
}

bool AciHeader::operator!=(const AciHeader & other) const
{
	return !isEqual(other);
}

void AciHeader::operator=(const AciHeader & other)
{
	this->importBinary(other.getBytes(), other.getSize());
}

const u8 * AciHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t AciHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void AciHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sAciHeader));
	sAciHeader* hdr = (sAciHeader*)mBinaryBlob.getBytes();

	// set type
	switch (mType)
	{
	case (TYPE_ACI0):
		hdr->set_signature(kAciStructSig.c_str());
		break;
	case (TYPE_ACID):
		hdr->set_signature(kAciDescStructSig.c_str());
		break;
	default:
		throw fnd::Exception(kModuleName, "Unexpected ACI type");
	}

	// set offset/size
	calculateSectionOffsets();
	hdr->fac().set_offset(mFac.offset);
	hdr->fac().set_size(mFac.size);
	hdr->sac().set_offset(mSac.offset);
	hdr->sac().set_size(mSac.size);
	hdr->kc().set_offset(mKc.offset);
	hdr->kc().set_size(mKc.size);

	u32 flags = 0;
	if (mIsProduction)
		flags |= BIT(0);

	hdr->set_flags(flags);

	if (mType == TYPE_ACI0)
	{
		// set program
		hdr->set_program_id(mProgramId);
	}
	else if (mType == TYPE_ACID)
	{
		mAcidSize = getAciSize();
		hdr->set_size(mAcidSize);
		hdr->set_program_id_min(mProgramIdMin);
		hdr->set_program_id_max(mProgramIdMax);	
	}
}

void AciHeader::importBinary(const u8 * bytes, size_t len)
{
	if (len < sizeof(sAciHeader))
	{
		throw fnd::Exception(kModuleName, "ACI header too small");
	}
	
	clear();

	mBinaryBlob.alloc(sizeof(sAciHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, sizeof(sAciHeader));

	sAciHeader* hdr = (sAciHeader*)mBinaryBlob.getBytes();

	if (memcmp(hdr->signature(), kAciStructSig.c_str(), 4) == 0)
	{
		mType = TYPE_ACI0;
	}
	else if (memcmp(hdr->signature(), kAciDescStructSig.c_str(), 4) == 0)
	{
		mType = TYPE_ACID;
	}
	else
	{
		throw fnd::Exception(kModuleName, "ACI header corrupt");
	}

	
	if (mType == TYPE_ACI0)
	{
		mProgramId = hdr->program_id();
		mIsProduction = false;
		mAcidSize = 0;
		mProgramIdMin = 0;
		mProgramIdMax = 0;
	}
	else if (mType == TYPE_ACID)
	{
		mProgramId = 0;
		mIsProduction = (hdr->flags() & BIT(0)) == BIT(0);
		mAcidSize = hdr->size();
		mProgramIdMin = hdr->program_id_min();
		mProgramIdMax = hdr->program_id_max();
	}
	
	// the header offset is the MIN(sac.offset, fac.offset, kc.offset) - sizeof(sHeader)
	mHeaderOffset = MAX(MIN(hdr->sac().offset(), MIN(hdr->fac().offset(), hdr->kc().offset())), align(sizeof(sAciHeader), kAciAlignSize)) - align(sizeof(sAciHeader), kAciAlignSize);

	mFac.offset = hdr->fac().offset() - mHeaderOffset;
	mFac.size = hdr->fac().size();
	mSac.offset = hdr->sac().offset() - mHeaderOffset;
	mSac.size = hdr->sac().size();
	mKc.offset = hdr->kc().offset() - mHeaderOffset;
	mKc.size = hdr->kc().size();
}

void nx::AciHeader::clear()
{
	mBinaryBlob.clear();
	mHeaderOffset = 0;
	mType = TYPE_ACI0;
	mProgramId = 0;
	mProgramIdMin = 0;
	mProgramIdMax = 0;
	mAcidSize = 0;
	mIsProduction = false;
	mFac.offset = 0;
	mFac.size = 0;
	mSac.offset = 0;
	mSac.size = 0;
	mKc.offset = 0;
	mKc.size = 0;
}

size_t nx::AciHeader::getAciSize() const
{
	return MAX(MAX(MAX(mSac.offset + mSac.size, mKc.offset + mKc.size), mFac.offset + mFac.size), sizeof(sAciHeader));
}

size_t nx::AciHeader::getAcidSize() const
{
	return mAcidSize;
}

/*
void nx::AciHeader::setAcidSize(size_t size)
{
	mAcidSize = size;
}
*/

u64 nx::AciHeader::getProgramIdMin() const
{
	return mProgramIdMin;
}

void nx::AciHeader::setProgramIdMin(u64 program_id)
{
	mProgramIdMin = program_id;
}

u64 nx::AciHeader::getProgramIdMax() const
{
	return mProgramIdMax;
}

void nx::AciHeader::setProgramIdMax(u64 program_id)
{
	mProgramIdMax = program_id;
}

void nx::AciHeader::setHeaderOffset(size_t offset)
{
	mHeaderOffset = offset;
}

AciHeader::AciType AciHeader::getAciType() const
{
	return mType;
}

void AciHeader::setAciType(AciType type)
{
	mType = type;
}

bool nx::AciHeader::isProduction() const
{
	return mIsProduction;
}

void nx::AciHeader::setIsProduction(bool isProduction)
{
	mIsProduction = isProduction;
}

u64 AciHeader::getProgramId() const
{
	return mProgramId;
}

void AciHeader::setProgramId(u64 program_id)
{
	mProgramId = program_id;
}

const AciHeader::sSection & AciHeader::getFacPos() const
{
	return mFac;
}

void AciHeader::setFacSize(size_t size)
{
	mFac.size = size;
}

const AciHeader::sSection & AciHeader::getSacPos() const
{
	return mSac;
}

void AciHeader::setSacSize(size_t size)
{
	mSac.size = size;
}

const AciHeader::sSection & AciHeader::getKcPos() const
{
	return mKc;
}

void AciHeader::setKcSize(size_t size)
{
	mKc.size = size;
}
