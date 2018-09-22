#pragma once
#include <fnd/types.h>
#include <fnd/rsa.h>
#include <nn/ctr/macro.h>

namespace nn
{
	namespace ctr
	{
		namespace code
		{
			static const uint32_t kStructMagic = _MAKE_STRUCT_MAGIC_U32("CODE");
			static const uint32_t kCtrFormatIdent = _MAKE_STRUCT_MAGIC_U32("CTR0");
			static const size_t kCodeSegmentNum = 3;

			enum CodeSegmentIndex
			{
				TEXT_SEGMENT,
				RODATA_SEGMENT,
				DATA_SEGMENT,
			};
		}
#pragma pack(push,1)
		struct sCodeBinaryHeader
		{
			le_uint32_t st_magic;
			le_uint32_t format_ident;
			le_uint32_t flags;
			byte_t reserved_00[4];
			le_uint32_t code_bin_offset;
			le_uint32_t code_bin_size;
			le_uint32_t stack_size;
			le_uint32_t bss_size;
			struct sCodeSegment
			{
				le_uint32_t v_addr;
				le_uint32_t size;
				le_uint32_t page_num;
				le_uint32_t padding;
			} segments[code::kCodeSegmentNum];
		};
#pragma pack(pop)
	}
}