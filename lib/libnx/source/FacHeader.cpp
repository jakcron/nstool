#include <nx/FacHeader.h>



nx::FacHeader::FacHeader() :
	mFsaRights()
{
	clear();
}

nx::FacHeader::FacHeader(const FacHeader & other) :
	mFsaRights()
{
	copyFrom(other);
}

nx::FacHeader::FacHeader(const byte_t * bytes, size_t len) :
	mFsaRights()
{
	importBinary(bytes, len);
}

bool nx::FacHeader::operator==(const FacHeader & other) const
{
	return isEqual(other);
}

bool nx::FacHeader::operator!=(const FacHeader & other) const
{
	return !isEqual(other);
}

void nx::FacHeader::operator=(const FacHeader & other)
{
	copyFrom(other);
}

const byte_t * nx::FacHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::FacHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::FacHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sFacHeader));
	sFacHeader* hdr = (sFacHeader*)mBinaryBlob.getBytes();

	if (mVersion != kFacFormatVersion)
	{
		fnd::Exception(kModuleName, "Unsupported format version");
	}
	hdr->version = (mVersion);

	uint64_t flag = 0;
	for (size_t i = 0; i < mFsaRights.getSize(); i++)
	{
		flag |= BIT((uint64_t)mFsaRights[i]);
	}
	hdr->fac_flags = (flag);

	calculateOffsets();
	hdr->content_owner_ids.start = (mContentOwnerIdPos.offset);
	hdr->content_owner_ids.end = (mContentOwnerIdPos.offset + mContentOwnerIdPos.size);
	hdr->save_data_owner_ids.start = (mSaveDataOwnerIdPos.offset);
	hdr->save_data_owner_ids.end = (mSaveDataOwnerIdPos.offset + mSaveDataOwnerIdPos.size);
}

void nx::FacHeader::importBinary(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sFacHeader))
	{
		throw fnd::Exception(kModuleName, "FAC header too small");
	}
	
	mBinaryBlob.alloc(sizeof(sFacHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sFacHeader* hdr = (sFacHeader*)mBinaryBlob.getBytes();

	if (hdr->version.get() != kFacFormatVersion)
	{
		throw fnd::Exception(kModuleName, "Unsupported FAC format version");
	}
	mVersion = hdr->version.get();

	clear();
	for (uint64_t i = 0; i < 64; i++)
	{
		if ((hdr->fac_flags.get() >> i) & 1)
		{
			mFsaRights.addElement((FsAccessFlag)i);
		}
	}
	mContentOwnerIdPos.offset = hdr->content_owner_ids.start.get();
	mContentOwnerIdPos.size = hdr->content_owner_ids.end.get() > hdr->content_owner_ids.start.get() ? hdr->content_owner_ids.end.get() - hdr->content_owner_ids.start.get() : 0;
	mSaveDataOwnerIdPos.offset = hdr->save_data_owner_ids.start.get();
	mSaveDataOwnerIdPos.size = hdr->save_data_owner_ids.end.get() > hdr->save_data_owner_ids.start.get() ? hdr->save_data_owner_ids.end.get() - hdr->save_data_owner_ids.start.get() : 0;
}

void nx::FacHeader::clear()
{
	mFsaRights.clear();
	mContentOwnerIdPos.offset = 0;
	mContentOwnerIdPos.size = 0;
	mSaveDataOwnerIdPos.offset = 0;
	mSaveDataOwnerIdPos.size = 0;
}

size_t nx::FacHeader::getFacSize() const
{
	size_t savedata = getSaveDataOwnerIdOffset() + getSaveDataOwnerIdSize();
	size_t content = getContentOwnerIdOffset() + getContentOwnerIdSize();
	return MAX(MAX(savedata, content), sizeof(sFacHeader));
}

uint32_t nx::FacHeader::getFormatVersion() const
{
	return mVersion;
}

void nx::FacHeader::setFormatVersion(uint32_t version)
{
	mVersion = version;
}

const fnd::List<nx::FacHeader::FsAccessFlag>& nx::FacHeader::getFsaRightsList() const
{
	return mFsaRights;
}

void nx::FacHeader::setFsaRightsList(const fnd::List<FsAccessFlag>& list)
{
	mFsaRights.clear();
	for (size_t i = 0; i < list.getSize(); i++)
	{
		mFsaRights.hasElement(list[i]) ? mFsaRights.addElement(list[i]) : throw fnd::Exception(kModuleName, "FSA right already exists");
	}
}

size_t nx::FacHeader::getContentOwnerIdOffset() const
{
	return mContentOwnerIdPos.offset;
}

size_t nx::FacHeader::getContentOwnerIdSize() const
{
	return mContentOwnerIdPos.size;
}

void nx::FacHeader::setContentOwnerIdSize(size_t size)
{
	mContentOwnerIdPos.size = size;
}

size_t nx::FacHeader::getSaveDataOwnerIdOffset() const
{
	return mSaveDataOwnerIdPos.offset;
}

size_t nx::FacHeader::getSaveDataOwnerIdSize() const
{
	return mSaveDataOwnerIdPos.size;
}

void nx::FacHeader::setSaveDataOwnerIdSize(size_t size)
{
	mSaveDataOwnerIdPos.size = size;
}

void nx::FacHeader::calculateOffsets()
{
	mContentOwnerIdPos.offset = align(sizeof(sFacHeader), 4);
	mSaveDataOwnerIdPos.offset = mContentOwnerIdPos.offset + align(mContentOwnerIdPos.size, 4);
}

bool nx::FacHeader::isEqual(const FacHeader & other) const
{
	return (mFsaRights == other.mFsaRights) \
		&& (mContentOwnerIdPos.offset == other.mContentOwnerIdPos.offset) \
		&& (mContentOwnerIdPos.size == other.mContentOwnerIdPos.size) \
		&& (mSaveDataOwnerIdPos.offset == other.mSaveDataOwnerIdPos.offset) \
		&& (mSaveDataOwnerIdPos.size == other.mSaveDataOwnerIdPos.size);
}

void nx::FacHeader::copyFrom(const FacHeader & other)
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
