#include "FacBinary.h"



FacBinary::FacBinary() :
	mHeader()
{
	clearVariables();
}

FacBinary::FacBinary(const FacBinary & other)
{
	importBinary(other.getBytes(), other.getSize());
}

FacBinary::FacBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool FacBinary::operator==(const FacBinary & other) const
{
	return isEqual(other);
}

bool FacBinary::operator!=(const FacBinary & other) const
{
	return !isEqual(other);
}

void FacBinary::operator=(const FacBinary & other)
{
	copyFrom(other);
}

const u8 * FacBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t FacBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void FacBinary::exportBinary()
{
	mHeader.setFsaRights(mFsaRights);
	mHeader.setContentOwnerIdSize(mContentOwnerIds.getSize() * sizeof(u32));
	mHeader.setSaveDataOwnerIdSize(mSaveDataOwnerIds.getSize() * sizeof(u32));
	mHeader.exportBinary();

	mBinaryBlob.alloc(mHeader.getFacSize());
	memcpy(mBinaryBlob.getBytes(), mHeader.getBytes(), mHeader.getSize());

	u32* rawContentOwnerIds = (u32*)(mBinaryBlob.getBytes() + mHeader.getContentOwnerIdOffset());
	for (size_t i = 0; i < mContentOwnerIds.getSize(); i++)
	{
		rawContentOwnerIds[i] = le_word(mContentOwnerIds[i]);
	}
	
	u32* rawSaveDataOwnerIds = (u32*)(mBinaryBlob.getBytes() + mHeader.getSaveDataOwnerIdOffset());
	for (size_t i = 0; i < mSaveDataOwnerIds.getSize(); i++)
	{
		rawSaveDataOwnerIds[i] = le_word(mSaveDataOwnerIds[i]);
	}
}

void FacBinary::importBinary(const u8 * bytes)
{
	throw fnd::Exception(kModuleName, "Unsupported operation. importBinary(const u8* bytes) is not supported for variable size structures.");
}

void FacBinary::importBinary(const u8 * bytes, size_t len)
{
	clearVariables();
	mHeader.importBinary(bytes, len);
	if (mHeader.getFacSize() > len)
	{
		throw fnd::Exception(kModuleName, "FAC binary too small");
	}

	mBinaryBlob.alloc(mHeader.getFacSize());
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	mFsaRights = mHeader.getFsaRights();
	u32* rawContentOwnerIds = (u32*)(mBinaryBlob.getBytes() + mHeader.getContentOwnerIdOffset());
	size_t rawContentOwnerIdNum = mHeader.getContentOwnerIdSize() / sizeof(u32);
	for (size_t i = 0; i < rawContentOwnerIdNum; i++)
	{
		addContentOwnerId(le_word(rawContentOwnerIds[i]));
	}

	u32* rawSaveDataOwnerIds = (u32*)(mBinaryBlob.getBytes() + mHeader.getSaveDataOwnerIdOffset());
	size_t rawSaveDataOwnerIdNum = mHeader.getSaveDataOwnerIdSize() / sizeof(u32);
	for (size_t i = 0; i < rawSaveDataOwnerIdNum; i++)
	{
		addSaveDataOwnerId(le_word(rawSaveDataOwnerIds[i]));
	}
}

bool FacBinary::isPermissionSet(FsAccessFlag flag) const
{
	return (mFsaRights & flag) == flag;
}

void FacBinary::addPermission(FsAccessFlag flag)
{
	mFsaRights |= flag;
}

void FacBinary::removePermission(FsAccessFlag flag)
{
	mFsaRights &= ~(u64)flag;
}

const fnd::List<u32>& FacBinary::getContentOwnerIds() const
{
	return mContentOwnerIds;
}

void FacBinary::addContentOwnerId(u32 id)
{
	mContentOwnerIds.addElement(id);
}

const fnd::List<u32>& FacBinary::getSaveDataOwnerIds() const
{
	return mSaveDataOwnerIds;
}

void FacBinary::addSaveDataOwnerId(u32 id)
{
	mSaveDataOwnerIds.addElement(id);
}

void FacBinary::clearVariables()
{
	mHeader = FacHeader();
	mFsaRights = 0;
	mContentOwnerIds.clear();
	mSaveDataOwnerIds.clear();
}

bool FacBinary::isEqual(const FacBinary & other) const
{
	return (mHeader == other.mHeader) \
		&& (mFsaRights == other.mFsaRights) \
		&& (mContentOwnerIds == other.mContentOwnerIds) \
		&& (mSaveDataOwnerIds == other.mSaveDataOwnerIds);
}

void FacBinary::copyFrom(const FacBinary & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mBinaryBlob.clear();
		mHeader = other.mHeader;
		mFsaRights = other.mFsaRights;
		mContentOwnerIds = other.mContentOwnerIds;
		mSaveDataOwnerIds = other.mSaveDataOwnerIds;
	}
}
