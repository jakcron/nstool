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

		static const uint64_t kRomfsHeaderAlign = 0x200;
		static const uint32_t kInvalidAddr = 0xffffffff;
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

	struct sRomfsDirEntry
	{
		le_uint32_t parent;
		le_uint32_t sibling;
		le_uint32_t child;
		le_uint32_t file;
		le_uint32_t hash;
		le_uint32_t name_size;
		char name[];
	};

	struct sRomfsFileEntry
	{
		le_uint32_t parent;
		le_uint32_t sibling;
		le_uint64_t offset;
		le_uint64_t size;
		le_uint32_t hash;
		le_uint32_t name_size;
		char name[];
	};
#pragma pack(pop)
}
