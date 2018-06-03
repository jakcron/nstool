#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/npdm.h>
#include <nx/NsoHeader.h>

#include "nstool.h"
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

	void setArchType(nx::npdm::InstructionType type);

	// processed data
	const nx::NsoHeader& getNsoHeader() const;
	const fnd::MemoryBlob& getTextBlob() const;
	const fnd::MemoryBlob& getRoBlob() const;
	const fnd::MemoryBlob& getDataBlob() const;
	const std::vector<std::string>& getApiList() const;

private:
	const std::string kModuleName = "NsoProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;

	CliOutputType mCliOutputType;
	bool mVerify;
	sOptional<nx::npdm::InstructionType> mArchType;

	nx::NsoHeader mHdr;
	fnd::MemoryBlob mTextBlob, mRoBlob, mDataBlob;
	std::vector<std::string> mApiList;
	DynamicSymbolParser mDynSymbolList;

	void importHeader();
	void importCodeSegments();
	void importApiList();
	void displayHeader();
	void displayRoApiList();
};