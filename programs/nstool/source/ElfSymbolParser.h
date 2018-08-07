#pragma once
#include <string>
#include <fnd/List.h>
#include <nn/hac/elf.h>

class ElfSymbolParser
{
public:
	struct sElfSymbol
	{
		nn::hac::elf::SpecialSectionIndex shn_index;
		nn::hac::elf::SymbolType symbol_type;
		nn::hac::elf::SymbolBinding symbol_binding;
		std::string name;

		void operator=(const sElfSymbol& other)
		{
			shn_index = other.shn_index;
			symbol_type = other.symbol_type;
			symbol_binding = other.symbol_binding;
			name = other.name;
		}

		bool operator==(const sElfSymbol& other) const
		{
			return (shn_index == other.shn_index && symbol_type == other.symbol_type && symbol_binding == other.symbol_binding && name == other.name);
		}

		bool operator!=(const sElfSymbol& other) const
		{
			return !(*this == other);
		}
	};

	ElfSymbolParser();

	void operator=(const ElfSymbolParser& other);
	bool operator==(const ElfSymbolParser& other) const;
	bool operator!=(const ElfSymbolParser& other) const;

	void parseData(const byte_t *dyn_sym, size_t dyn_sym_size, const byte_t *dyn_str, size_t dyn_str_size, bool is64Bit);

	const fnd::List<sElfSymbol>& getSymbolList() const;
private:

	// data
	fnd::List<sElfSymbol> mSymbolList;
};