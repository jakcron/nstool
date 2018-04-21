#include <string>
#include <fnd/types.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	namespace pfs
	{
		const std::string kPfsSig = "PFS0";
		const std::string kHashedPfsSig = "HFS0";
		static const size_t kHeaderAlign = 64;
	}
	
#pragma pack(push,1)
	struct sPfsHeader
	{
		char signature[4];
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
