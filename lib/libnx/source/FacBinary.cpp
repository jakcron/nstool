#include <nx/FacBinary.h>



nx::FacBinary::FacBinary()
{}

nx::FacBinary::FacBinary(const FacBinary & other)
{
	copyFrom(other);
}

nx::FacBinary::FacBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::FacBinary::operator==(const FacBinary & other) const
{
	return isEqual(other);
}

bool nx::FacBinary::operator!=(const FacBinary & other) const
{
	return !isEqual(other);
}

void nx::FacBinary::operator=(const FacBinary & other)
{
	copyFrom(other);
}

const u8 * nx::FacBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::FacBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::FacBinary::exportBinary()
{
	FacHeader::setContentOwnerIdSize(mContentOwnerIdList.getSize() * sizeof(u32));
	FacHeader::setSaveDataOwnerIdSize(mSaveDataOwnerIdList.getSize() * sizeof(u32));
	FacHeader::exportBinary();

	mBinaryBlob.alloc(getFacSize());
	memcpy(mBinaryBlob.getBytes(), FacHeader::getBytes(), FacHeader::getSize());

	u32* rawContentOwnerIds = (u32*)(mBinaryBlob.getBytes() + FacHeader::getContentOwnerIdOffset());
	for (size_t i = 0; i < mContentOwnerIdList.getSize(); i++)
	{
		rawContentOwnerIds[i] = le_word(mContentOwnerIdList[i]);
	}

	u32* rawSaveDataOwnerIds = (u32*)(mBinaryBlob.getBytes() + FacHeader::getSaveDataOwnerIdOffset());
	for (size_t i = 0; i < mSaveDataOwnerIdList.getSize(); i++)
	{
		rawSaveDataOwnerIds[i] = le_word(mSaveDataOwnerIdList[i]);
	}
}

void nx::FacBinary::importBinary(const u8 * bytes, size_t len)
{
	clear();
	FacHeader::importBinary(bytes, len);
	if (FacHeader::getFacSize() > len)
	{
		throw fnd::Exception(kModuleName, "FAC binary too small");
	}

	mBinaryBlob.alloc(FacHeader::getFacSize());
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	u32* rawContentOwnerIds = (u32*)(mBinaryBlob.getBytes() + FacHeader::getContentOwnerIdOffset());
	size_t rawContentOwnerIdNum = FacHeader::getContentOwnerIdSize() / sizeof(u32);
	for (size_t i = 0; i < rawContentOwnerIdNum; i++)
	{
		mContentOwnerIdList.addElement(le_word(rawContentOwnerIds[i]));
	}

	u32* rawSaveDataOwnerIds = (u32*)(mBinaryBlob.getBytes() + FacHeader::getSaveDataOwnerIdOffset());
	size_t rawSaveDataOwnerIdNum = FacHeader::getSaveDataOwnerIdSize() / sizeof(u32);
	for (size_t i = 0; i < rawSaveDataOwnerIdNum; i++)
	{
		mSaveDataOwnerIdList.addElement(le_word(rawSaveDataOwnerIds[i]));
	}
}

bool nx::FacBinary::isEqual(const FacBinary & other) const
{
	return (FacHeader::operator==(other)) \
		&& (mContentOwnerIdList == other.mContentOwnerIdList) \
		&& (mSaveDataOwnerIdList == other.mSaveDataOwnerIdList);
}

void nx::FacBinary::copyFrom(const FacBinary & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		FacHeader::operator=(other);
		mContentOwnerIdList = other.mContentOwnerIdList;
		mSaveDataOwnerIdList = other.mSaveDataOwnerIdList;
	}
}

void nx::FacBinary::clear()
{
	FacHeader::clear();
	mContentOwnerIdList.clear();
	mSaveDataOwnerIdList.clear();
}

const fnd::List<u32>& nx::FacBinary::getContentOwnerIdList() const
{
	return mContentOwnerIdList;
}

void nx::FacBinary::setContentOwnerIdList(const fnd::List<u32>& list)
{
	mContentOwnerIdList = list;
}

const fnd::List<u32>& nx::FacBinary::getSaveDataOwnerIdList() const
{
	return mSaveDataOwnerIdList;
}

void nx::FacBinary::setSaveDataOwnerIdList(const fnd::List<u32>& list)
{
	mSaveDataOwnerIdList = list;
}
