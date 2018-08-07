#pragma once
#include <fnd/types.h>

namespace nn
{
namespace hac
{
	namespace elf
	{
		enum SpecialSectionIndex
		{
			SHN_UNDEF,
			SHN_LORESERVE = 0xFF00,
			SHN_LOPROC = 0xFF00,
			SHN_HIPROC = 0xFF1F,
			SHN_LOOS,
			SHN_HIOS = 0xFF3F,
			SHN_ABS = 0xFFF1,
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

		enum SymbolBinding
		{
			STB_LOCAL,
			STB_GLOBAL,
			STB_WEAK,
			STB_LOOS = 10,
			STB_HIOS = 12,
			STB_LOPROC,
			STB_HIPROC = 0xF
		};
	}

#pragma pack(push,1)
	struct sElfSymbol32Bit
	{
		le_uint32_t name;
		le_uint32_t value;
		le_uint32_t size;
		le_uint32_t info;
		le_uint32_t other;
		le_uint32_t special_section_index;
	};

	struct sElfSymbol64Bit
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
}