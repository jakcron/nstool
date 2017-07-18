#include "NcaHeader.h"
#include <fnd/exception.h>

using namespace nx;


void NcaHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sNcaHeader));
	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	hdr->set_signature(kNcaSig.c_str());
	hdr->set_distribution_type(mDistributionType);
	hdr->set_content_type(mContentType);
	hdr->set_key_generation(mEncryptionType);
	hdr->set_key_area_encryption_key_index(mKeyIndex);
	hdr->set_nca_size(mNcaSize);
	hdr->set_program_id(mProgramId);
	hdr->set_content_index(mContentIndex);
	hdr->set_sdk_addon_version(mSdkAddonVersion);

	// TODO: properly reconstruct NCA layout? atm in hands of user

	for (size_t i = 0; i < mSections.getSize(); i++)
	{
		// determine section index
		u8 section = mSections.getSize() - 1 - i;

		hdr->section(section).set_start(sizeToBlockNum(mSections[i].offset));
		hdr->section(section).set_end(sizeToBlockNum(mSections[i].offset) + sizeToBlockNum(mSections[i].size));
		hdr->section(section).set_enabled(1);
		hdr->section_hash(section) = mSections[i].hash;
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		hdr->enc_aes_key(i) = mEncAesKeys[i];
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

	if (memcmp(hdr->signature(), kNcaSig.c_str(), 4) != 0)
	{
		throw fnd::Exception(kModuleName, "NCA header corrupt");
	}

	mDistributionType = (DistributionType)hdr->distribution_type();
	mContentType = (ContentType)hdr->content_type();
	mEncryptionType = (EncryptionType)hdr->key_generation();
	mKeyIndex = (EncryptionKeyIndex)hdr->key_area_encryption_key_index();
	mNcaSize = hdr->nca_size();
	mProgramId = hdr->program_id();
	mContentIndex = hdr->content_index();
	mSdkAddonVersion = hdr->sdk_addon_version();

	for (size_t i = 0; i < kSectionNum; i++)
	{
		// determine section index
		u8 section = kSectionNum - 1 - i;

		// skip sections that don't exist
		if (hdr->section(section).start() == 0 && hdr->section(section).end() == 0) continue;

		EncryptionType encType = mEncryptionType;
		if (encType == CRYPT_AUTO)
		{
			if (mContentType == TYPE_PROGRAM && section == SECTION_LOGO)
			{
				encType = CRYPT_NONE;
			}
			else
			{
				encType = CRYPT_AESCTR;
			}
		}

		// add high level struct
		mSections.addElement({ blockNumToSize(hdr->section(section).start()), blockNumToSize(hdr->section(section).end() - hdr->section(section).start()), encType, hdr->section_hash(section) });
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		mEncAesKeys.addElement(hdr->enc_aes_key(i));
	}
}

void nx::NcaHeader::clear()
{
	mDistributionType = DIST_DOWNLOAD;
	mContentType = TYPE_PROGRAM;
	mEncryptionType = CRYPT_AUTO;
	mKeyIndex = KEY_DEFAULT;
	mNcaSize = 0;
	mProgramId = 0;
	mContentIndex = 0;
	mSdkAddonVersion = 0;
	mSections.clear();
	mEncAesKeys.clear();
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

nx::NcaHeader::EncryptionType nx::NcaHeader::getEncryptionType() const
{
	return mEncryptionType;
}

void nx::NcaHeader::setEncryptionType(EncryptionType type)
{
	mEncryptionType = type;
}

nx::NcaHeader::EncryptionKeyIndex nx::NcaHeader::getKeyIndex() const
{
	return mKeyIndex;
}

void nx::NcaHeader::setKeyIndex(EncryptionKeyIndex index)
{
	mKeyIndex = index;
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
		&& (mEncryptionType == other.mEncryptionType) \
		&& (mKeyIndex == other.mKeyIndex) \
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
		mEncryptionType = other.mEncryptionType;
		mKeyIndex = other.mKeyIndex;
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