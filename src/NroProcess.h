#pragma once
#include "types.h"
#include "RoMetadataProcess.h"
#include "AssetProcess.h"

#include <nn/hac/define/meta.h>
#include <nn/hac/NroHeader.h>

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
	void setAssetListFs(bool list);
	void setAssetIconExtractPath(const tc::io::Path& path);
	void setAssetNacpExtractPath(const tc::io::Path& path);
	void setAssetRomfsExtractPath(const tc::io::Path& path);

	const RoMetadataProcess& getRoMetadataProcess() const;
private:
	const std::string kModuleName = "NroProcess";

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::NroHeader mHdr;
	tc::ByteData mTextBlob, mRoBlob, mDataBlob;
	RoMetadataProcess mRoMeta;
	bool mIsHomebrewNro;
	AssetProcess mAssetProc;

	void importHeader();
	void importCodeSegments();
	void displayHeader();
	void processRoMeta();
};

}