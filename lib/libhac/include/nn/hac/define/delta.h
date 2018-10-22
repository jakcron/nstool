#pragma once
#include <fnd/types.h>
#include <nn/hac/define/macro.h>

namespace nn
{
namespace hac
{
	namespace delta
	{
		static const uint32_t kDeltaStructMagic = _MAKE_STRUCT_MAGIC_U32("NDV0");
	}
#pragma pack(push,1)
	struct sDeltaHeader
	{
		le_uint32_t st_magic;
		byte_t reserved_00[4];
		le_uint64_t source_size;
		le_uint64_t destination_size;
		le_uint64_t header_size;
		le_uint64_t body_size;
		byte_t reserved_01[0x18];
	};
#pragma pack(pop)
}
}
