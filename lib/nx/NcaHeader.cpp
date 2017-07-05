#include "NcaHeader.h"
#include <fnd/exception.h>

void NcaHeader::exportBinary()
{
	// TODO: implement export
}

void NcaHeader::importBinary(const u8 * bytes)
{
	clearVariables();
	
	binary_blob_.alloc(sizeof(sNcaHeader));
	memcpy(binary_blob_.data(), bytes, sizeof(sNcaHeader));

	sNcaHeader* hdr = (sNcaHeader*)binary_blob_.data();

	if (memcmp(hdr->signature(), kNcaSig.c_str(), 4) != 0)
	{
		throw fnd::Exception(kModuleName, "NCA header corrupt");
	}
	
	block_size_ = hdr->block_size();
	nca_size_ = hdr->nca_size();
	program_id_ = hdr->program_id();
	unk0_ = hdr->unk0();

	for (size_t i = 0; i < kSectionNum; i++)
	{
		// determine section index
		u8 section = kSectionNum - 1 - i;
		
		// skip sections that don't exist
		if (hdr->section(section).start() == 0 && hdr->section(section).end() == 0) continue;
		
		// add high level struct
		sections_.push_back({ hdr->section(section).start(), hdr->section(section).end(), blockNumToSize(hdr->section(section).start()), blockNumToSize(hdr->section(section).end()- hdr->section(section).start()), hdr->section(section).key_type(), hdr->section_hash(section) });
	}

	for (size_t i = 0; i < kAesKeyNum; i++)
	{
		aes_keys_.push_back(hdr->aes_key(i));
	}
}

u64 NcaHeader::getNcaSize() const
{
	return nca_size_;
}

void NcaHeader::setNcaSize(u64 size)
{
	nca_size_ = size;
}

u64 NcaHeader::getProgramId() const
{
	return program_id_;
}

void NcaHeader::setProgramId(u64 program_id)
{
	program_id_ = program_id;
}

u32 NcaHeader::getUnk() const
{
	return unk0_;
}

const std::vector<NcaHeader::sSection>& NcaHeader::getSections() const
{
	return sections_;
}

void NcaHeader::addSection(const sSection & section)
{
	if (sections_.size() >= kSectionNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA sections");
	}
	sections_.push_back(section);
}

const std::vector<crypto::aes::sAes128Key>& NcaHeader::getAesKeys() const
{
	return aes_keys_;
}

void NcaHeader::addKey(const crypto::aes::sAes128Key & key)
{
	if (aes_keys_.size() >= kAesKeyNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA aes keys");
	}

	aes_keys_.push_back(key);
}

void NcaHeader::clearVariables()
{
	block_size_ = 0;
	nca_size_ = 0;
	program_id_ = 0;
	unk0_ = 0;
	sections_.clear();
	aes_keys_.clear();
}

u64 NcaHeader::blockNumToSize(u32 block_num) const
{
	return block_num*block_size_;
}

u32 NcaHeader::sizeToBlockNum(u64 real_size) const
{
	return align(real_size, block_size_)/block_size_;
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

const u8 * NcaHeader::getBytes() const
{
	return binary_blob_.data();
}

size_t NcaHeader::getSize() const
{
	return binary_blob_.size();
}