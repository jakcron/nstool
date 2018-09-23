#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/npdm.h>
#include <nn/hac/NroHeader.h>
#include "AssetProcess.h"

#include "common.h"
#include "RoMetadataProcess.h"

class NroProcess
{
public:
	NroProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setInstructionType(nn::hac::npdm::InstructionType type);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	// for homebrew NROs with Asset blobs appended
	void setAssetListFs(bool list);
	void setAssetIconExtractPath(const std::string& path);
	void setAssetNacpExtractPath(const std::string& path);
	void setAssetRomfsExtractPath(const std::string& path);

	const RoMetadataProcess& getRoMetadataProcess() const;
private:
	const std::string kModuleName = "NroProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::NroHeader mHdr;
	fnd::Vec<byte_t> mTextBlob, mRoBlob, mDataBlob;
	RoMetadataProcess mRoMeta;
	bool mIsHomebrewNro;
	AssetProcess mAssetProc;

	void importHeader();
	void importCodeSegments();
	void displayHeader();
	void processRoMeta();
};