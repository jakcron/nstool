#include <sstream>
#include <nx/HierarchicalIntegrityHeader.h>

nx::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader()
{
	clear();
}

nx::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader(const HierarchicalIntegrityHeader & other)
{
	*this = other;
}

void nx::HierarchicalIntegrityHeader::operator=(const HierarchicalIntegrityHeader & other)
{
	if (other.getBytes().size() != 0)
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mLayerInfo = other.mLayerInfo;
		mMasterHashList = other.mMasterHashList;
	}
}

bool nx::HierarchicalIntegrityHeader::operator==(const HierarchicalIntegrityHeader & other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mMasterHashList == other.mMasterHashList);
}

bool nx::HierarchicalIntegrityHeader::operator!=(const HierarchicalIntegrityHeader & other) const
{
	return !(*this == other);
}

void nx::HierarchicalIntegrityHeader::toBytes()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nx::HierarchicalIntegrityHeader::fromBytes(const byte_t* data, size_t len)
{
	std::stringstream error_str;

	// validate size for at least header
	if (len < sizeof(nx::sHierarchicalIntegrityHeader))
	{
		throw fnd::Exception(kModuleName, "Header too small");
	}

	const nx::sHierarchicalIntegrityHeader* hdr = (const nx::sHierarchicalIntegrityHeader*)data;

	// Validate Header Sig "IVFC"
	if (hdr->st_magic.get() != hierarchicalintegrity::kStructMagic)
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
	mRawBinary.alloc(total_size);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// save layer info
	const nx::sHierarchicalIntegrityLayerInfo* layer_info = (const nx::sHierarchicalIntegrityLayerInfo*)(mRawBinary.data() + sizeof(nx::sHierarchicalIntegrityHeader));
	for (size_t i = 0; i < hierarchicalintegrity::kDefaultLayerNum; i++)
	{
		mLayerInfo.addElement({layer_info[i].offset.get(), layer_info[i].size.get(), layer_info[i].block_size.get()});
	}

	// save hash list
	const crypto::sha::sSha256Hash* hash_list = (const crypto::sha::sSha256Hash*)(mRawBinary.data() + master_hash_offset);
	for (size_t i = 0; i < hdr->master_hash_size.get()/sizeof(crypto::sha::sSha256Hash); i++)
	{
		mMasterHashList.addElement(hash_list[i]);
	}
}

const fnd::Vec<byte_t>& nx::HierarchicalIntegrityHeader::getBytes() const
{
	return mRawBinary;
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