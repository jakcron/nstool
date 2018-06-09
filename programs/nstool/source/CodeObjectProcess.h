#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/npdm.h>
#include <nx/NsoHeader.h>
#include <nx/NroHeader.h>

#include "nstool.h"
#include "SdkApiString.h"
#include "DynamicSymbolParser.h"

class CodeObjectProcess
{
public:
	enum CodeObjectType
	{
		OBJ_NSO,
		OBJ_NRO,
		OBJ_INVALID
	};

	CodeObjectProcess();
	~CodeObjectProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	void setCodeObjectType(CodeObjectType type);
	void setInstructionType(nx::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	// processed data
	const nx::NsoHeader& getNsoHeader() const;
	const fnd::MemoryBlob& getTextBlob() const;
	const fnd::MemoryBlob& getRoBlob() const;
	const fnd::MemoryBlob& getDataBlob() const;
	const std::vector<SdkApiString>& getApiList() const;

private:
	const std::string kModuleName = "CodeObjectProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;

	CliOutputType mCliOutputType;
	bool mVerify;
	CodeObjectType mObjType;
	nx::npdm::InstructionType mInstructionType;
	bool mListApi;
	bool mListSymbols;

	nx::NsoHeader mNsoHdr;
	nx::NroHeader mNroHdr;
	fnd::MemoryBlob mTextBlob, mRoBlob, mDataBlob;
	std::vector<SdkApiString> mApiList;
	DynamicSymbolParser mDynSymbolList;

	void importHeader();
	void importCodeSegments();
	void importApiList();
	void displayNsoHeader();
	void displayNroHeader();
	void displayRoMetaData();

	const char* getApiTypeStr(SdkApiString::ApiType type) const;
	const char* getSectionIndexStr(nx::dynsym::SpecialSectionIndex shn_index) const;
	const char* getSymbolTypeStr(nx::dynsym::SymbolType symbol_type) const;
};