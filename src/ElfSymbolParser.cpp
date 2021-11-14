#include "ElfSymbolParser.h"

nstool::ElfSymbolParser::ElfSymbolParser() :
	mModuleName("nstool::ElfSymbolParser"),
	mSymbolList()
{
}

void nstool::ElfSymbolParser::operator=(const ElfSymbolParser& other)
{
	mSymbolList = other.mSymbolList;
}

bool nstool::ElfSymbolParser::operator==(const ElfSymbolParser& other) const
{
	return mSymbolList == other.mSymbolList;
}

bool nstool::ElfSymbolParser::operator!=(const ElfSymbolParser& other) const
{
	return !(*this == other);
}

void nstool::ElfSymbolParser::parseData(const byte_t *dyn_sym, size_t dyn_sym_size, const byte_t *dyn_str, size_t dyn_str_size, bool is64Bit)
{
	size_t dynSymSize = is64Bit ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym);

	sElfSymbol symbol;
	for (size_t i = 0; i < dyn_sym_size; i += dynSymSize)
	{
		uint32_t name_pos;

		if (is64Bit)
		{
			name_pos = tc::bn::detail::__le_uint32(((Elf64_Sym*)(dyn_sym + i))->st_name);
			symbol.shn_index = tc::bn::detail::__le_uint16(((Elf64_Sym*)(dyn_sym + i))->st_shndx);
			symbol.symbol_type = elf::get_elf_st_type(((Elf64_Sym*)(dyn_sym + i))->st_info);
			symbol.symbol_binding = elf::get_elf_st_bind(((Elf64_Sym*)(dyn_sym + i))->st_info);
		}
		else
		{
			name_pos = tc::bn::detail::__le_uint32(((Elf32_Sym*)(dyn_sym + i))->st_name);
			symbol.shn_index = tc::bn::detail::__le_uint16(((Elf32_Sym*)(dyn_sym + i))->st_shndx);
			symbol.symbol_type = elf::get_elf_st_type(((Elf32_Sym*)(dyn_sym + i))->st_info);
			symbol.symbol_binding = elf::get_elf_st_bind(((Elf32_Sym*)(dyn_sym + i))->st_info);
		}

		if (name_pos >= dyn_str_size)
		{
			throw tc::Exception(mModuleName, "Out of bounds symbol name offset");
		}

		//for (; dyn_str[name_pos] == 0x00 && name_pos < dyn_str_size; name_pos++);
		
		symbol.name = std::string((char*)&dyn_str[name_pos]);
		mSymbolList.push_back(symbol);
	}
}

const std::vector<nstool::ElfSymbolParser::sElfSymbol>& nstool::ElfSymbolParser::getSymbolList() const
{
	return mSymbolList;
}