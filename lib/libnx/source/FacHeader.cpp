#include <nx/FacHeader.h>

nx::FacHeader::FacHeader() :
	mFsaRights()
{
	clear();
}

nx::FacHeader::FacHeader(const FacHeader & other) :
	mFsaRights()
{
	*this = other;
}

void nx::FacHeader::operator=(const FacHeader & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mFsaRights = other.mFsaRights;
		mContentOwnerIdPos.offset = other.mContentOwnerIdPos.offset;
		mContentOwnerIdPos.size = other.mContentOwnerIdPos.size;
		mSaveDataOwnerIdPos.offset = other.mSaveDataOwnerIdPos.offset;
		mSaveDataOwnerIdPos.size = other.mSaveDataOwnerIdPos.size;
	}
}

bool nx::FacHeader::operator==(const FacHeader & other) const
{
	return (mFsaRights == other.mFsaRights) \
		&& (mContentOwnerIdPos.offset == other.mContentOwnerIdPos.offset) \
		&& (mContentOwnerIdPos.size == other.mContentOwnerIdPos.size) \
		&& (mSaveDataOwnerIdPos.offset == other.mSaveDataOwnerIdPos.offset) \
		&& (mSaveDataOwnerIdPos.size == other.mSaveDataOwnerIdPos.size);
}

bool nx::FacHeader::operator!=(const FacHeader & other) const
{
	return !(*this == other);
}

void nx::FacHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sFacHeader));
	sFacHeader* hdr = (sFacHeader*)mRawBinary.data();

	if (mVersion != fac::kFacFormatVersion)
	{
		fnd::Exception(kModuleName, "Unsupported format version");
	}
	hdr->version = (mVersion);

	uint64_t flag = 0;
	for (size_t i = 0; i < mFsaRights.size(); i++)
	{
		flag |= _BIT((uint64_t)mFsaRights[i]);
	}
	hdr->fac_flags = (flag);

	hdr->content_owner_ids.start = (uint32_t)(mContentOwnerIdPos.offset);
	hdr->content_owner_ids.end = (uint32_t)(mContentOwnerIdPos.offset + mContentOwnerIdPos.size);
	hdr->save_data_owner_ids.start = (uint32_t)(mSaveDataOwnerIdPos.offset);
	hdr->save_data_owner_ids.end = (uint32_t)(mSaveDataOwnerIdPos.offset + mSaveDataOwnerIdPos.size);
}

void nx::FacHeader::fromBytes(const byte_t* data, size_t len)
{
	if (len < sizeof(sFacHeader))
	{
		throw fnd::Exception(kModuleName, "FAC header too small");
	}
	
	// clear internal members
	clear();

	mRawBinary.alloc(sizeof(sFacHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());
	sFacHeader* hdr = (sFacHeader*)mRawBinary.data();

	if (hdr->version.get() != fac::kFacFormatVersion)
	{
		throw fnd::Exception(kModuleName, "Unsupported FAC format version");
	}
	mVersion = hdr->version.get();

	for (uint64_t i = 0; i < 64; i++)
	{
		if (_HAS_BIT(hdr->fac_flags.get(), i))
		{
			mFsaRights.addElement((fac::FsAccessFlag)i);
		}
	}
	mContentOwnerIdPos.offset = hdr->content_owner_ids.start.get();
	mContentOwnerIdPos.size = hdr->content_owner_ids.end.get() > hdr->content_owner_ids.start.get() ? hdr->content_owner_ids.end.get() - hdr->content_owner_ids.start.get() : 0;
	mSaveDataOwnerIdPos.offset = hdr->save_data_owner_ids.start.get();
	mSaveDataOwnerIdPos.size = hdr->save_data_owner_ids.end.get() > hdr->save_data_owner_ids.start.get() ? hdr->save_data_owner_ids.end.get() - hdr->save_data_owner_ids.start.get() : 0;
}

const fnd::Vec<byte_t>& nx::FacHeader::getBytes() const
{
	return mRawBinary;
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
	size_t savedata = mSaveDataOwnerIdPos.offset + mSaveDataOwnerIdPos.size;
	size_t content = mContentOwnerIdPos.offset + mContentOwnerIdPos.size;
	return _MAX(_MAX(savedata, content), sizeof(sFacHeader));
}

uint32_t nx::FacHeader::getFormatVersion() const
{
	return mVersion;
}

void nx::FacHeader::setFormatVersion(uint32_t version)
{
	mVersion = version;
}

const fnd::List<nx::fac::FsAccessFlag>& nx::FacHeader::getFsaRightsList() const
{
	return mFsaRights;
}

void nx::FacHeader::setFsaRightsList(const fnd::List<fac::FsAccessFlag>& list)
{
	mFsaRights.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		mFsaRights.hasElement(list[i]) ? mFsaRights.addElement(list[i]) : throw fnd::Exception(kModuleName, "FSA right already exists");
	}
}

const nx::FacHeader::sSection& nx::FacHeader::getContentOwnerIdPos() const
{
	return mContentOwnerIdPos;
}

void nx::FacHeader::setContentOwnerIdPos(const sSection& pos)
{
	mContentOwnerIdPos = pos;
}

const nx::FacHeader::sSection& nx::FacHeader::getSaveDataOwnerIdPos() const
{
	return mSaveDataOwnerIdPos;
}

void nx::FacHeader::setSaveDataOwnerIdPos(const sSection& pos)
{
	mSaveDataOwnerIdPos = pos;
}