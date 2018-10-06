#include "LayeredIntegrityMetadata.h"

LayeredIntegrityMetadata::LayeredIntegrityMetadata() :
	mLayerInfo(),
	mDataLayer(),
	mMasterHashList(),
	mDoAlignHashToBlock(false)
{

}

void LayeredIntegrityMetadata::operator=(const LayeredIntegrityMetadata& other)
{
	mLayerInfo = other.mLayerInfo;
	mDataLayer = other.mDataLayer;
	mMasterHashList = other.mMasterHashList;
	mDoAlignHashToBlock = other.mDoAlignHashToBlock;
}

bool LayeredIntegrityMetadata::operator==(const LayeredIntegrityMetadata& other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mDataLayer == other.mDataLayer) \
		&& (mMasterHashList == other.mMasterHashList) \
		&& (mDoAlignHashToBlock == other.mDoAlignHashToBlock);
}

bool LayeredIntegrityMetadata::operator!=(const LayeredIntegrityMetadata& other) const
{
	return !(*this == other);
}

const fnd::List<LayeredIntegrityMetadata::sLayer>& LayeredIntegrityMetadata::getHashLayerInfo() const
{
	return mLayerInfo;
}

void LayeredIntegrityMetadata::setHashLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}

const LayeredIntegrityMetadata::sLayer& LayeredIntegrityMetadata::getDataLayer() const
{
	return mDataLayer;
}

void  LayeredIntegrityMetadata::setDataLayerInfo(const sLayer& data_info)
{
	mDataLayer = data_info;
}

const fnd::List<fnd::sha::sSha256Hash>&  LayeredIntegrityMetadata::getMasterHashList() const
{
	return mMasterHashList;
}

void  LayeredIntegrityMetadata::setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list)
{
	mMasterHashList = master_hash_list;
}

bool LayeredIntegrityMetadata::getAlignHashToBlock() const
{
	return mDoAlignHashToBlock;
}

void LayeredIntegrityMetadata::setAlignHashToBlock(bool doAlign)
{
	mDoAlignHashToBlock = doAlign;
}