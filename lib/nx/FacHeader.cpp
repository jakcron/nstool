#include "FacHeader.h"

using namespace nx;

FacHeader::FacHeader()
{
	clearVariables();
}

FacHeader::FacHeader(const FacHeader & other)
{
	copyFrom(other);
}

FacHeader::FacHeader(const u8 * bytes)
{
	importBinary(bytes);
}

bool FacHeader::operator==(const FacHeader & other) const
{
	return isEqual(other);
}

bool FacHeader::operator!=(const FacHeader & other) const
{
	return !isEqual(other);
}

void FacHeader::operator=(const FacHeader & other)
{
	copyFrom(other);
}

const u8 * FacHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t FacHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void FacHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sFacHeader));
	sFacHeader* hdr = (sFacHeader*)mBinaryBlob.getBytes();

	hdr->set_version(kFacFormatVersion);
	hdr->set_fac_flags(mFsaRights);

	calculateOffsets();
	hdr->content_owner_ids().set_start(mContentOwnerIdPos.offset);
	hdr->content_owner_ids().set_end(mContentOwnerIdPos.offset + mContentOwnerIdPos.size);
	hdr->save_data_owner_ids().set_start(mSaveDataOwnerIdPos.offset);
	hdr->save_data_owner_ids().set_end(mSaveDataOwnerIdPos.offset + mSaveDataOwnerIdPos.size);
}

void FacHeader::importBinary(const u8 * bytes)
{
	mBinaryBlob.alloc(sizeof(sFacHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sFacHeader* hdr = (sFacHeader*)mBinaryBlob.getBytes();

	if (hdr->version() != kFacFormatVersion)
	{
		throw fnd::Exception(kModuleName, "Unsupported FAC format version");
	}

	mFsaRights = hdr->fac_flags();
	mContentOwnerIdPos.offset = hdr->content_owner_ids().start();
	mContentOwnerIdPos.size = hdr->content_owner_ids().end() - hdr->content_owner_ids().start();
	mSaveDataOwnerIdPos.offset = hdr->save_data_owner_ids().start();
	mSaveDataOwnerIdPos.size = hdr->save_data_owner_ids().end() - hdr->save_data_owner_ids().start();
}

void FacHeader::importBinary(const u8 * bytes, size_t len)
{
	if (len < sizeof(sFacHeader))
	{
		throw fnd::Exception(kModuleName, "FAC header too small");
	}
	importBinary(bytes);
}

u64 FacHeader::getFacSize() const
{
	return MAX(getSaveDataOwnerIdOffset() + getSaveDataOwnerIdSize(), getContentOwnerIdOffset() + getContentOwnerIdSize());
}

u64 FacHeader::getFsaRights() const
{
	return mFsaRights;
}

void FacHeader::setFsaRights(u64 flag)
{
	mFsaRights = flag;
}

size_t FacHeader::getContentOwnerIdOffset() const
{
	return mContentOwnerIdPos.offset;
}

size_t FacHeader::getContentOwnerIdSize() const
{
	return mContentOwnerIdPos.size;
}

void FacHeader::setContentOwnerIdSize(size_t size)
{
	mContentOwnerIdPos.size = size;
}

size_t FacHeader::getSaveDataOwnerIdOffset() const
{
	return mSaveDataOwnerIdPos.offset;
}

size_t FacHeader::getSaveDataOwnerIdSize() const
{
	return mSaveDataOwnerIdPos.size;
}

void FacHeader::setSaveDataOwnerIdSize(size_t size)
{
	mSaveDataOwnerIdPos.size = size;
}

void FacHeader::clearVariables()
{
	mFsaRights = 0;
	mContentOwnerIdPos.offset = 0;
	mContentOwnerIdPos.size = 0;
	mSaveDataOwnerIdPos.offset = 0;
	mSaveDataOwnerIdPos.size = 0;
}

void FacHeader::calculateOffsets()
{
	mContentOwnerIdPos.offset = align(sizeof(sFacHeader), 4);
	mSaveDataOwnerIdPos.offset = mContentOwnerIdPos.offset + align(mContentOwnerIdPos.size, 4);
}

bool FacHeader::isEqual(const FacHeader & other) const
{
	return (mFsaRights == other.mFsaRights) \
		&& (mContentOwnerIdPos.offset == other.mContentOwnerIdPos.offset) \
		&& (mContentOwnerIdPos.size == other.mContentOwnerIdPos.size) \
		&& (mSaveDataOwnerIdPos.offset == other.mSaveDataOwnerIdPos.offset) \
		&& (mSaveDataOwnerIdPos.size == other.mSaveDataOwnerIdPos.size);
}

void FacHeader::copyFrom(const FacHeader & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mBinaryBlob.clear();
		mFsaRights = other.mFsaRights;
		mContentOwnerIdPos.offset = other.mContentOwnerIdPos.offset;
		mContentOwnerIdPos.size = other.mContentOwnerIdPos.size;
		mSaveDataOwnerIdPos.offset = other.mSaveDataOwnerIdPos.offset;
		mSaveDataOwnerIdPos.size = other.mSaveDataOwnerIdPos.size;
	}
}
