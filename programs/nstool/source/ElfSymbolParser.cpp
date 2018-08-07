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
	//printf("ElfSymbolParser::parseData()");
	size_t dynSymSize = is64Bit ? sizeof(nn::hac::sElfSymbol64Bit) : sizeof(nn::hac::sElfSymbol32Bit);

	sElfSymbol symbol;
	for (size_t i = 0; i < dyn_sym_size; i += dynSymSize)
	{
		//printf("pos %x\n", i);

		uint32_t name_pos;

		if (is64Bit)
		{
			name_pos = ((nn::hac::sElfSymbol64Bit*)(dyn_sym + i))->name.get();
			symbol.shn_index = (nn::hac::elf::SpecialSectionIndex)((nn::hac::sElfSymbol64Bit*)(dyn_sym + i))->special_section_index.get();
			symbol.symbol_type = (nn::hac::elf::SymbolType)((((nn::hac::sElfSymbol64Bit*)(dyn_sym + i))->info) & nn::hac::elf::STT_HIPROC);
			symbol.symbol_binding = (nn::hac::elf::SymbolBinding)(((((nn::hac::sElfSymbol64Bit*)(dyn_sym + i))->info) >> 4) & nn::hac::elf::STB_HIPROC);
		}
		else
		{
			name_pos = ((nn::hac::sElfSymbol64Bit*)(dyn_sym + i))->name.get();
			symbol.shn_index = (nn::hac::elf::SpecialSectionIndex)((nn::hac::sElfSymbol32Bit*)(dyn_sym + i))->special_section_index.get();
			symbol.symbol_type = (nn::hac::elf::SymbolType)((((nn::hac::sElfSymbol32Bit*)(dyn_sym + i))->info.get()) & nn::hac::elf::STT_HIPROC);
			symbol.symbol_binding = (nn::hac::elf::SymbolBinding)(((((nn::hac::sElfSymbol32Bit*)(dyn_sym + i))->info.get()) >> 4) & nn::hac::elf::STB_HIPROC);
		}

		for (; dyn_str[name_pos] == 0x00 && name_pos < dyn_str_size; name_pos++);
		
		//printf("name_pos = 0x%x\n", name_pos);
		symbol.name = std::string((char*)&dyn_str[name_pos]);
		mSymbolList.addElement(symbol);
	}
	//printf("ElfSymbolParser::parseData() end\n");
}

const fnd::List<ElfSymbolParser::sElfSymbol>& ElfSymbolParser::getSymbolList() const
{
	return mSymbolList;
}