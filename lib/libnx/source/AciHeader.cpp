#include <nx/AciHeader.h>

using namespace nx;

AciHeader::AciHeader()
{
	clear();
}

AciHeader::AciHeader(const AciHeader & other)
{
	*this = other;
}

void AciHeader::operator=(const AciHeader & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		mHeaderOffset = other.mHeaderOffset;
		mType = other.mType;
		mIsProduction = other.mIsProduction;
		mIsUnqualifiedApproval = other.mIsUnqualifiedApproval;
		mAcidSize = other.mAcidSize;
		mProgramIdMin = other.mProgramIdMin;
		mProgramIdMax = other.mProgramIdMax;
		mProgramId = other.mProgramId;
		mFac = other.mFac;
		mSac = other.mSac;
		mKc = other.mKc;
	}
}

bool AciHeader::operator==(const AciHeader & other) const
{
	return (mHeaderOffset == other.mHeaderOffset) \
		&& (mType == other.mType) \
		&& (mIsProduction == other.mIsProduction) \
		&& (mIsUnqualifiedApproval == other.mIsUnqualifiedApproval) \
		&& (mAcidSize == other.mAcidSize) \
		&& (mProgramIdMin == other.mProgramIdMin) \
		&& (mProgramIdMax == other.mProgramIdMax) \
		&& (mProgramId == other.mProgramId) \
		&& (mFac == other.mFac) \
		&& (mSac == other.mSac) \
		&& (mKc == other.mKc);
}

bool AciHeader::operator!=(const AciHeader & other) const
{
	return !(*this == other);
}

void AciHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sAciHeader));
	sAciHeader* hdr = (sAciHeader*)mRawBinary.data();

	// set type
	switch (mType)
	{
	case (TYPE_ACI0):
		hdr->signature = aci::kAciStructSig;
		break;
	case (TYPE_ACID):
		hdr->signature = aci::kAciDescStructSig;
		break;
	default:
		throw fnd::Exception(kModuleName, "Unexpected ACI type");
	}

	// set offset/size
	calculateSectionOffsets();
	hdr->fac.offset = (uint32_t)mFac.offset;
	hdr->fac.size = (uint32_t)mFac.size;
	hdr->sac.offset = (uint32_t)mSac.offset;
	hdr->sac.size = (uint32_t)mSac.size;
	hdr->kc.offset = (uint32_t)mKc.offset;
	hdr->kc.size = (uint32_t)mKc.size;

	uint32_t flags = 0;
	if (mIsProduction)
		flags |= _BIT(aci::FLAG_PRODUCTION);
	if (mIsUnqualifiedApproval)
		flags |= _BIT(aci::FLAG_UNQUALIFIED_APPROVAL);

	hdr->flags = flags;

	if (mType == TYPE_ACI0)
	{
		// set program
		hdr->program_id_info.program_id = mProgramId;
	}
	else if (mType == TYPE_ACID)
	{
		mAcidSize = getAciSize();
		hdr->size = (uint32_t)mAcidSize;
		hdr->program_id_info.program_id_restrict.min = mProgramIdMin;
		hdr->program_id_info.program_id_restrict.max = mProgramIdMax;
	}
}

void AciHeader::fromBytes(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sAciHeader))
	{
		throw fnd::Exception(kModuleName, "ACI header too small");
	}
	
	clear();

	mRawBinary.alloc(sizeof(sAciHeader));
	memcpy(mRawBinary.data(), bytes, sizeof(sAciHeader));

	sAciHeader* hdr = (sAciHeader*)mRawBinary.data();

	switch (hdr->signature.get())
	{
	case (aci::kAciStructSig):
		mType = TYPE_ACI0;
		break;
	case (aci::kAciDescStructSig):
		mType = TYPE_ACID;
		break;
	default:
		throw fnd::Exception(kModuleName, "ACI header corrupt");
	}

	
	if (mType == TYPE_ACI0)
	{
		mProgramId = hdr->program_id_info.program_id.get();
		mIsProduction = false;
		mIsUnqualifiedApproval = false;
		mAcidSize = 0;
		mProgramIdMin = 0;
		mProgramIdMax = 0;
	}
	else if (mType == TYPE_ACID)
	{
		mProgramId = 0;
		mIsProduction = _HAS_BIT(hdr->flags.get(), aci::FLAG_PRODUCTION);
		mIsUnqualifiedApproval = _HAS_BIT(hdr->flags.get(), aci::FLAG_UNQUALIFIED_APPROVAL);
		mAcidSize = hdr->size.get();
		mProgramIdMin = hdr->program_id_info.program_id_restrict.min.get();
		mProgramIdMax = hdr->program_id_info.program_id_restrict.max.get();
	}
	
	// the header offset is the MIN(sac.offset, fac.offset, kc.offset) - sizeof(sHeader)
	mHeaderOffset = MAX(MIN(hdr->sac.offset.get(), MIN(hdr->fac.offset.get(), hdr->kc.offset.get())), align(sizeof(sAciHeader), aci::kAciAlignSize)) - align(sizeof(sAciHeader), aci::kAciAlignSize);

	mFac.offset = hdr->fac.offset.get() - mHeaderOffset;
	mFac.size = hdr->fac.size.get();
	mSac.offset = hdr->sac.offset.get() - mHeaderOffset;
	mSac.size = hdr->sac.size.get();
	mKc.offset = hdr->kc.offset.get() - mHeaderOffset;
	mKc.size = hdr->kc.size.get();
}

const fnd::Vec<byte_t>& AciHeader::getBytes() const
{
	return mRawBinary;
}

void nx::AciHeader::clear()
{
	mRawBinary.clear();
	mHeaderOffset = 0;
	mType = TYPE_ACI0;
	mProgramId = 0;
	mProgramIdMin = 0;
	mProgramIdMax = 0;
	mAcidSize = 0;
	mIsProduction = false;
	mIsUnqualifiedApproval = false;
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

uint64_t nx::AciHeader::getProgramIdMin() const
{
	return mProgramIdMin;
}

void nx::AciHeader::setProgramIdMin(uint64_t program_id)
{
	mProgramIdMin = program_id;
}

uint64_t nx::AciHeader::getProgramIdMax() const
{
	return mProgramIdMax;
}

void nx::AciHeader::setProgramIdMax(uint64_t program_id)
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

bool nx::AciHeader::isUnqualifiedApproval() const
{
	return mIsUnqualifiedApproval;
}

void nx::AciHeader::setIsUnqualifiedApproval(bool isUnqualifiedApproval)
{
	mIsUnqualifiedApproval = isUnqualifiedApproval;
}

uint64_t AciHeader::getProgramId() const
{
	return mProgramId;
}

void AciHeader::setProgramId(uint64_t program_id)
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

void AciHeader::calculateSectionOffsets()
{
	mFac.offset = align(mHeaderOffset, aci::kAciAlignSize) + align(sizeof(sAciHeader), aci::kAciAlignSize);
	mSac.offset = mFac.offset + align(mFac.size, aci::kAciAlignSize);
	mKc.offset = mSac.offset + align(mSac.size, aci::kAciAlignSize);
}