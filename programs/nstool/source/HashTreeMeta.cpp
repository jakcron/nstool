#include "HashTreeMeta.h"

HashTreeMeta::HashTreeMeta() :
	mLayerInfo(),
	mDataLayer(),
	mMasterHashList(),
	mDoAlignHashToBlock(false)
{

}

HashTreeMeta::HashTreeMeta(const byte_t* data, size_t len, HashTreeType type) :
	HashTreeMeta()
{
	importData(data, len, type);
}

void HashTreeMeta::operator=(const HashTreeMeta& other)
{
	mLayerInfo = other.mLayerInfo;
	mDataLayer = other.mDataLayer;
	mMasterHashList = other.mMasterHashList;
	mDoAlignHashToBlock = other.mDoAlignHashToBlock;
}

bool HashTreeMeta::operator==(const HashTreeMeta& other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mDataLayer == other.mDataLayer) \
		&& (mMasterHashList == other.mMasterHashList) \
		&& (mDoAlignHashToBlock == other.mDoAlignHashToBlock);
}

bool HashTreeMeta::operator!=(const HashTreeMeta& other) const
{
	return !(*this == other);
}

void HashTreeMeta::importData(const byte_t* data, size_t len, HashTreeType type)
{
	if (type == HASH_TYPE_INTEGRITY)
	{
		nn::hac::HierarchicalIntegrityHeader hdr;
		hdr.fromBytes(data, len);
		importHierarchicalIntergityHeader(hdr);
	}
	else if (type == HASH_TYPE_SHA256)
	{
		nn::hac::HierarchicalSha256Header hdr;
		hdr.fromBytes(data, len);
		importHierarchicalSha256Header(hdr);
	}
}

const fnd::List<HashTreeMeta::sLayer>& HashTreeMeta::getHashLayerInfo() const
{
	return mLayerInfo;
}

void HashTreeMeta::setHashLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}

const HashTreeMeta::sLayer& HashTreeMeta::getDataLayer() const
{
	return mDataLayer;
}

void  HashTreeMeta::setDataLayer(const sLayer& data_info)
{
	mDataLayer = data_info;
}

const fnd::List<fnd::sha::sSha256Hash>&  HashTreeMeta::getMasterHashList() const
{
	return mMasterHashList;
}

void  HashTreeMeta::setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list)
{
	mMasterHashList = master_hash_list;
}

bool HashTreeMeta::getAlignHashToBlock() const
{
	return mDoAlignHashToBlock;
}

void HashTreeMeta::setAlignHashToBlock(bool doAlign)
{
	mDoAlignHashToBlock = doAlign;
}

void HashTreeMeta::importHierarchicalIntergityHeader(const nn::hac::HierarchicalIntegrityHeader& hdr)
{
	mDoAlignHashToBlock = true;
	for (size_t i = 0; i < hdr.getLayerInfo().size(); i++)
	{
		sLayer layer;
		layer.offset = hdr.getLayerInfo()[i].offset;
		layer.size = hdr.getLayerInfo()[i].size;
		layer.block_size = _BIT(hdr.getLayerInfo()[i].block_size);
		if (i+1 == hdr.getLayerInfo().size())
		{
			mDataLayer = layer;
		}
		else
		{
			mLayerInfo.addElement(layer);
		}
	}
	mMasterHashList = hdr.getMasterHashList();
}

void HashTreeMeta::importHierarchicalSha256Header(const nn::hac::HierarchicalSha256Header& hdr)
{
	mDoAlignHashToBlock = false;
	for (size_t i = 0; i < hdr.getLayerInfo().size(); i++)
	{
		sLayer layer;
		layer.offset = hdr.getLayerInfo()[i].offset;
		layer.size = hdr.getLayerInfo()[i].size;
		layer.block_size = hdr.getHashBlockSize();
		if (i+1 == hdr.getLayerInfo().size())
		{
			mDataLayer = layer;
		}
		else
		{
			mLayerInfo.addElement(layer);
		}
	}
	mMasterHashList.addElement(hdr.getMasterHash());
}