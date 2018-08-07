#include <sstream>
#include <nn/hac/HierarchicalIntegrityHeader.h>

nn::hac::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader()
{
	clear();
}

nn::hac::HierarchicalIntegrityHeader::HierarchicalIntegrityHeader(const HierarchicalIntegrityHeader & other)
{
	*this = other;
}

void nn::hac::HierarchicalIntegrityHeader::operator=(const HierarchicalIntegrityHeader & other)
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

bool nn::hac::HierarchicalIntegrityHeader::operator==(const HierarchicalIntegrityHeader & other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mMasterHashList == other.mMasterHashList);
}

bool nn::hac::HierarchicalIntegrityHeader::operator!=(const HierarchicalIntegrityHeader & other) const
{
	return !(*this == other);
}

void nn::hac::HierarchicalIntegrityHeader::toBytes()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nn::hac::HierarchicalIntegrityHeader::fromBytes(const byte_t* data, size_t len)
{
	std::stringstream error_str;

	// validate size for at least header
	if (len < sizeof(nn::hac::sHierarchicalIntegrityHeader))
	{
		throw fnd::Exception(kModuleName, "Header too small");
	}

	const nn::hac::sHierarchicalIntegrityHeader* hdr = (const nn::hac::sHierarchicalIntegrityHeader*)data;

	// Validate Header Sig "IVFC"
	if (hdr->st_magic.get() != hierarchicalintegrity::kStructMagic)
	{
		throw fnd::Exception(kModuleName, "Invalid struct magic");
	}

	// Validate TypeId
	if (hdr->type_id.get() != nn::hac::hierarchicalintegrity::kRomfsTypeId)
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
		error_str << "(actual=" << std::dec << hdr->layer_num.get() << ", expected=" << nn::hac::hierarchicalintegrity::kDefaultLayerNum+1 << ")";
		throw fnd::Exception(kModuleName, error_str.str());
	}
	
	// Get Sizes/Offsets
	size_t master_hash_offset = align((sizeof(nn::hac::sHierarchicalIntegrityHeader) + sizeof(nn::hac::sHierarchicalIntegrityLayerInfo) * hdr->layer_num.get()), nn::hac::hierarchicalintegrity::kHeaderAlignLen);
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
	const nn::hac::sHierarchicalIntegrityLayerInfo* layer_info = (const nn::hac::sHierarchicalIntegrityLayerInfo*)(mRawBinary.data() + sizeof(nn::hac::sHierarchicalIntegrityHeader));
	for (size_t i = 0; i < hierarchicalintegrity::kDefaultLayerNum; i++)
	{
		mLayerInfo.addElement({layer_info[i].offset.get(), layer_info[i].size.get(), layer_info[i].block_size.get()});
	}

	// save hash list
	const fnd::sha::sSha256Hash* hash_list = (const fnd::sha::sSha256Hash*)(mRawBinary.data() + master_hash_offset);
	for (size_t i = 0; i < hdr->master_hash_size.get()/sizeof(fnd::sha::sSha256Hash); i++)
	{
		mMasterHashList.addElement(hash_list[i]);
	}
}

const fnd::Vec<byte_t>& nn::hac::HierarchicalIntegrityHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::HierarchicalIntegrityHeader::clear()
{
	mLayerInfo.clear();
	mMasterHashList.clear();
}

const fnd::List<nn::hac::HierarchicalIntegrityHeader::sLayer>& nn::hac::HierarchicalIntegrityHeader::getLayerInfo() const
{
	return mLayerInfo;
}

void nn::hac::HierarchicalIntegrityHeader::setLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}

const fnd::List<fnd::sha::sSha256Hash>& nn::hac::HierarchicalIntegrityHeader::getMasterHashList() const
{
	return mMasterHashList;
}

void nn::hac::HierarchicalIntegrityHeader::setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list)
{
	mMasterHashList = master_hash_list;
}