#include <nx/HierarchicalIntegrityHeader.h>

nx::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader()
{
	clear();
}

nx::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader(const HierarchicalIntegrityHeader & other)
{
	copyFrom(other);
}

nx::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::HierarchicalIntegrityHeader::operator==(const HierarchicalIntegrityHeader & other) const
{
	return isEqual(other);
}

bool nx::HierarchicalIntegrityHeader::operator!=(const HierarchicalIntegrityHeader & other) const
{
	return !isEqual(other);
}

void nx::HierarchicalIntegrityHeader::operator=(const HierarchicalIntegrityHeader & other)
{
	copyFrom(other);
}

const byte_t * nx::HierarchicalIntegrityHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::HierarchicalIntegrityHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::HierarchicalIntegrityHeader::exportBinary()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nx::HierarchicalIntegrityHeader::importBinary(const byte_t * bytes, size_t len)
{
	throw fnd::Exception(kModuleName, "importBinary() not implemented");
}

void nx::HierarchicalIntegrityHeader::clear()
{
	mLayerInfo.clear();
	mMasterHashList.clear();
}

const fnd::List<nx::HierarchicalIntegrityHeader::sLayer>& nx::HierarchicalIntegrityHeader::getLayerInfo() const
{
	return mLayerInfo;
}

void nx::HierarchicalIntegrityHeader::setLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}

const fnd::List<crypto::sha::sSha256Hash>& nx::HierarchicalIntegrityHeader::getMasterHashList() const
{
	return mMasterHashList;
}

void nx::HierarchicalIntegrityHeader::setMasterHashList(const fnd::List<crypto::sha::sSha256Hash>& master_hash_list)
{
	mMasterHashList = master_hash_list;
}

bool nx::HierarchicalIntegrityHeader::isEqual(const HierarchicalIntegrityHeader & other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mMasterHashList == other.mMasterHashList);
}

void nx::HierarchicalIntegrityHeader::copyFrom(const HierarchicalIntegrityHeader & other)
{
	if (other.getSize() != 0)
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mLayerInfo = other.mLayerInfo;
		mMasterHashList = other.mMasterHashList;
	}
}
