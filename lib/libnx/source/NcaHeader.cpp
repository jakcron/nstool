#include <nx/NcaHeader.h>

using namespace nx;


void NcaHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sNcaHeader));
	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	
	switch(mFormatVersion)
	{
	case (NCA2_FORMAT):
		strncpy(hdr->signature, kNca2Sig.c_str(), 4);
		break;
	case (NCA3_FORMAT):
		strncpy(hdr->signature, kNca3Sig.c_str(), 4);
		break;
	default:
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}
	hdr->distribution_type = mDistributionType;
	hdr->content_type = mContentType;
	hdr->crypto_type = mCryptoType;
	hdr->key_area_encryption_key_index = mKaekIndex;
	hdr->nca_size = mNcaSize;
	hdr->program_id = mProgramId;
	hdr->content_index = mContentIndex;
	hdr->sdk_addon_version = mSdkAddonVersion;
	hdr->crypto_type_2 = 0;

	// TODO: properly reconstruct NCA layout? atm in hands of user

	for (size_t i = 0; i < mSections.getSize(); i++)
	{
		// determine section index
		u8 section = mSections.getSize() - 1 - i;

		hdr->section[section].start = sizeToBlockNum(mSections[i].offset);
		hdr->section[section].end = (sizeToBlockNum(mSections[i].offset) + sizeToBlockNum(mSections[i].size));
		hdr->section[section].enabled = true;
		hdr->section_hash[section] = mSections[i].hash;
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		hdr->enc_aes_key[i] = mEncAesKeys[i];
	}
}

void NcaHeader::importBinary(const u8 * bytes, size_t len)
{
	if (len < sizeof(sNcaHeader))
	{
		throw fnd::Exception(kModuleName, "NCA header size is too small");
	}

	clear();

	mBinaryBlob.alloc(sizeof(sNcaHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, sizeof(sNcaHeader));

	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	if (memcmp(hdr->signature, kNca2Sig.c_str(), 4) == 0)
	{
		mFormatVersion = NCA2_FORMAT;
	}
	else if (memcmp(hdr->signature, kNca3Sig.c_str(), 4) == 0)
	{
		mFormatVersion = NCA3_FORMAT;
	}
	else
	{
		throw fnd::Exception(kModuleName, "NCA header corrupt");
	}

	mDistributionType = (DistributionType)hdr->distribution_type;
	mContentType = (ContentType)hdr->content_type;
	mCryptoType = MAX(hdr->crypto_type, hdr->crypto_type_2);
	mKaekIndex = hdr->key_area_encryption_key_index;
	mNcaSize = *hdr->nca_size;
	mProgramId = *hdr->program_id;
	mContentIndex = *hdr->content_index;
	mSdkAddonVersion = *hdr->sdk_addon_version;

	for (size_t i = 0; i < kSectionNum; i++)
	{
		// determine section index
		u8 section = kSectionNum - 1 - i;

		// skip sections that don't exist
		if (*hdr->section[section].start == 0 && *hdr->section[section].end == 0) continue;

		// add high level struct
		mSections.addElement({ blockNumToSize(*hdr->section[section].start), blockNumToSize(hdr->section[section].end.get() - hdr->section[section].start.get()), hdr->section_hash[section] });
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		mEncAesKeys.addElement(hdr->enc_aes_key[i]);
	}
}

void nx::NcaHeader::clear()
{
	mFormatVersion = NCA3_FORMAT;
	mDistributionType = DIST_DOWNLOAD;
	mContentType = TYPE_PROGRAM;
	mCryptoType = 0;
	mKaekIndex = 0;
	mNcaSize = 0;
	mProgramId = 0;
	mContentIndex = 0;
	mSdkAddonVersion = 0;
	mSections.clear();
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

nx::NcaHeader::DistributionType nx::NcaHeader::getDistributionType() const
{
	return mDistributionType;
}

void nx::NcaHeader::setDistributionType(DistributionType type)
{
	mDistributionType = type;
}

nx::NcaHeader::ContentType nx::NcaHeader::getContentType() const
{
	return mContentType;
}

void nx::NcaHeader::setContentType(ContentType type)
{
	mContentType = type;
}

byte_t nx::NcaHeader::getCryptoType() const
{
	return mCryptoType;
}

void nx::NcaHeader::setCryptoType(byte_t type)
{
	mCryptoType = type;
}

byte_t nx::NcaHeader::getKaekIndex() const
{
	return mKaekIndex;
}

void nx::NcaHeader::setKaekIndex(byte_t index)
{
	mKaekIndex = index;
}

u64 NcaHeader::getNcaSize() const
{
	return mNcaSize;
}

void NcaHeader::setNcaSize(u64 size)
{
	mNcaSize = size;
}

u64 NcaHeader::getProgramId() const
{
	return mProgramId;
}

void NcaHeader::setProgramId(u64 program_id)
{
	mProgramId = program_id;
}

u32 nx::NcaHeader::getContentIndex() const
{
	return mContentIndex;
}

void nx::NcaHeader::setContentIndex(u32 index)
{
	mContentIndex = index;
}

u32 nx::NcaHeader::getSdkAddonVersion() const
{
	return mSdkAddonVersion;
}

void nx::NcaHeader::setSdkAddonVersion(u32 version)
{
	mSdkAddonVersion = version;
}

const fnd::List<NcaHeader::sSection>& NcaHeader::getSections() const
{
	return mSections;
}

void NcaHeader::addSection(const sSection & section)
{
	if (mSections.getSize() >= kSectionNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA sections");
	}
	mSections.addElement(section);
}

const fnd::List<crypto::aes::sAes128Key>& NcaHeader::getEncAesKeys() const
{
	return mEncAesKeys;
}

void NcaHeader::addEncAesKey(const crypto::aes::sAes128Key & key)
{
	if (mEncAesKeys.getSize() >= kAesKeyNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA aes keys");
	}

	mEncAesKeys.addElement(key);
}

u64 NcaHeader::blockNumToSize(u32 block_num) const
{
	return block_num*kBlockSize;
}

u32 NcaHeader::sizeToBlockNum(u64 real_size) const
{
	return align(real_size, kBlockSize)/kBlockSize;
}

bool NcaHeader::isEqual(const NcaHeader & other) const
{
	return (mDistributionType == other.mDistributionType) \
		&& (mContentType == other.mContentType) \
		&& (mCryptoType == other.mCryptoType) \
		&& (mKaekIndex == other.mKaekIndex) \
		&& (mNcaSize == other.mNcaSize) \
		&& (mProgramId == other.mProgramId) \
		&& (mContentIndex == other.mContentIndex) \
		&& (mSdkAddonVersion == other.mSdkAddonVersion) \
		&& (mSections == other.mSections) \
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
		mCryptoType = other.mCryptoType;
		mKaekIndex = other.mKaekIndex;
		mNcaSize = other.mNcaSize;
		mProgramId = other.mProgramId;
		mContentIndex = other.mContentIndex;
		mSdkAddonVersion = other.mSdkAddonVersion;
		mSections = other.mSections;
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

NcaHeader::NcaHeader(const u8 * bytes, size_t len)
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

const u8 * NcaHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t NcaHeader::getSize() const
{
	return mBinaryBlob.getSize();
}