#pragma once
#include <fnd/types.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace hierarchicalintegrity
	{
		static const uint32_t kStructMagic = _MAKE_STRUCT_MAGIC_U32("IVFC");
		static const uint32_t kRomfsTypeId = 0x20000;
		static const size_t kDefaultLayerNum = 6;
		static const size_t kHeaderAlignLen = 0x20;
	}
	
#pragma pack(push,1)
	struct sHierarchicalIntegrityHeader
	{
		le_uint32_t st_magic;
		le_uint32_t type_id;
		le_uint32_t master_hash_size;
		le_uint32_t layer_num;
	};

	struct sHierarchicalIntegrityLayerInfo // sizeof(0x18)
	{
		le_uint64_t offset;
		le_uint64_t size;
		le_uint32_t block_size;
		byte_t reserved[4];
	};
#pragma pack(pop)
}
}