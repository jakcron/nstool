#pragma once
#include "types.h"
#include "SdkApiString.h"
#include "ElfSymbolParser.h"

#include <pietendo/hac/define/meta.h>

namespace nstool {

class RoMetadataProcess
{
public:
	RoMetadataProcess();

	void process();

	void setRoBinary(const tc::ByteData& bin);
	void setApiInfo(size_t offset, size_t size);
	void setDynSym(size_t offset, size_t size);
	void setDynStr(size_t offset, size_t size);

	void setCliOutputMode(CliOutputMode type);

	void setIs64BitInstruction(bool flag);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	const std::vector<nstool::SdkApiString>& getSdkVerApiList() const;
	const std::vector<nstool::SdkApiString>& getPublicApiList() const;
	const std::vector<nstool::SdkApiString>& getDebugApiList() const;
	const std::vector<nstool::SdkApiString>& getPrivateApiList() const;
	const std::vector<nstool::SdkApiString>& getGuidelineApiList() const;
	const std::vector<nstool::ElfSymbolParser::sElfSymbol>& getSymbolList() const;
private:
	std::string mModuleName;

	CliOutputMode mCliOutputMode;
	bool mIs64BitInstruction;
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
	tc::ByteData mRoBlob;
	std::vector<SdkApiString> mSdkVerApiList;
	std::vector<SdkApiString> mPublicApiList;
	std::vector<SdkApiString> mDebugApiList;
	std::vector<SdkApiString> mPrivateApiList;
	std::vector<SdkApiString> mGuidelineApiList;
	
	ElfSymbolParser mSymbolList;

	void importApiList();
	void displayRoMetaData();

	std::string getSectionIndexStr(uint16_t shn_index) const;
	std::string getSymbolTypeStr(byte_t symbol_type) const;
	std::string getSymbolBindingStr(byte_t symbol_binding) const;
};

}