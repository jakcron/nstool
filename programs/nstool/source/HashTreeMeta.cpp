#include "HashTreeMeta.h"

HashTreeMeta::HashTreeMeta() :
	mLayerInfo(),
	mDataLayer(),
	mMasterHashList(),
	mDoAlignHashToBlock(false)
{

}

HashTreeMeta::HashTreeMeta(const nx::HierarchicalIntegrityHeader& hdr) :
	mLayerInfo(),
	mDataLayer(),
	mMasterHashList(),
	mDoAlignHashToBlock(false)
{
	importHierarchicalIntergityHeader(hdr);
}

HashTreeMeta::HashTreeMeta(const nx::HierarchicalSha256Header& hdr) :
	mLayerInfo(),
	mDataLayer(),
	mMasterHashList(),
	mDoAlignHashToBlock(false)
{
	importHierarchicalSha256Header(hdr);
}

bool HashTreeMeta::operator==(const HashTreeMeta& other) const
{
	return isEqual(other);
}

bool HashTreeMeta::operator!=(const HashTreeMeta& other) const
{
	return !isEqual(other);
}

void HashTreeMeta::operator=(const HashTreeMeta& other)
{
	copyFrom(other);
}

void HashTreeMeta::importHierarchicalIntergityHeader(const nx::HierarchicalIntegrityHeader& hdr)
{
	mDoAlignHashToBlock = true;
	for (size_t i = 0; i < hdr.getLayerInfo().getSize(); i++)
	{
		sLayer layer;
		layer.offset = hdr.getLayerInfo()[i].offset;
		layer.size = hdr.getLayerInfo()[i].size;
		layer.block_size = _BIT(hdr.getLayerInfo()[i].block_size);
		if (i+1 == hdr.getLayerInfo().getSize())
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

void HashTreeMeta::importHierarchicalSha256Header(const nx::HierarchicalSha256Header& hdr)
{
	mDoAlignHashToBlock = false;
	for (size_t i = 0; i < hdr.getLayerInfo().getSize(); i++)
	{
		sLayer layer;
		layer.offset = hdr.getLayerInfo()[i].offset;
		layer.size = hdr.getLayerInfo()[i].size;
		layer.block_size = hdr.getHashBlockSize();
		if (i+1 == hdr.getLayerInfo().getSize())
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

const fnd::List<crypto::sha::sSha256Hash>&  HashTreeMeta::getMasterHashList() const
{
	return mMasterHashList;
}

void  HashTreeMeta::setMasterHashList(const fnd::List<crypto::sha::sSha256Hash>& master_hash_list)
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

bool  HashTreeMeta::isEqual(const HashTreeMeta& other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mDataLayer == other.mDataLayer) \
		&& (mMasterHashList == other.mMasterHashList) \
		&& (mDoAlignHashToBlock == other.mDoAlignHashToBlock);
}

void  HashTreeMeta::copyFrom(const HashTreeMeta& other)
{
	mLayerInfo = other.mLayerInfo;
	mDataLayer = other.mDataLayer;
	mMasterHashList = other.mMasterHashList;
	mDoAlignHashToBlock = other.mDoAlignHashToBlock;
}
