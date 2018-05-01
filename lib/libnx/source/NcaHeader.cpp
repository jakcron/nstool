#include <nx/NcaHeader.h>

using namespace nx;


void NcaHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sNcaHeader));
	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	
	switch(mFormatVersion)
	{
	case (NCA2_FORMAT):
		strncpy(hdr->signature, nca::kNca2Sig.c_str(), 4);
		break;
	case (NCA3_FORMAT):
		strncpy(hdr->signature, nca::kNca3Sig.c_str(), 4);
		break;
	default:
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}
	hdr->distribution_type = mDistributionType;
	hdr->content_type = mContentType;
	if (mKeyGeneration > 2)
	{
		hdr->key_generation = 2;
		hdr->key_generation_2 = mKeyGeneration;
	}
	else
	{
		hdr->key_generation = mKeyGeneration;
		hdr->key_generation_2 = 0;
	}
	
	hdr->key_area_encryption_key_index = mKaekIndex;
	hdr->content_size = mContentSize;
	hdr->program_id = mProgramId;
	hdr->content_index = mContentIndex;
	hdr->sdk_addon_version = mSdkAddonVersion;
	memcpy(hdr->rights_id, mRightsId, nca::kRightsIdLen);

	// TODO: properly reconstruct NCA layout? atm in hands of user
	for (size_t i = 0; i < mPartitions.getSize(); i++)
	{
		// determine partition index
		byte_t idx = mPartitions[i].index;

		if (mPartitions[i].index >= nca::kPartitionNum || hdr->partition[idx].enabled) continue;

		hdr->partition[idx].start = sizeToBlockNum(mPartitions[i].offset);
		hdr->partition[idx].end = (sizeToBlockNum(mPartitions[i].offset) + sizeToBlockNum(mPartitions[i].size));
		hdr->partition[idx].enabled = true;
		hdr->partition_hash[idx] = mPartitions[i].hash;
	}

	for (size_t i = 0; i < nca::kAesKeyNum; i++)
	{
		hdr->enc_aes_key[i] = mEncAesKeys[i];
	}
}

void NcaHeader::importBinary(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sNcaHeader))
	{
		throw fnd::Exception(kModuleName, "NCA header size is too small");
	}

	clear();

	mBinaryBlob.alloc(sizeof(sNcaHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, sizeof(sNcaHeader));

	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	std::string sig = std::string(hdr->signature, 4);
	if (sig == nca::kNca2Sig)
	{
		mFormatVersion = NCA2_FORMAT;
	}
	else if (sig == nca::kNca3Sig)
	{
		mFormatVersion = NCA3_FORMAT;
	}
	else
	{
		throw fnd::Exception(kModuleName, "NCA header corrupt");
	}

	mDistributionType = (nca::DistributionType)hdr->distribution_type;
	mContentType = (nca::ContentType)hdr->content_type;
	mKeyGeneration = MAX(hdr->key_generation, hdr->key_generation_2);
	mKaekIndex = hdr->key_area_encryption_key_index;
	mContentSize = *hdr->content_size;
	mProgramId = *hdr->program_id;
	mContentIndex = *hdr->content_index;
	mSdkAddonVersion = *hdr->sdk_addon_version;
	memcpy(mRightsId, hdr->rights_id, nca::kRightsIdLen);

	for (size_t i = 0; i < nca::kPartitionNum; i++)
	{
		// skip sections that don't exist
		if (hdr->partition[i].enabled == 0) continue;

		// add high level struct
		mPartitions.addElement({(byte_t)i, blockNumToSize(hdr->partition[i].start.get()), blockNumToSize(hdr->partition[i].end.get() - hdr->partition[i].start.get()), hdr->partition_hash[i] });
	}

	for (size_t i = 0; i < nca::kAesKeyNum; i++)
	{
		mEncAesKeys.addElement(hdr->enc_aes_key[i]);
	}
}

void nx::NcaHeader::clear()
{
	mFormatVersion = NCA3_FORMAT;
	mDistributionType = nca::DIST_DOWNLOAD;
	mContentType = nca::TYPE_PROGRAM;
	mKeyGeneration = 0;
	mKaekIndex = 0;
	mContentSize = 0;
	mProgramId = 0;
	mContentIndex = 0;
	mSdkAddonVersion = 0;

	mPartitions.clear();
	mEncAesKeys.clear();
}

nx::NcaHeader::FormatVersion nx::NcaHeader::getFormatVersion() const
{
	return mFormatVersion;
}

void nx::NcaHeader::setFormatVersion(FormatVersion version)
{
	mFormatVersion = version;
}

nx::nca::DistributionType nx::NcaHeader::getDistributionType() const
{
	return mDistributionType;
}

void nx::NcaHeader::setDistributionType(nca::DistributionType type)
{
	mDistributionType = type;
}

