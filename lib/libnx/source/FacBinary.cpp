#include <nx/FacBinary.h>

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
		FacHeader::operator=(other);
		mContentOwnerIdList = other.mContentOwnerIdList;
		mSaveDataOwnerIdList = other.mSaveDataOwnerIdList;
	}
}

bool nx::FacBinary::operator==(const FacBinary & other) const
{
	return (FacHeader::operator==(other)) \
		&& (mContentOwnerIdList == other.mContentOwnerIdList) \
		&& (mSaveDataOwnerIdList == other.mSaveDataOwnerIdList);
}

bool nx::FacBinary::operator!=(const FacBinary & other) const
{
	return !(*this == other);
}

void nx::FacBinary::toBytes()
{
	FacHeader::setContentOwnerIdSize(mContentOwnerIdList.size() * sizeof(uint32_t));
	FacHeader::setSaveDataOwnerIdSize(mSaveDataOwnerIdList.size() * sizeof(uint32_t));
	FacHeader::toBytes();

	mRawBinary.alloc(getFacSize());
	memcpy(mRawBinary.data(), FacHeader::getBytes().data(), FacHeader::getBytes().size());

	uint32_t* rawContentOwnerIds = (uint32_t*)(mRawBinary.data() + FacHeader::getContentOwnerIdPos().offset);
	for (size_t i = 0; i < mContentOwnerIdList.size(); i++)
	{
		rawContentOwnerIds[i] = le_word(mContentOwnerIdList[i]);
	}

	uint32_t* rawSaveDataOwnerIds = (uint32_t*)(mRawBinary.data() + FacHeader::getSaveDataOwnerIdPos().offset);
	for (size_t i = 0; i < mSaveDataOwnerIdList.size(); i++)
	{
		rawSaveDataOwnerIds[i] = le_word(mSaveDataOwnerIdList[i]);
	}
}

void nx::FacBinary::fromBytes(const byte_t* data, size_t len)
{
	clear();
	FacHeader::fromBytes(data, len);
	if (FacHeader::getFacSize() > len)
	{
		throw fnd::Exception(kModuleName, "FAC binary too small");
	}

	mRawBinary.alloc(FacHeader::getFacSize());
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	uint32_t* rawContentOwnerIds = (uint32_t*)(mRawBinary.data() + FacHeader::getContentOwnerIdPos().offset);
	size_t rawContentOwnerIdNum = FacHeader::getContentOwnerIdPos().size / sizeof(uint32_t);
	for (size_t i = 0; i < rawContentOwnerIdNum; i++)
	{
		mContentOwnerIdList.addElement(le_word(rawContentOwnerIds[i]));
	}

	uint32_t* rawSaveDataOwnerIds = (uint32_t*)(mRawBinary.data() + FacHeader::getSaveDataOwnerIdPos().offset);
	size_t rawSaveDataOwnerIdNum = FacHeader::getSaveDataOwnerIdPos().size / sizeof(uint32_t);
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
	FacHeader::clear();
	mRawBinary.clear();
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
