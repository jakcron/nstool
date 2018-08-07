#pragma once
#include <fnd/types.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace npdm
	{
		static const uint32_t kNpdmStructMagic = _MAKE_STRUCT_MAGIC_U32("META");
		static const size_t kNameMaxLen = 0x10;
		static const size_t kProductCodeMaxLen = 0x10;
		static const uint32_t kMaxPriority = BIT(6) - 1;
		static const size_t kSectionAlignSize = 0x10;
		static const uint32_t kDefaultMainThreadStackSize = 4096;

		enum InstructionType
		{
			INSTR_32BIT,
			INSTR_64BIT,
		};

		enum ProcAddrSpaceType
		{
			ADDR_SPACE_64BIT = 1,
			ADDR_SPACE_32BIT,
			ADDR_SPACE_32BIT_NO_RESERVED,
		};
	}
#pragma pack(push,1)

	struct sNpdmHeader
	{
		le_uint32_t st_magic;
		byte_t reserved_0[8];
		byte_t flags;
		byte_t reserved_1;
		byte_t main_thread_priority;
		byte_t main_thread_cpu_id;
		byte_t reserved_2[8];
		le_uint32_t version;
		le_uint32_t main_thread_stack_size;
		char name[npdm::kNameMaxLen]; // important
		char product_code[npdm::kProductCodeMaxLen]; // can be empty
		byte_t reserved_3[48];
		// Access Control Info
		struct sNpdmSection
		{
			le_uint32_t offset;
			le_uint32_t size;
		} aci, acid;
	};

#pragma pack(pop)
}
}