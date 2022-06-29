#pragma once
#include "types.h"
#include "RoMetadataProcess.h"
#include "AssetProcess.h"

#include <pietendo/hac/NroHeader.h>

namespace nstool {

class NroProcess
{
public:
	NroProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	void setIs64BitInstruction(bool flag);
	void setListApi(bool listApi);
	void setListSymbols(bool listSymbols);

	// for homebrew NROs with Asset blobs appended
	void setAssetIconExtractPath(const tc::io::Path& path);
	void setAssetNacpExtractPath(const tc::io::Path& path);
	void setAssetRomfsShowFsTree(bool show_fs_tree);
	void setAssetRomfsExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs);

	const nstool::RoMetadataProcess& getRoMetadataProcess() const;
private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	pie::hac::NroHeader mHdr;
	tc::ByteData mTextBlob, mRoBlob, mDataBlob;
	nstool::RoMetadataProcess mRoMeta;
	bool mIsHomebrewNro;
	nstool::AssetProcess mAssetProc;

	void importHeader();
	void importCodeSegments();
	void displayHeader();
	void processRoMeta();
};

}