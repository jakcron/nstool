#include "NcaHeader.h"
#include <fnd/exception.h>

using namespace nx;


void NcaHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sNcaHeader));
	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	hdr->set_signature(kNcaSig.c_str());
	hdr->set_block_size(kDefaultBlockSize);
	hdr->set_nca_size(mNcaSize);
	hdr->set_program_id(mProgramId);
	hdr->set_unk0(mUnk0);

	// TODO: properly reconstruct NCA layout? atm in hands of user

	for (size_t i = 0; i < mSections.getSize(); i++)
	{
		// determine section index
		u8 section = mSections.getSize() - 1 - i;

		hdr->section(section).set_start(sizeToBlockNum(mSections[i].offset));
		hdr->section(section).set_end(sizeToBlockNum(mSections[i].offset) + sizeToBlockNum(mSections[i].size));
		hdr->section(section).set_key_type(mSections[i].key_type);
		hdr->section_hash(section) = mSections[i].hash;
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		hdr->aes_key(i) = mAesKeys[i];
	}
}

void NcaHeader::importBinary(const u8 * bytes)
{
	clearVariables();
	
	mBinaryBlob.alloc(sizeof(sNcaHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, sizeof(sNcaHeader));

	sNcaHeader* hdr = (sNcaHeader*)mBinaryBlob.getBytes();

	if (memcmp(hdr->signature(), kNcaSig.c_str(), 4) != 0)
	{
		throw fnd::Exception(kModuleName, "NCA header corrupt");
	}
	
	mBlockSize = hdr->block_size();
	mNcaSize = hdr->nca_size();
	mProgramId = hdr->program_id();
	mUnk0 = hdr->unk0();

	for (size_t i = 0; i < kSectionNum; i++)
	{
		// determine section index
		u8 section = kSectionNum - 1 - i;
		
		// skip sections that don't exist
		if (hdr->section(section).start() == 0 && hdr->section(section).end() == 0) continue;
		
		// add high level struct
		mSections.addElement({ blockNumToSize(hdr->section(section).start()), blockNumToSize(hdr->section(section).end() - hdr->section(section).start()), hdr->section(section).key_type(), hdr->section_hash(section) });
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		mAesKeys.addElement(hdr->aes_key(i));
	}
}

void NcaHeader::importBinary(const u8 * bytes, size_t len)
{
	if (len < sizeof(sNcaHeader))
	{
		throw fnd::Exception(kModuleName, "NCA header size is too small");
	}
	importBinary(bytes);
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

u32 NcaHeader::getUnk() const
{
	return mUnk0;
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

const fnd::List<crypto::aes::sAes128Key>& NcaHeader::getAesKeys() const
{
	return mAesKeys;
}

void NcaHeader::addKey(const crypto::aes::sAes128Key & key)
{
	if (mAesKeys.getSize() >= kAesKeyNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA aes keys");
	}

	mAesKeys.addElement(key);
}

void NcaHeader::clearVariables()
{
	mBlockSize = 0;
	mNcaSize = 0;
	mProgramId = 0;
	mUnk0 = 0;
	mSections.clear();
	mAesKeys.clear();
}

u64 NcaHeader::blockNumToSize(u32 block_num) const
{
	return block_num*mBlockSize;
}

u32 NcaHeader::sizeToBlockNum(u64 real_size) const
{
	return align(real_size, mBlockSize)/mBlockSize;
}

bool NcaHeader::isEqual(const NcaHeader & other) const
{
	return (mBlockSize == other.mBlockSize) \
		&& (mNcaSize == other.mNcaSize) \
		&& (mProgramId == other.mProgramId) \
		&& (mUnk0 == other.mUnk0) \
		&& (mSections == other.mSections) \
		&& (mAesKeys == other.mAesKeys);
}

void NcaHeader::copyFrom(const NcaHeader & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		this->mBinaryBlob.clear();
		mBlockSize = other.mBlockSize;
		mNcaSize = other.mNcaSize;
		mProgramId = other.mProgramId;
		mUnk0 = other.mUnk0;
		mSections = other.mSections;
		mAesKeys = other.mAesKeys;
	}
}

NcaHeader::NcaHeader()
{
	clearVariables();
}

NcaHeader::NcaHeader(const NcaHeader & other)
{
	copyFrom(other);
}

NcaHeader::NcaHeader(const u8 * bytes)
{
	importBinary(bytes);
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