nx::nca::ContentType nx::NcaHeader::getContentType() const
{
	return mContentType;
}

void nx::NcaHeader::setContentType(nca::ContentType type)
{
	mContentType = type;
}

byte_t nx::NcaHeader::getKeyGeneration() const
{
	return mKeyGeneration;
}

void nx::NcaHeader::setKeyGeneration(byte_t gen)
{
	mKeyGeneration = gen;
}

byte_t nx::NcaHeader::getKaekIndex() const
{
	return mKaekIndex;
}

void nx::NcaHeader::setKaekIndex(byte_t index)
{
	mKaekIndex = index;
}

uint64_t NcaHeader::getContentSize() const
{
	return mContentSize;
}

void NcaHeader::setContentSize(uint64_t size)
{
	mContentSize = size;
}

uint64_t NcaHeader::getProgramId() const
{
	return mProgramId;
}

void NcaHeader::setProgramId(uint64_t program_id)
{
	mProgramId = program_id;
}

uint32_t nx::NcaHeader::getContentIndex() const
{
	return mContentIndex;
}

void nx::NcaHeader::setContentIndex(uint32_t index)
{
	mContentIndex = index;
}

uint32_t nx::NcaHeader::getSdkAddonVersion() const
{
	return mSdkAddonVersion;
}

void nx::NcaHeader::setSdkAddonVersion(uint32_t version)
{
	mSdkAddonVersion = version;
}

bool nx::NcaHeader::hasRightsId() const
{
	bool rightsIdIsSet = false;

	for (size_t i = 0; i < nca::kRightsIdLen; i++)
	{
		if (mRightsId[i] != 0)
			rightsIdIsSet = true;
	}

	return rightsIdIsSet;
}

const byte_t* nx::NcaHeader::getRightsId() const
{
	return mRightsId;
}

void nx::NcaHeader::setRightsId(const byte_t* rights_id)
{
	memcpy(mRightsId, rights_id, nca::kRightsIdLen);
}

const fnd::List<NcaHeader::sPartition>& NcaHeader::getPartitions() const
{
	return mPartitions;
}

void NcaHeader::setPartitions(const fnd::List<NcaHeader::sPartition>& partitions)
{
	mPartitions = partitions;
	if (mPartitions.getSize() >= nca::kPartitionNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA partitions");
	}
}

const fnd::List<crypto::aes::sAes128Key>& NcaHeader::getEncAesKeys() const
{
	return mEncAesKeys;
}

void NcaHeader::setEncAesKeys(const fnd::List<crypto::aes::sAes128Key>& keys)
{
	mEncAesKeys = keys;
}

uint64_t NcaHeader::blockNumToSize(uint32_t block_num) const
{
	return block_num*nca::kSectorSize;
}

uint32_t NcaHeader::sizeToBlockNum(uint64_t real_size) const
{
	return align(real_size, nca::kSectorSize) / nca::kSectorSize;
}

bool NcaHeader::isEqual(const NcaHeader & other) const
{
	return (mDistributionType == other.mDistributionType) \
		&& (mContentType == other.mContentType) \
		&& (mKeyGeneration == other.mKeyGeneration) \
		&& (mKaekIndex == other.mKaekIndex) \
		&& (mContentSize == other.mContentSize) \
		&& (mProgramId == other.mProgramId) \
		&& (mContentIndex == other.mContentIndex) \
		&& (mSdkAddonVersion == other.mSdkAddonVersion) \
		&& (mPartitions == other.mPartitions) \
		&& (mEncAesKeys == other.mEncAesKeys);
}

void NcaHeader::copyFrom(const NcaHeader & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mBinaryBlob.clear();
		mDistributionType = other.mDistributionType;
		mContentType = other.mContentType;
		mKeyGeneration = other.mKeyGeneration;
		mKaekIndex = other.mKaekIndex;
		mContentSize = other.mContentSize;
		mProgramId = other.mProgramId;
		mContentIndex = other.mContentIndex;
		mSdkAddonVersion = other.mSdkAddonVersion;
		mPartitions = other.mPartitions;
		mEncAesKeys = other.mEncAesKeys;
	}
}

NcaHeader::NcaHeader()
{
	clear();
}

NcaHeader::NcaHeader(const NcaHeader & other)
{
	copyFrom(other);
}

NcaHeader::NcaHeader(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool NcaHeader::operator==(const NcaHeader & other) const
{
	return isEqual(other);
}

bool NcaHeader::operator!=(const NcaHeader & other) const
{
	return !isEqual(other);
}

void NcaHeader::operator=(const NcaHeader & other)
{
	this->importBinary(other.getBytes(), other.getSize());
}

const byte_t * NcaHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t NcaHeader::getSize() const
{
	return mBinaryBlob.getSize();
}