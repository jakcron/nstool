#pragma once
#include <fnd/types.h>

namespace nn
{
namespace hac
{
	namespace romfs
	{
		static const uint64_t kRomfsHeaderAlign = 0x200;
		static const uint32_t kInvalidAddr = 0xffffffff;

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

	struct sRomfsDirEntry
	{
		le_uint32_t parent;
		le_uint32_t sibling;
		le_uint32_t child;
		le_uint32_t file;
		le_uint32_t hash;
		le_uint32_t name_size;
		char* name() { return ((char*)(this)) + sizeof(sRomfsDirEntry); }
		const char* name() const { return ((char*)(this)) + sizeof(sRomfsDirEntry); }
	};

	struct sRomfsFileEntry
	{
		le_uint32_t parent;
		le_uint32_t sibling;
		le_uint64_t offset;
		le_uint64_t size;
		le_uint32_t hash;
		le_uint32_t name_size;
		char* name() { return ((char*)(this)) + sizeof(sRomfsFileEntry); }
		const char* name() const { return ((char*)(this)) + sizeof(sRomfsFileEntry); }
	};
#pragma pack(pop)
}
}