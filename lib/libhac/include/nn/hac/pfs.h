#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace pfs
	{
		static const uint32_t kPfsStructMagic = _MAKE_STRUCT_MAGIC_U32("PFS0");
		static const uint32_t kHashedPfsStructMagic = _MAKE_STRUCT_MAGIC_U32("HFS0");
		static const size_t kHeaderAlign = 64;
	}
	
#pragma pack(push,1)
	struct sPfsHeader
	{
		le_uint32_t st_magic;
		le_uint32_t file_num;
		le_uint32_t name_table_size;
		byte_t padding[4];
	};

	struct sPfsFile
	{
		le_uint64_t data_offset;
		le_uint64_t size;
		le_uint32_t name_offset;
		byte_t padding[4];
	}; // sizeof(0x18)

	struct sHashedPfsFile
	{
		le_uint64_t data_offset;
		le_uint64_t size;
		le_uint32_t name_offset;
		le_uint32_t hash_protected_size;
		byte_t padding[8];
		fnd::sha::sSha256Hash hash;
	}; // sizeof(0x40)
#pragma pack(pop)
}
}