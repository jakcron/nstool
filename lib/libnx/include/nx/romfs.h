#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	namespace romfs
	{
		enum HeaderSectionIndex
		{
			DIR_HASHMAP_TABLE,
			DIR_NODE_TABLE,
			FILE_HASHMAP_TABLE,
			FILE_NODE_TABLE,

			SECTION_NUM
		};
	}
	
#pragma pack(push,1)
	struct sRomfsHeader
	{
		le_uint64_t header_size;
		struct sSection
		{
			le_uint64_t offset;
			le_uint64_t size;
		} sections[romfs::SECTION_NUM];
		le_uint64_t data_offset;
	};
#pragma pack(pop)
}
