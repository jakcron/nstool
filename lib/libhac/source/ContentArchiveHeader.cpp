#include <nn/hac/ContentArchiveHeader.h>

nn::hac::ContentArchiveHeader::ContentArchiveHeader()
{
	mRightsId.alloc(nca::kRightsIdLen);
	mKeyArea.alloc(nca::kKeyAreaSize);
	clear();
}

nn::hac::ContentArchiveHeader::ContentArchiveHeader(const ContentArchiveHeader & other) :
	ContentArchiveHeader()
{
	*this = other;
}

bool nn::hac::ContentArchiveHeader::operator==(const ContentArchiveHeader & other) const
{
	return (mFormatVersion == other.mFormatVersion) \
		&& (mDistributionType == other.mDistributionType) \
		&& (mContentType == other.mContentType) \
		&& (mKeyGeneration == other.mKeyGeneration) \
		&& (mKaekIndex == other.mKaekIndex) \
		&& (mContentSize == other.mContentSize) \
		&& (mProgramId == other.mProgramId) \
		&& (mContentIndex == other.mContentIndex) \
		&& (mSdkAddonVersion == other.mSdkAddonVersion) \
		&& (mRightsId == other.mRightsId) \
		&& (mPartitionEntryList == other.mPartitionEntryList) \
		&& (mKeyArea == other.mKeyArea);
}

bool nn::hac::ContentArchiveHeader::operator!=(const ContentArchiveHeader & other) const
{
	return !(*this == other);
}

void nn::hac::ContentArchiveHeader::operator=(const ContentArchiveHeader & other)
{
	mRawBinary = other.mRawBinary;
	mDistributionType = other.mDistributionType;
	mContentType = other.mContentType;
	mKeyGeneration = other.mKeyGeneration;
	mKaekIndex = other.mKaekIndex;
	mContentSize = other.mContentSize;
	mProgramId = other.mProgramId;
	mContentIndex = other.mContentIndex;
	mSdkAddonVersion = other.mSdkAddonVersion;
	mRightsId = other.mRightsId;
	mPartitionEntryList = other.mPartitionEntryList;
	mKeyArea = other.mKeyArea;
}

void nn::hac::ContentArchiveHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sContentArchiveHeader));
	sContentArchiveHeader* hdr = (sContentArchiveHeader*)mRawBinary.data();

	// set header magic
	switch(mFormatVersion)
	{
	case (nca::FORMAT_NCA2):
		hdr->st_magic = nca::kNca2StructMagic;
		break;
	case (nca::FORMAT_NCA3):
		hdr->st_magic = nca::kNca3StructMagic;
		break;
	default:
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}

	// set variables
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
	memcpy(hdr->rights_id, mRightsId.data(), nca::kRightsIdLen);
	memcpy(hdr->key_area, mKeyArea.data(), nca::kKeyAreaSize);

	for (size_t i = 0; i < mPartitionEntryList.size(); i++)
	{
		byte_t index = mPartitionEntryList[i].header_index;
		
		if (index >= nca::kPartitionNum) continue;

		hdr->partition_entry[index].start_blk = sizeToBlockNum(mPartitionEntryList[index].offset);
		hdr->partition_entry[index].end_blk = (sizeToBlockNum(mPartitionEntryList[index].offset) + sizeToBlockNum(mPartitionEntryList[index].size));
		hdr->partition_entry[index].enabled = true;
		hdr->fs_header_hash[index] = mPartitionEntryList[i].fs_header_hash;	
	}
}

void nn::hac::ContentArchiveHeader::fromBytes(const byte_t * data, size_t len)
{
	if (len < sizeof(sContentArchiveHeader))
	{
		throw fnd::Exception(kModuleName, "ContentArchive header size is too small");
	}

	clear();

	mRawBinary.alloc(sizeof(sContentArchiveHeader));
	memcpy(mRawBinary.data(), data, sizeof(sContentArchiveHeader));

	sContentArchiveHeader* hdr = (sContentArchiveHeader*)mRawBinary.data();

	// check magic
	switch(hdr->st_magic.get())
	{
		case (nca::kNca2StructMagic) :
			mFormatVersion = nca::FORMAT_NCA2;
			break;
		case (nca::kNca3StructMagic) :
			mFormatVersion = nca::FORMAT_NCA3;
			break;
		throw fnd::Exception(kModuleName, "ContentArchive header corrupt (unrecognised header magic).");
	}

	// variables
	mDistributionType = (nca::DistributionType)hdr->distribution_type;
	mContentType = (nca::ContentType)hdr->content_type;
	mKeyGeneration = _MAX(hdr->key_generation, hdr->key_generation_2);
	mKaekIndex = hdr->key_area_encryption_key_index;
	mContentSize = *hdr->content_size;
	mProgramId = *hdr->program_id;
	mContentIndex = *hdr->content_index;
	mSdkAddonVersion = *hdr->sdk_addon_version;
	memcpy(mRightsId.data(), hdr->rights_id, nca::kRightsIdLen);
	memcpy(mKeyArea.data(), hdr->key_area, nca::kKeyAreaSize);

	for (size_t i = 0; i < nca::kPartitionNum; i++)
	{
		if (hdr->partition_entry[i].enabled)
		{
			mPartitionEntryList.addElement({(byte_t)i, blockNumToSize(hdr->partition_entry[i].start_blk.get()), blockNumToSize(hdr->partition_entry[i].end_blk.get() - hdr->partition_entry[i].start_blk.get()), hdr->fs_header_hash[i] });
		}
	}
}

