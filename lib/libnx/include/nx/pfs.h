#include <fnd/types.h>
#include <crypto/sha.h>
#include <nx/macro.h>

namespace nx
{
	namespace pfs
	{
		static const uint32_t kPfsSig = _MAKE_STRUCT_SIGNATURE("PFS0");
		static const uint32_t kHashedPfsSig = _MAKE_STRUCT_SIGNATURE("HFS0");
		static const size_t kHeaderAlign = 64;
	}
	
#pragma pack(push,1)
	struct sPfsHeader
	{
		le_uint32_t signature;
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
		crypto::sha::sSha256Hash hash;
	}; // sizeof(0x40)
#pragma pack(pop)
}
