#pragma once
#include <nn/hac/HierarchicalIntegrityHeader.h>
#include <nn/hac/HierarchicalSha256Header.h>

class HashTreeMeta
{
public:
	enum HashTreeType
	{
		HASH_TYPE_INTEGRITY,
		HASH_TYPE_SHA256
	};

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
	HashTreeMeta(const byte_t* data, size_t len, HashTreeType type);

	void operator=(const HashTreeMeta& other);
	bool operator==(const HashTreeMeta& other) const;
	bool operator!=(const HashTreeMeta& other) const;

	void importData(const byte_t* data, size_t len, HashTreeType type);

	const fnd::List<sLayer>& getHashLayerInfo() const;
	void setHashLayerInfo(const fnd::List<sLayer>& layer_info);

	const sLayer& getDataLayer() const;
	void setDataLayer(const sLayer& data_info);

	const fnd::List<fnd::sha::sSha256Hash>& getMasterHashList() const;
	void setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list);

	bool getAlignHashToBlock() const;
	void setAlignHashToBlock(bool doAlign);
private:

	// data
	fnd::List<sLayer> mLayerInfo;
	sLayer mDataLayer;
	fnd::List<fnd::sha::sSha256Hash> mMasterHashList;
	bool mDoAlignHashToBlock;

	void importHierarchicalIntergityHeader(const nn::hac::HierarchicalIntegrityHeader& hdr);
	void importHierarchicalSha256Header(const nn::hac::HierarchicalSha256Header& hdr);
};