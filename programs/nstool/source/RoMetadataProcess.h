#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/Vec.h>

#include <nn/hac/npdm.h>

#include "nstool.h"
#include "SdkApiString.h"
#include "ElfSymbolParser.h"

class RoMetadataProcess
{
public:
	RoMetadataProcess();

	void process();

	void setRoBinary(const fnd::Vec<byte_t>& bin);
	void setApiInfo(size_t offset, size_t size);
	void setDynSym(size_t offset, size_t size);
	void setDynStr(size_t offset, size_t size);

	void setCliOutputMode(CliOutputMode type);

	void setInstructionType(nn::hac::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	const std::vector<SdkApiString>& getSdkVerApiList() const;
	const std::vector<SdkApiString>& getPublicApiList() const;
	const std::vector<SdkApiString>& getDebugApiList() const;
	const std::vector<SdkApiString>& getPrivateApiList() const;
	const fnd::List<ElfSymbolParser::sElfSymbol>& getSymbolList() const;
private:
	const std::string kModuleName = "RoMetadataProcess";

	CliOutputMode mCliOutputMode;
	nn::hac::npdm::InstructionType mInstructionType;
	bool mListApi;
	bool mListSymbols;	

	struct sLayout 
	{ 
		sLayout() : offset(0), size(0) {}
		size_t offset;
		size_t size;
	};

	sLayout mApiInfo;
	sLayout mDynSym;
	sLayout mDynStr;
	fnd::Vec<byte_t> mRoBlob;
	std::vector<SdkApiString> mSdkVerApiList;
	std::vector<SdkApiString> mPublicApiList;
	std::vector<SdkApiString> mDebugApiList;
	std::vector<SdkApiString> mPrivateApiList;
	
	ElfSymbolParser mSymbolList;

	void importApiList();
	void displayRoMetaData();

	const char* getSectionIndexStr(nn::hac::elf::SpecialSectionIndex shn_index) const;
	const char* getSymbolTypeStr(nn::hac::elf::SymbolType symbol_type) const;
	const char* getSymbolBindingStr(nn::hac::elf::SymbolBinding symbol_binding) const;
};