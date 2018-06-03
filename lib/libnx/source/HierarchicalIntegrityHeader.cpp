#include <sstream>
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
	std::stringstream error_str;

	// validate size for at least header
	if (len < sizeof(nx::sHierarchicalIntegrityHeader))
	{
		throw fnd::Exception(kModuleName, "Header too small");
	}

	const nx::sHierarchicalIntegrityHeader* hdr = (const nx::sHierarchicalIntegrityHeader*)bytes;

	// Validate Header Sig "IVFC"
	if (hdr->signature.get() != hierarchicalintegrity::kStructSig)
	{
		throw fnd::Exception(kModuleName, "Invalid struct magic");
	}

	// Validate TypeId
	if (hdr->type_id.get() != nx::hierarchicalintegrity::kRomfsTypeId)
	{
		error_str.clear();
		error_str << "Unsupported type id (" << std::hex << hdr->type_id.get() << ")";
		throw fnd::Exception(kModuleName, error_str.str());
	}

	// Validate Layer Num
	if (hdr->layer_num.get() != hierarchicalintegrity::kDefaultLayerNum+1)
	{
		error_str.clear();
		error_str << "Invalid layer count. ";
		error_str << "(actual=" << std::dec << hdr->layer_num.get() << ", expected=" << nx::hierarchicalintegrity::kDefaultLayerNum+1 << ")";
		throw fnd::Exception(kModuleName, error_str.str());
	}
	
	// Get Sizes/Offsets
	size_t master_hash_offset = align((sizeof(nx::sHierarchicalIntegrityHeader) + sizeof(nx::sHierarchicalIntegrityLayerInfo) * hdr->layer_num.get()), nx::hierarchicalintegrity::kHeaderAlignLen);
	size_t total_size = master_hash_offset + hdr->master_hash_size.get();

	// Validate total size
	if (len < total_size)
	{
		throw fnd::Exception(kModuleName, "Header too small");
	}

	// copy to internal storage
	mBinaryBlob.alloc(total_size);
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	// save layer info
	const nx::sHierarchicalIntegrityLayerInfo* layer_info = (const nx::sHierarchicalIntegrityLayerInfo*)(mBinaryBlob.getBytes() + sizeof(nx::sHierarchicalIntegrityHeader));
	for (size_t i = 0; i < hierarchicalintegrity::kDefaultLayerNum; i++)
	{
		mLayerInfo.addElement({layer_info[i].offset.get(), layer_info[i].size.get(), layer_info[i].block_size.get()});
	}

	// save hash list
	const crypto::sha::sSha256Hash* hash_list = (const crypto::sha::sSha256Hash*)(mBinaryBlob.getBytes() + master_hash_offset);
	for (size_t i = 0; i < hdr->master_hash_size.get()/sizeof(crypto::sha::sSha256Hash); i++)
	{
		mMasterHashList.addElement(hash_list[i]);
	}
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
