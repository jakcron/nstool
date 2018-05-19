#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	// Also known as HierarchicalIntegrity
	namespace hierarchicalintegrity
	{
		const std::string kStructSig = "IVFC";
		static const size_t kMaxLayerNum = 7;
		static const uint32_t kTypeId = 0x20000;
		static const size_t kMaxMasterHashNum = 3;
	}
	
#pragma pack(push,1)
	struct sHierarchicalIntegrityHeader
	{
		char signature[4];
		le_uint32_t type_id;
		le_uint32_t master_hash_size;
		le_uint32_t layer_num;
		struct sLayer
		{
			le_uint64_t offset;
			le_uint64_t size;
			le_uint32_t block_size;
			byte_t reserved[4];
		} layer[hierarchicalintegrity::kMaxLayerNum];
		byte_t reserved_00[0x8];
		crypto::sha::sSha256Hash master_hash[hierarchicalintegrity::kMaxMasterHashNum];
	};
#pragma pack(pop)
}
