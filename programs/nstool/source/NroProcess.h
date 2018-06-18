#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/npdm.h>
#include <nx/NroHeader.h>
#include "AssetProcess.h"

#include "nstool.h"
#include "SdkApiString.h"
#include "DynamicSymbolParser.h"

class NroProcess
{
public:
	NroProcess();
	~NroProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setInstructionType(nx::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	// for homebrew NROs with Asset blobs appended
	void setAssetListFs(bool list);
	void setAssetIconExtractPath(const std::string& path);
	void setAssetNacpExtractPath(const std::string& path);
	void setAssetRomfsExtractPath(const std::string& path);
private:
	const std::string kModuleName = "NroProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;

	CliOutputMode mCliOutputMode;
	bool mVerify;
	nx::npdm::InstructionType mInstructionType;
	bool mListApi;
	bool mListSymbols;	

	nx::NroHeader mHdr;
	bool mIsHomebrewNro;
	AssetProcess mAssetProc;
	fnd::MemoryBlob mTextBlob, mRoBlob, mDataBlob;
	std::vector<SdkApiString> mApiList;
	DynamicSymbolParser mDynSymbolList;

	void importHeader();
	void importCodeSegments();
	void importApiList();
	void displayHeader();
	void displayRoMetaData();

	const char* getApiTypeStr(SdkApiString::ApiType type) const;
	const char* getSectionIndexStr(nx::dynsym::SpecialSectionIndex shn_index) const;
	const char* getSymbolTypeStr(nx::dynsym::SymbolType symbol_type) const;
};