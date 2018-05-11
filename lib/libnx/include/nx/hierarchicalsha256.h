#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	namespace hierarchicalsha256
	{
		static const size_t kDefaultLevelNum = 2;
	}
	
#pragma pack(push,1)
	struct sHierarchicalSha256Header
	{
		crypto::sha::sSha256Hash master_hash;
		le_uint32_t hash_block_size;
		le_uint32_t hash_level_num;
		struct sLayout
		{
			le_uint64_t offset;
			le_uint64_t size;
		} hash_data, hash_target;
	};
#pragma pack(pop)
}
