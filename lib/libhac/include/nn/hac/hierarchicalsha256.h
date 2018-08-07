#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>

namespace nn
{
namespace hac
{
	namespace hierarchicalsha256
	{
		static const size_t kDefaultLayerNum = 2;
		static const size_t kMaxLayerNum = 2;
	}
	
#pragma pack(push,1)
	struct sHierarchicalSha256Header
	{
		fnd::sha::sSha256Hash master_hash;
		le_uint32_t hash_block_size;
		le_uint32_t layer_num;
		struct sLayer
		{
			le_uint64_t offset;
			le_uint64_t size;
		} layer[hierarchicalsha256::kMaxLayerNum];
	};
#pragma pack(pop)
}
}