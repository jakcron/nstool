#include "ElfSymbolParser.h"

ElfSymbolParser::ElfSymbolParser()
{
	mSymbolList.clear();
}

void ElfSymbolParser::operator=(const ElfSymbolParser& other)
{
	mSymbolList = other.mSymbolList;
}

bool ElfSymbolParser::operator==(const ElfSymbolParser& other) const
{
	return mSymbolList == other.mSymbolList;
}

bool ElfSymbolParser::operator!=(const ElfSymbolParser& other) const
{
	return !(*this == other);
}

void ElfSymbolParser::parseData(const byte_t *dyn_sym, size_t dyn_sym_size, const byte_t *dyn_str, size_t dyn_str_size, bool is64Bit)
{
	size_t dynSymSize = is64Bit ? sizeof(fnd::Elf64_Sym) : sizeof(fnd::Elf32_Sym);

	sElfSymbol symbol;
	for (size_t i = 0; i < dyn_sym_size; i += dynSymSize)
	{
		uint32_t name_pos;

		if (is64Bit)
		{
			name_pos = le_word(((fnd::Elf64_Sym*)(dyn_sym + i))->st_name);
			symbol.shn_index = le_hword(((fnd::Elf64_Sym*)(dyn_sym + i))->st_shndx);
			symbol.symbol_type = fnd::elf::get_elf_st_type(((fnd::Elf64_Sym*)(dyn_sym + i))->st_info);
			symbol.symbol_binding = fnd::elf::get_elf_st_bind(((fnd::Elf64_Sym*)(dyn_sym + i))->st_info);
		}
		else
		{
			name_pos = le_word(((fnd::Elf32_Sym*)(dyn_sym + i))->st_name);
			symbol.shn_index = le_hword(((fnd::Elf32_Sym*)(dyn_sym + i))->st_shndx);
			symbol.symbol_type = fnd::elf::get_elf_st_type(((fnd::Elf32_Sym*)(dyn_sym + i))->st_info);
			symbol.symbol_binding = fnd::elf::get_elf_st_bind(((fnd::Elf32_Sym*)(dyn_sym + i))->st_info);
		}

		if (name_pos >= dyn_str_size)
		{
			throw fnd::Exception(kModuleName, "Out of bounds symbol name offset");
		}

		//for (; dyn_str[name_pos] == 0x00 && name_pos < dyn_str_size; name_pos++);
		
		symbol.name = std::string((char*)&dyn_str[name_pos]);
		mSymbolList.addElement(symbol);
	}
}

const fnd::List<ElfSymbolParser::sElfSymbol>& ElfSymbolParser::getSymbolList() const
{
	return mSymbolList;
}