#include <sstream>
#include <nn/hac/HierarchicalSha256Header.h>

nn::hac::HierarchicalSha256Header::HierarchicalSha256Header()
{
	clear();
}

nn::hac::HierarchicalSha256Header::HierarchicalSha256Header(const HierarchicalSha256Header & other)
{
	*this = other;
}

void nn::hac::HierarchicalSha256Header::operator=(const HierarchicalSha256Header & other)
{
	if (other.getBytes().size() != 0)
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		mMasterHash = other.mMasterHash;
		mHashBlockSize = other.mHashBlockSize;
		mLayerInfo = other.mLayerInfo;
	}
}

bool nn::hac::HierarchicalSha256Header::operator==(const HierarchicalSha256Header & other) const
{
	return (mMasterHash == other.mMasterHash) \
		&& (mHashBlockSize == other.mHashBlockSize) \
		&& (mLayerInfo == other.mLayerInfo);
}

bool nn::hac::HierarchicalSha256Header::operator!=(const HierarchicalSha256Header & other) const
{
	return !(*this == other);
}

void nn::hac::HierarchicalSha256Header::toBytes()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nn::hac::HierarchicalSha256Header::fromBytes(const byte_t* data, size_t len)
{
	std::stringstream error_str;

	if (len < sizeof(nn::hac::sHierarchicalSha256Header))
	{
		throw fnd::Exception(kModuleName, "Header too small");
	}

	const nn::hac::sHierarchicalSha256Header* hdr = (const nn::hac::sHierarchicalSha256Header*)data;

	if (hdr->layer_num.get() != nn::hac::hierarchicalsha256::kDefaultLayerNum)
	{
		error_str.clear();
		error_str << "Invalid layer count. ";
		error_str << "(actual=" << std::dec << hdr->layer_num.get() << ", expected=" << nn::hac::hierarchicalsha256::kDefaultLayerNum << ")";
		throw fnd::Exception(kModuleName, error_str.str());
	}

	mMasterHash = hdr->master_hash;
	mHashBlockSize = hdr->hash_block_size.get();
	for (size_t i = 0; i < hdr->layer_num.get(); i++)
	{
		mLayerInfo.addElement({hdr->layer[i].offset.get(), hdr->layer[i].size.get()});
	}
}

const fnd::Vec<byte_t>& nn::hac::HierarchicalSha256Header::getBytes() const
{
	return mRawBinary;
}

void nn::hac::HierarchicalSha256Header::clear()
{
	memset(mMasterHash.bytes, 0, sizeof(fnd::sha::sSha256Hash));
	mHashBlockSize = 0;
	mLayerInfo.clear();
}

const fnd::sha::sSha256Hash & nn::hac::HierarchicalSha256Header::getMasterHash() const
{
	return mMasterHash;
}

void nn::hac::HierarchicalSha256Header::setMasterHash(const fnd::sha::sSha256Hash & master_hash)
{
	mMasterHash = master_hash;
}

size_t nn::hac::HierarchicalSha256Header::getHashBlockSize() const
{
	return mHashBlockSize;
}

void nn::hac::HierarchicalSha256Header::setHashBlockSize(size_t hash_block_size)
{
	mHashBlockSize = hash_block_size;
}

const fnd::List<nn::hac::HierarchicalSha256Header::sLayer>& nn::hac::HierarchicalSha256Header::getLayerInfo() const
{
	return mLayerInfo;
}

void nn::hac::HierarchicalSha256Header::setLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}