const fnd::Vec<byte_t>& nn::hac::ContentArchiveHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::ContentArchiveHeader::clear()
{
	mFormatVersion = nca::FORMAT_NCA3;
	mDistributionType = nca::DIST_DOWNLOAD;
	mContentType = nca::TYPE_PROGRAM;
	mKeyGeneration = 0;
	mKaekIndex = 0;
	mContentSize = 0;
	mProgramId = 0;
	mContentIndex = 0;
	mSdkAddonVersion = 0;
	memset(mRightsId.data(), 0, mRightsId.size());
	mPartitionEntryList.clear();
	memset(mKeyArea.data(), 0, mKeyArea.size());
}

byte_t nn::hac::ContentArchiveHeader::getFormatVersion() const
{
	return mFormatVersion;
}

void nn::hac::ContentArchiveHeader::setFormatVersion(byte_t version)
{
	mFormatVersion = version;
}

nn::hac::nca::DistributionType nn::hac::ContentArchiveHeader::getDistributionType() const
{
	return mDistributionType;
}

void nn::hac::ContentArchiveHeader::setDistributionType(nca::DistributionType type)
{
	mDistributionType = type;
}

nn::hac::nca::ContentType nn::hac::ContentArchiveHeader::getContentType() const
{
	return mContentType;
}

void nn::hac::ContentArchiveHeader::setContentType(nca::ContentType type)
{
	mContentType = type;
}

byte_t nn::hac::ContentArchiveHeader::getKeyGeneration() const
{
	return mKeyGeneration;
}

void nn::hac::ContentArchiveHeader::setKeyGeneration(byte_t gen)
{
	mKeyGeneration = gen;
}

byte_t nn::hac::ContentArchiveHeader::getKeyAreaEncryptionKeyIndex() const
{
	return mKaekIndex;
}

void nn::hac::ContentArchiveHeader::setKeyAreaEncryptionKeyIndex(byte_t index)
{
	mKaekIndex = index;
}

uint64_t nn::hac::ContentArchiveHeader::getContentSize() const
{
	return mContentSize;
}

void nn::hac::ContentArchiveHeader::setContentSize(uint64_t size)
{
	mContentSize = size;
}

uint64_t nn::hac::ContentArchiveHeader::getProgramId() const
{
	return mProgramId;
}

void nn::hac::ContentArchiveHeader::setProgramId(uint64_t program_id)
{
	mProgramId = program_id;
}

uint32_t nn::hac::ContentArchiveHeader::getContentIndex() const
{
	return mContentIndex;
}

void nn::hac::ContentArchiveHeader::setContentIndex(uint32_t index)
{
	mContentIndex = index;
}

uint32_t nn::hac::ContentArchiveHeader::getSdkAddonVersion() const
{
	return mSdkAddonVersion;
}

void nn::hac::ContentArchiveHeader::setSdkAddonVersion(uint32_t version)
{
	mSdkAddonVersion = version;
}

bool nn::hac::ContentArchiveHeader::hasRightsId() const
{
	bool rightsIdIsSet = false;

	for (size_t i = 0; i < nca::kRightsIdLen; i++)
	{
		if (mRightsId[i] != 0)
			rightsIdIsSet = true;
	}

	return rightsIdIsSet;
}

const byte_t* nn::hac::ContentArchiveHeader::getRightsId() const
{
	return mRightsId.data();
}

void nn::hac::ContentArchiveHeader::setRightsId(const byte_t* rights_id)
{
	memcpy(mRightsId.data(), rights_id, nca::kRightsIdLen);
}

const fnd::List<nn::hac::ContentArchiveHeader::sPartitionEntry>& nn::hac::ContentArchiveHeader::getPartitionEntryList() const
{
	return mPartitionEntryList;
}

void nn::hac::ContentArchiveHeader::setPartitionEntryList(const fnd::List<nn::hac::ContentArchiveHeader::sPartitionEntry>& partition_entry_list)
{
	mPartitionEntryList = partition_entry_list;

	// sanity check the list
	if (mPartitionEntryList.size() >= nca::kPartitionNum)
	{
		throw fnd::Exception(kModuleName, "Too many partitions");
	}
	for (size_t i = 0; i < mPartitionEntryList.size(); i++)
	{
		if (mPartitionEntryList[i].header_index >= nca::kPartitionNum)
		{
			throw fnd::Exception(kModuleName, "Illegal partition index");
		}
		for (size_t j = i+1; j < mPartitionEntryList.size(); j++)
		{
			if (mPartitionEntryList[i].header_index == mPartitionEntryList[j].header_index)
			{
				throw fnd::Exception(kModuleName, "Duplicated partition index");
			}
		}
	}
}

const byte_t* nn::hac::ContentArchiveHeader::getKeyArea() const
{
	return mKeyArea.data();
}

void nn::hac::ContentArchiveHeader::setKeyArea(const byte_t* key_area)
{
	memcpy(mKeyArea.data(), key_area, nca::kKeyAreaSize);
}

uint64_t nn::hac::ContentArchiveHeader::blockNumToSize(uint32_t block_num) const
{
	return block_num * nca::kSectorSize;
}

uint32_t nn::hac::ContentArchiveHeader::sizeToBlockNum(uint64_t real_size) const
{
	return (uint32_t)(align(real_size, nca::kSectorSize) / nca::kSectorSize);
}