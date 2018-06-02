#pragma once
#include <fnd/types.h>

namespace nx
{
	namespace dynsym
	{
		enum SpecialSectionIndex
		{
			SHN_UNDEF,
			SHN_LORESERVE = 65280,
			SHN_LOPROC = 65280,
			SHN_HIPROC = 65311,
			SHN_LOOS,
			SHN_HIOS = 65343,
			SHN_ABS = 65521,
			SHN_COMMON,
			SHN_HIRESERVE = 0xFFFF
		};

		enum SymbolType
		{
			STT_NOTYPE,
			STT_OBJECT,
			STT_FUNC,
			STT_SECTION,
			STT_FILE,
			STT_LOOS = 10,
			STT_HIOS = 12,
			STT_LOPROC,
			STT_HIPROC = 0xF
		};
	}

#pragma pack(push,1)
	struct sDynSymbol32Bit
	{
		le_uint32_t name;
		le_uint32_t value;
		le_uint32_t size;
		le_uint32_t info;
		le_uint32_t other;
		le_uint32_t special_section_index;
	};

	struct sDynSymbol64Bit
	{
		le_uint32_t name;
		byte_t info;
		byte_t other;
		le_uint16_t special_section_index;
		le_uint64_t value;
		le_uint64_t size;
	};
#pragma pack(pop)
}