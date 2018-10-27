#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>
#include <nn/hac/define/macro.h>

namespace nn
{
namespace hac
{
	namespace kip
	{
		static const uint32_t kKipStructMagic = _MAKE_STRUCT_MAGIC_U32("KIP1");
		static const size_t kNameMaxLen = 0xC;
		static const size_t kKernCapabilityNum = 0x20;
		static const size_t kKernCapabilitySize = kKernCapabilityNum * sizeof(uint32_t);

		enum ProcessCategory
		{
			PROCCAT_REGULAR,
			PROCCAT_KERNAL_KIP
		};

		enum HeaderFlags
		{
			FLAG_TEXT_COMPRESS,
			FLAG_RO_COMPRESS,
			FLAG_DATA_COMPRESS,
			FLAG_INSTRUCTION_64BIT,
			FLAG_ADDR_SPACE_64BIT,
			FLAG_USE_SYSTEM_POOL_PARTITION
		};
	}
	
#pragma pack(push,1)
	struct sKipCodeSegment
	{
		le_uint32_t memory_offset;
		le_uint32_t memory_size;
		le_uint32_t file_size;
	};

	struct sKipHeader
	{
		le_uint32_t st_magic;
		char name[kip::kNameMaxLen];
		le_uint64_t title_id;
		le_uint32_t process_category;
		byte_t main_thread_priority;
		byte_t main_thread_cpu_id;
		byte_t reserved_01;
		byte_t flags;
		sKipCodeSegment text;
		byte_t reserved_02[4];
		sKipCodeSegment ro;
		le_uint32_t main_thread_stack_size;
		sKipCodeSegment data;
		byte_t reserved_03[4];
		sKipCodeSegment bss;
		byte_t reserved_04[4];
		byte_t reserved_05[0x20];
		byte_t capabilities[kip::kKernCapabilitySize];
	};
#pragma pack(pop)
}
}