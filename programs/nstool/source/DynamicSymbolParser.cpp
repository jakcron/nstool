#include "DynamicSymbolParser.h"

DynamicSymbolParser::DynamicSymbolParser()
{
	mDynSymbolList.clear();
}

bool DynamicSymbolParser::operator==(const DynamicSymbolParser& other) const
{
	return isEqual(other);
}

bool DynamicSymbolParser::operator!=(const DynamicSymbolParser& other) const
{
	return !isEqual(other);
}

void DynamicSymbolParser::operator=(const DynamicSymbolParser& other)
{
	copyFrom(other);
}

void DynamicSymbolParser::parseData(const byte_t *dyn_sym, size_t dyn_sym_size, const byte_t *dyn_str, size_t dyn_str_size, bool is64Bit)
{
	//printf("DynamicSymbolParser::parseData()");
	size_t dynSymSize = is64Bit ? sizeof(nx::sDynSymbol64Bit) : sizeof(nx::sDynSymbol32Bit);

	sDynSymbol symbol;
	for (size_t i = 0; i < dyn_sym_size; i += dynSymSize)
	{
		//printf("pos %x\n", i);

		uint32_t name_pos;

		if (is64Bit)
		{
			name_pos = ((nx::sDynSymbol64Bit*)(dyn_sym + i))->name.get();
			symbol.shn_index = (nx::dynsym::SpecialSectionIndex)((nx::sDynSymbol64Bit*)(dyn_sym + i))->special_section_index.get();
			symbol.symbol_type = (nx::dynsym::SymbolType)((((nx::sDynSymbol64Bit*)(dyn_sym + i))->info) & nx::dynsym::STT_HIPROC);
		}
		else
		{
			name_pos = ((nx::sDynSymbol64Bit*)(dyn_sym + i))->name.get();
			symbol.shn_index = (nx::dynsym::SpecialSectionIndex)((nx::sDynSymbol32Bit*)(dyn_sym + i))->special_section_index.get();
			symbol.symbol_type = (nx::dynsym::SymbolType)((((nx::sDynSymbol32Bit*)(dyn_sym + i))->info.get()) & nx::dynsym::STT_HIPROC);
		}

		for (; dyn_str[name_pos] == 0x00 && name_pos < dyn_str_size; name_pos++);
		
		//printf("name_pos = 0x%x\n", name_pos);
		symbol.name = std::string((char*)&dyn_str[name_pos]);
		mDynSymbolList.addElement(symbol);
	}
	//printf("DynamicSymbolParser::parseData() end\n");
}

const fnd::List<DynamicSymbolParser::sDynSymbol>& DynamicSymbolParser::getDynamicSymbolList() const
{
	return mDynSymbolList;
}

bool DynamicSymbolParser::isEqual(const DynamicSymbolParser& other) const
{
	return mDynSymbolList == other.mDynSymbolList;
}

void DynamicSymbolParser::copyFrom(const DynamicSymbolParser& other)
{
	mDynSymbolList = other.mDynSymbolList;
}
