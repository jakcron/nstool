#pragma once
#include <nx/HierarchicalIntegrityHeader.h>
#include <nx/HierarchicalSha256Header.h>

class HashTreeMeta
{
public:
	struct sLayer
	{
		size_t offset;
		size_t size;
		size_t block_size;

		void operator=(const sLayer& other)
		{
			offset = other.offset;
			size = other.size;
			block_size = other.block_size;
		}

		bool operator==(const sLayer& other) const
		{
			return (offset == other.offset && size == other.size && block_size == other.block_size);
		}

		bool operator!=(const sLayer& other) const
		{
			return !(*this == other);
		}
	};

	HashTreeMeta();
	HashTreeMeta(const nx::HierarchicalIntegrityHeader& hdr);
	HashTreeMeta(const nx::HierarchicalSha256Header& hdr);

	bool operator==(const HashTreeMeta& other) const;
	bool operator!=(const HashTreeMeta& other) const;
	void operator=(const HashTreeMeta& other);

	void importHierarchicalIntergityHeader(const nx::HierarchicalIntegrityHeader& hdr);
	void importHierarchicalSha256Header(const nx::HierarchicalSha256Header& hdr);

	const fnd::List<sLayer>& getHashLayerInfo() const;
	void setHashLayerInfo(const fnd::List<sLayer>& layer_info);

	const sLayer& getDataLayer() const;
	void setDataLayer(const sLayer& data_info);

	const fnd::List<crypto::sha::sSha256Hash>& getMasterHashList() const;
	void setMasterHashList(const fnd::List<crypto::sha::sSha256Hash>& master_hash_list);
private:

	// data
	fnd::List<sLayer> mLayerInfo;
	sLayer mDataLayer;
	fnd::List<crypto::sha::sSha256Hash> mMasterHashList;	

	bool isEqual(const HashTreeMeta& other) const;
	void copyFrom(const HashTreeMeta& other);
};