#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/npdm.h>
#include <nx/NsoHeader.h>

#include "nstool.h"
#include "SdkApiString.h"
#include "DynamicSymbolParser.h"

class NsoProcess
{
public:
	NsoProcess();
	~NsoProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	void setInstructionType(nx::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);
private:
	const std::string kModuleName = "NsoProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;

	CliOutputType mCliOutputType;
	bool mVerify;
	nx::npdm::InstructionType mInstructionType;
	bool mListApi;
	bool mListSymbols;

	nx::NsoHeader mNsoHdr;
	fnd::MemoryBlob mTextBlob, mRoBlob, mDataBlob;
	std::vector<SdkApiString> mApiList;
	DynamicSymbolParser mDynSymbolList;

	void importHeader();
	void importCodeSegments();
	void importApiList();
	void displayNsoHeader();
	void displayRoMetaData();

	const char* getApiTypeStr(SdkApiString::ApiType type) const;
	const char* getSectionIndexStr(nx::dynsym::SpecialSectionIndex shn_index) const;
	const char* getSymbolTypeStr(nx::dynsym::SymbolType symbol_type) const;
};