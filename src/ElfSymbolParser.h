#pragma once
#include "types.h"
#include "elf.h"

namespace nstool {

class ElfSymbolParser
{
public:
	struct sElfSymbol
	{
		uint16_t shn_index;
		byte_t symbol_type;
		byte_t symbol_binding;
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

	const std::vector<sElfSymbol>& getSymbolList() const;
private:
	std::string mModuleName;

	// data
	std::vector<sElfSymbol> mSymbolList;
};

}