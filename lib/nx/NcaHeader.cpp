#include "NcaHeader.h"
#include <fnd/exception.h>

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

	for (size_t i = 0; i < mSections.size(); i++)
	{
		// determine section index
		u8 section = mSections.size() - 1 - i;

		hdr->section(section).set_start(mSections[i].start_blk);
		hdr->section(section).set_end(mSections[i].end_blk);
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
		mSections.push_back({ hdr->section(section).start(), hdr->section(section).end(), blockNumToSize(hdr->section(section).start()), blockNumToSize(hdr->section(section).end()- hdr->section(section).start()), hdr->section(section).key_type(), hdr->section_hash(section) });
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		mAesKeys.push_back(hdr->aes_key(i));
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

const std::vector<NcaHeader::sSection>& NcaHeader::getSections() const
{
	return mSections;
}

void NcaHeader::addSection(const sSection & section)
{
	if (mSections.size() >= kSectionNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA sections");
	}
	mSections.push_back(section);
}

const std::vector<crypto::aes::sAes128Key>& NcaHeader::getAesKeys() const
{
	return mAesKeys;
}

void NcaHeader::addKey(const crypto::aes::sAes128Key & key)
{
	if (mAesKeys.size() >= kAesKeyNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA aes keys");
	}

	mAesKeys.push_back(key);
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

NcaHeader::NcaHeader()
{
	clearVariables();
}

NcaHeader::NcaHeader(const NcaHeader & other)
{
	importBinary(other.getBytes());
}

NcaHeader::NcaHeader(const u8 * bytes)
{
	importBinary(bytes);
}

bool NcaHeader::operator==(const NcaHeader & other)
{
	return memcmp(this->getBytes(), other.getBytes(), this->getSize()) == 0;
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