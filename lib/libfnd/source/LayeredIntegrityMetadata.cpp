#include <fnd/LayeredIntegrityMetadata.h>

fnd::LayeredIntegrityMetadata::LayeredIntegrityMetadata() :
	mLayerInfo(),
	mDataLayer(),
	mMasterHashList(),
	mDoAlignHashToBlock(false)
{

}

void fnd::LayeredIntegrityMetadata::operator=(const LayeredIntegrityMetadata& other)
{
	mLayerInfo = other.mLayerInfo;
	mDataLayer = other.mDataLayer;
	mMasterHashList = other.mMasterHashList;
	mDoAlignHashToBlock = other.mDoAlignHashToBlock;
}

bool fnd::LayeredIntegrityMetadata::operator==(const LayeredIntegrityMetadata& other) const
{
	return (mLayerInfo == other.mLayerInfo) \
		&& (mDataLayer == other.mDataLayer) \
		&& (mMasterHashList == other.mMasterHashList) \
		&& (mDoAlignHashToBlock == other.mDoAlignHashToBlock);
}

bool fnd::LayeredIntegrityMetadata::operator!=(const LayeredIntegrityMetadata& other) const
{
	return !(*this == other);
}

const fnd::List<fnd::LayeredIntegrityMetadata::sLayer>& fnd::LayeredIntegrityMetadata::getHashLayerInfo() const
{
	return mLayerInfo;
}

void fnd::LayeredIntegrityMetadata::setHashLayerInfo(const fnd::List<sLayer>& layer_info)
{
	mLayerInfo = layer_info;
}

const fnd::LayeredIntegrityMetadata::sLayer& fnd::LayeredIntegrityMetadata::getDataLayer() const
{
	return mDataLayer;
}

void  fnd::LayeredIntegrityMetadata::setDataLayerInfo(const sLayer& data_info)
{
	mDataLayer = data_info;
}

const fnd::List<fnd::sha::sSha256Hash>&  fnd::LayeredIntegrityMetadata::getMasterHashList() const
{
	return mMasterHashList;
}

void  fnd::LayeredIntegrityMetadata::setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list)
{
	mMasterHashList = master_hash_list;
}

bool fnd::LayeredIntegrityMetadata::getAlignHashToBlock() const
{
	return mDoAlignHashToBlock;
}

void fnd::LayeredIntegrityMetadata::setAlignHashToBlock(bool doAlign)
{
	mDoAlignHashToBlock = doAlign;
}