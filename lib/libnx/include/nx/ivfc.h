#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	// Also known as HierarchicalIntegrity
	namespace ivfc
	{
		const std::string kIvfcSig = "IVFC";
		static const size_t kMaxIvfcLevel = 4;
		static const uint32_t kIvfcId = 0x20000;
	}
	
#pragma pack(push,1)
	struct sIvfcHeader
	{
		char signature[4];
		le_uint32_t id;
		le_uint32_t master_hash_size;
		le_uint32_t level_num;
		struct sIvfcLevelHeader
		{
			uint64_t logical_offset;
			uint64_t hash_data_size;
			uint32_t block_size;
			byte_t reserved[4];
		} level_header[ivfc::kMaxIvfcLevel];
		byte_t unk_0xA0[0x20];
		byte_t master_hash[0x20];
	};
#pragma pack(pop)
}
