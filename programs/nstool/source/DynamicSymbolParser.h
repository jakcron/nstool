#pragma once
#include <string>
#include <fnd/List.h>
#include <nx/dynamic_symbol.h>

class DynamicSymbolParser
{
public:
	struct sDynSymbol
	{
		nx::dynsym::SpecialSectionIndex shn_index;
		nx::dynsym::SymbolType symbol_type;
		std::string name;

		void operator=(const sDynSymbol& other)
		{
			shn_index = other.shn_index;
			symbol_type = other.symbol_type;
			name = other.name;
		}

		bool operator==(const sDynSymbol& other) const
		{
			return (shn_index == other.shn_index && symbol_type == other.symbol_type && name == other.name);
		}

		bool operator!=(const sDynSymbol& other) const
		{
			return !(*this == other);
		}
	};

	DynamicSymbolParser();

	bool operator==(const DynamicSymbolParser& other) const;
	bool operator!=(const DynamicSymbolParser& other) const;
	void operator=(const DynamicSymbolParser& other);

	void parseData(const byte_t *dyn_sym, size_t dyn_sym_size, const byte_t *dyn_str, size_t dyn_str_size, bool is64Bit);

	const fnd::List<sDynSymbol>& getDynamicSymbolList() const;
private:

	// data
	fnd::List<sDynSymbol> mDynSymbolList;

	bool isEqual(const DynamicSymbolParser& other) const;
	void copyFrom(const DynamicSymbolParser& other);
};