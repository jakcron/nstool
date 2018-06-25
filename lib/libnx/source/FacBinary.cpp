#include <nx/FacBinary.h>
#include <nx/FacHeader.h>

nx::FacBinary::FacBinary()
{
	clear();
}

nx::FacBinary::FacBinary(const FacBinary & other)
{
	*this = other;
}

void nx::FacBinary::operator=(const FacBinary & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mFsaRights = other.mFsaRights;
		mContentOwnerIdList = other.mContentOwnerIdList;
		mSaveDataOwnerIdList = other.mSaveDataOwnerIdList;
	}
}

bool nx::FacBinary::operator==(const FacBinary & other) const
{
	return (mFsaRights == other.mFsaRights) \
		&& (mContentOwnerIdList == other.mContentOwnerIdList) \
		&& (mSaveDataOwnerIdList == other.mSaveDataOwnerIdList);
}

bool nx::FacBinary::operator!=(const FacBinary & other) const
{
	return !(*this == other);
}

void nx::FacBinary::toBytes()
{
	FacHeader hdr;

	FacHeader::sSection content_id_list_pos, savedata_owner_id_list_pos;

	content_id_list_pos.size = mContentOwnerIdList.size() * sizeof(uint32_t);
	content_id_list_pos.offset = align(sizeof(sFacHeader), 4);
	savedata_owner_id_list_pos.size = mSaveDataOwnerIdList.size() * sizeof(uint32_t);
	savedata_owner_id_list_pos.offset = content_id_list_pos.offset + align(content_id_list_pos.size, 4);

	hdr.setFormatVersion(fac::kFacFormatVersion);
	hdr.setFsaRightsList(mFsaRights);
	hdr.setContentOwnerIdPos(content_id_list_pos);
	hdr.setSaveDataOwnerIdPos(savedata_owner_id_list_pos);
	hdr.toBytes();

	mRawBinary.alloc(hdr.getFacSize());
	memcpy(mRawBinary.data(), hdr.getBytes().data(), hdr.getBytes().size());

	uint32_t* rawContentOwnerIds = (uint32_t*)(mRawBinary.data() + content_id_list_pos.offset);
	for (size_t i = 0; i < mContentOwnerIdList.size(); i++)
	{
		rawContentOwnerIds[i] = le_word(mContentOwnerIdList[i]);
	}

	uint32_t* rawSaveDataOwnerIds = (uint32_t*)(mRawBinary.data() + savedata_owner_id_list_pos.offset);
	for (size_t i = 0; i < mSaveDataOwnerIdList.size(); i++)
	{
		rawSaveDataOwnerIds[i] = le_word(mSaveDataOwnerIdList[i]);
	}
}

void nx::FacBinary::fromBytes(const byte_t* data, size_t len)
{
	clear();

	FacHeader hdr;
	hdr.fromBytes(data, len);

	mFsaRights = hdr.getFsaRightsList();

	if (hdr.getFacSize() > len)
	{
		throw fnd::Exception(kModuleName, "FAC binary too small");
	}

	mRawBinary.alloc(hdr.getFacSize());
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	uint32_t* rawContentOwnerIds = (uint32_t*)(mRawBinary.data() + hdr.getContentOwnerIdPos().offset);
	size_t rawContentOwnerIdNum = hdr.getContentOwnerIdPos().size / sizeof(uint32_t);
	for (size_t i = 0; i < rawContentOwnerIdNum; i++)
	{
		mContentOwnerIdList.addElement(le_word(rawContentOwnerIds[i]));
	}

	uint32_t* rawSaveDataOwnerIds = (uint32_t*)(mRawBinary.data() + hdr.getSaveDataOwnerIdPos().offset);
	size_t rawSaveDataOwnerIdNum = hdr.getSaveDataOwnerIdPos().size / sizeof(uint32_t);
	for (size_t i = 0; i < rawSaveDataOwnerIdNum; i++)
	{
		mSaveDataOwnerIdList.addElement(le_word(rawSaveDataOwnerIds[i]));
	}
}

const fnd::Vec<byte_t>& nx::FacBinary::getBytes() const
{
	return mRawBinary;
}

void nx::FacBinary::clear()
{
	mRawBinary.clear();
	mContentOwnerIdList.clear();
	mSaveDataOwnerIdList.clear();
}

const fnd::List<nx::fac::FsAccessFlag>& nx::FacBinary::getFsaRightsList() const
{
	return mFsaRights;
}

void nx::FacBinary::setFsaRightsList(const fnd::List<fac::FsAccessFlag>& list)
{
	mFsaRights.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		mFsaRights.hasElement(list[i]) ? mFsaRights.addElement(list[i]) : throw fnd::Exception(kModuleName, "FSA right already exists");
	}
}

const fnd::List<uint32_t>& nx::FacBinary::getContentOwnerIdList() const
{
	return mContentOwnerIdList;
}

void nx::FacBinary::setContentOwnerIdList(const fnd::List<uint32_t>& list)
{
	mContentOwnerIdList = list;
}

const fnd::List<uint32_t>& nx::FacBinary::getSaveDataOwnerIdList() const
{
	return mSaveDataOwnerIdList;
}

void nx::FacBinary::setSaveDataOwnerIdList(const fnd::List<uint32_t>& list)
{
	mSaveDataOwnerIdList = list;
}
