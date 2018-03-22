#include <nx/FacBinary.h>



nx::FacBinary::FacBinary()
{}

nx::FacBinary::FacBinary(const FacBinary & other)
{
	copyFrom(other);
}

nx::FacBinary::FacBinary(const byte_t * bytes, size_t len)
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

const byte_t * nx::FacBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::FacBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::FacBinary::exportBinary()
{
	FacHeader::setContentOwnerIdSize(mContentOwnerIdList.getSize() * sizeof(uint32_t));
	FacHeader::setSaveDataOwnerIdSize(mSaveDataOwnerIdList.getSize() * sizeof(uint32_t));
	FacHeader::exportBinary();

	mBinaryBlob.alloc(getFacSize());
	memcpy(mBinaryBlob.getBytes(), FacHeader::getBytes(), FacHeader::getSize());

	uint32_t* rawContentOwnerIds = (uint32_t*)(mBinaryBlob.getBytes() + FacHeader::getContentOwnerIdOffset());
	for (size_t i = 0; i < mContentOwnerIdList.getSize(); i++)
	{
		rawContentOwnerIds[i] = le_word(mContentOwnerIdList[i]);
	}

	uint32_t* rawSaveDataOwnerIds = (uint32_t*)(mBinaryBlob.getBytes() + FacHeader::getSaveDataOwnerIdOffset());
	for (size_t i = 0; i < mSaveDataOwnerIdList.getSize(); i++)
	{
		rawSaveDataOwnerIds[i] = le_word(mSaveDataOwnerIdList[i]);
	}
}

void nx::FacBinary::importBinary(const byte_t * bytes, size_t len)
{
	clear();
	FacHeader::importBinary(bytes, len);
	if (FacHeader::getFacSize() > len)
	{
		throw fnd::Exception(kModuleName, "FAC binary too small");
	}

	mBinaryBlob.alloc(FacHeader::getFacSize());
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	uint32_t* rawContentOwnerIds = (uint32_t*)(mBinaryBlob.getBytes() + FacHeader::getContentOwnerIdOffset());
	size_t rawContentOwnerIdNum = FacHeader::getContentOwnerIdSize() / sizeof(uint32_t);
	for (size_t i = 0; i < rawContentOwnerIdNum; i++)
	{
		mContentOwnerIdList.addElement(le_word(rawContentOwnerIds[i]));
	}

	uint32_t* rawSaveDataOwnerIds = (uint32_t*)(mBinaryBlob.getBytes() + FacHeader::getSaveDataOwnerIdOffset());
	size_t rawSaveDataOwnerIdNum = FacHeader::getSaveDataOwnerIdSize() / sizeof(uint32_t);
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
