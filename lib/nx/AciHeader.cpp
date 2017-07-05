#include "AciHeader.h"

void AciHeader::clearVariables()
{
	mType = TYPE_ACI0;
	mProgramId = 0;
	mFac.offset = 0;
	mFac.size = 0;
	mSac.offset = 0;
	mSac.size = 0;
	mKc.offset = 0;
	mKc.size = 0;
}

void AciHeader::calculateSectionOffsets()
{
	mFac.offset = align(sizeof(sAciHeader), kAciAlignSize);
	mSac.offset = mFac.offset + align(mFac.size, kAciAlignSize);
	mKc.offset = mSac.offset + align(mSac.size, kAciAlignSize);
}

const u8 * AciHeader::getBytes() const
{
	return mBinaryBlob.data();
}

size_t AciHeader::getSize() const
{
	return mBinaryBlob.size();
}

void AciHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sAciHeader));
	sAciHeader* hdr = (sAciHeader*)mBinaryBlob.data();

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

	// set program
	hdr->set_program_id(mProgramId);

	// set offset/size
	calculateSectionOffsets();
	hdr->fac().set_offset(mFac.offset);
	hdr->fac().set_size(mFac.size);
	hdr->sac().set_offset(mSac.offset);
	hdr->sac().set_size(mSac.size);
	hdr->kc().set_offset(mKc.offset);
	hdr->kc().set_size(mKc.size);
}

void AciHeader::importBinary(const u8 * bytes)
{
	clearVariables();

	mBinaryBlob.alloc(sizeof(sAciHeader));
	memcpy(mBinaryBlob.data(), bytes, sizeof(sAciHeader));

	sAciHeader* hdr = (sAciHeader*)mBinaryBlob.data();

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

	mProgramId = hdr->program_id();
	mFac.offset = hdr->fac().offset();
	mFac.size = hdr->fac().size();
	mSac.offset = hdr->sac().offset();
	mSac.size = hdr->sac().size();
	mKc.offset = hdr->kc().offset();
	mKc.size = hdr->kc().size();
}

AciHeader::AciType AciHeader::getAciType() const
{
	return mType;
}

void AciHeader::setAciType(AciType type)
{
	mType = type;
}

u64 AciHeader::getProgramId() const
{
	return mProgramId;
}

void AciHeader::setProgramId(u64 program_id)
{
	mProgramId = program_id;
}

const AciHeader::sSection & AciHeader::getFileAccessControl() const
{
	return mFac;
}

void AciHeader::setFileAccessControl(u32 size)
{
	mFac.size = size;
}

const AciHeader::sSection & AciHeader::getServiceAccessControl() const
{
	return mSac;
}

void AciHeader::setServiceAccessControl(u32 size)
{
	mSac.size = size;
}

const AciHeader::sSection & AciHeader::getKernelCapabilities() const
{
	return mKc;
}

void AciHeader::setKernelCapabilities(u32 size)
{
	mKc.size = size;
}
