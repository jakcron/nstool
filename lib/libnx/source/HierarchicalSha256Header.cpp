#include <sstream>
#include <nx/HierarchicalSha256Header.h>


nx::HierarchicalSha256Header::HierarchicalSha256Header()
{
	clear();
}

nx::HierarchicalSha256Header::HierarchicalSha256Header(const HierarchicalSha256Header & other)
{
	copyFrom(other);
}

nx::HierarchicalSha256Header::HierarchicalSha256Header(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::HierarchicalSha256Header::operator==(const HierarchicalSha256Header & other) const
{
	return isEqual(other);
}

bool nx::HierarchicalSha256Header::operator!=(const HierarchicalSha256Header & other) const
{
	return !isEqual(other);
}

void nx::HierarchicalSha256Header::operator=(const HierarchicalSha256Header & other)
{
	copyFrom(other);
}

const byte_t * nx::HierarchicalSha256Header::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::HierarchicalSha256Header::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::HierarchicalSha256Header::exportBinary()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nx::HierarchicalSha256Header::importBinary(const byte_t * bytes, size_t len)
{
	if (len < sizeof(nx::sHierarchicalSha256Header))
	{
		throw fnd::Exception(kModuleName, "Header too small");
	}

	const nx::sHierarchicalSha256Header* hdr = (const nx::sHierarchicalSha256Header*)bytes;

	if (hdr->layer_num.get() != nx::hierarchicalsha256::kDefaultLevelNum)
	{
		std::stringstream ss;
		ss.clear();
		ss << "Invalid layer count. ";
		ss << "(actual=" << hdr->layer_num.get() << ", expected=" << nx::hierarchicalsha256::kDefaultLevelNum << ")";
		throw fnd::Exception(kModuleName, ss.str());
	}

	mMasterHash = hdr->master_hash;
	mHashBlockSize = hdr->hash_block_size.get();
	for (size_t i = 0; i < hdr->layer_num.get(); i++)
	{
		mLayerInfo.addElement({hdr->layer[i].offset.get(), hdr->layer[i].size.get()});
	}
}

void nx::HierarchicalSha256Header::clear()
{
	memset(mMasterHash.bytes, 0, sizeof(crypto::sha::sSha256Hash));
	mHashBlockSize = 0;
	mLayerInfo.clear();
}

const crypto::sha::sSha256Hash & nx::HierarchicalSha256Header::getMasterHash() const
{
	return mMasterHash;
}

void nx::HierarchicalSha256Header::setMasterHash(const crypto::sha::sSha256Hash & master_hash)
{
	mMasterHash = master_hash;
}

size_t nx::HierarchicalSha256Header::getHashBlockSize() const
{
	return mHashBlockSize;
}

void nx::HierarchicalSha256Header::setHashBlockSize(size_t hash_block_size)
{
	mHashBlockSize = hash_block_size;
}

const fnd::List<nx::HierarchicalSha256Header::sLayer>& nx::HierarchicalSha256Header::getLayerInfo() const
{
	return mLayerInfo;
}

void nx::HierarchicalSha256Header::setLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}

bool nx::HierarchicalSha256Header::isEqual(const HierarchicalSha256Header & other) const
{
	return (mMasterHash == other.mMasterHash) \
		&& (mHashBlockSize == other.mHashBlockSize) \
		&& (mLayerInfo == other.mLayerInfo);
}

void nx::HierarchicalSha256Header::copyFrom(const HierarchicalSha256Header & other)
{
	if (other.getSize() != 0)
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mMasterHash = other.mMasterHash;
		mHashBlockSize = other.mHashBlockSize;
		mLayerInfo = other.mLayerInfo;
	}
}
