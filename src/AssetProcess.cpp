#include "AssetProcess.h"

#include "util.h"

nstool::AssetProcess::AssetProcess() :
	mModuleName("nstool::AssetProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}    

void nstool::AssetProcess::process()
{
	importHeader();
	if (mCliOutputMode.show_basic_info)
		displayHeader();
	processSections();
}     

void nstool::AssetProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::AssetProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::AssetProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::AssetProcess::setIconExtractPath(const tc::io::Path& path)
{
	mIconExtractPath = path;
}

void nstool::AssetProcess::setNacpExtractPath(const tc::io::Path& path)
{
	mNacpExtractPath = path;
}

void nstool::AssetProcess::setRomfsShowFsTree(bool show_fs_tree)
{
	mRomfs.setShowFsTree(show_fs_tree);
}

void nstool::AssetProcess::setRomfsExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs)
{
	mRomfs.setExtractJobs(extract_jobs);
}

void nstool::AssetProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(pie::hac::sAssetHeader)))
	{
		throw tc::Exception(mModuleName, "Corrupt ASET: file too small");
	}

	tc::ByteData scratch = tc::ByteData(sizeof(pie::hac::sAssetHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());
}

void nstool::AssetProcess::processSections()
{
	int64_t file_size = mFile->length();

	if (mHdr.getIconInfo().size > 0 && mIconExtractPath.isSet())
	{
		if ((mHdr.getIconInfo().size + mHdr.getIconInfo().offset) > file_size) 
			throw tc::Exception(mModuleName, "ASET geometry for icon beyond file size");

		fmt::print("Saving {:s}...", mIconExtractPath.get().to_string());
		writeSubStreamToFile(mFile, mHdr.getIconInfo().offset, mHdr.getIconInfo().size, mIconExtractPath.get());
	}

	if (mHdr.getNacpInfo().size > 0)
	{
		if ((mHdr.getNacpInfo().size + mHdr.getNacpInfo().offset) > file_size) 
			throw tc::Exception(mModuleName, "ASET geometry for nacp beyond file size");

		if (mNacpExtractPath.isSet())
		{
			fmt::print("Saving {:s}...", mNacpExtractPath.get().to_string());
			writeSubStreamToFile(mFile, mHdr.getNacpInfo().offset, mHdr.getNacpInfo().size, mNacpExtractPath.get());
		}
		
		mNacp.setInputFile(std::make_shared<tc::io::SubStream>(mFile, mHdr.getNacpInfo().offset, mHdr.getNacpInfo().size));
		mNacp.setCliOutputMode(mCliOutputMode);
		mNacp.setVerifyMode(mVerify);

		mNacp.process();
	}

	if (mHdr.getRomfsInfo().size > 0)
	{
		if ((mHdr.getRomfsInfo().size + mHdr.getRomfsInfo().offset) > file_size) 
			throw tc::Exception(mModuleName, "ASET geometry for romfs beyond file size");

		mRomfs.setInputFile(std::make_shared<tc::io::SubStream>(mFile, mHdr.getRomfsInfo().offset, mHdr.getRomfsInfo().size));
		mRomfs.setCliOutputMode(mCliOutputMode);
		mRomfs.setVerifyMode(mVerify);

		mRomfs.process();
	}
}

void nstool::AssetProcess::displayHeader()
{
	if (mCliOutputMode.show_layout)
	{
		fmt::print("[ASET Header]\n");
		fmt::print("  Icon:\n");
		fmt::print("    Offset:       0x{:x}\n", mHdr.getIconInfo().offset);
		fmt::print("    Size:         0x{:x}\n", mHdr.getIconInfo().size);
		fmt::print("  NACP:\n");
		fmt::print("    Offset:       0x{:x}\n", mHdr.getNacpInfo().offset);
		fmt::print("    Size:         0x{:x}\n", mHdr.getNacpInfo().size);
		fmt::print("  RomFs:\n");
		fmt::print("    Offset:       0x{:x}\n", mHdr.getRomfsInfo().offset);
		fmt::print("    Size:         0x{:x}\n", mHdr.getRomfsInfo().size);
	}	
}
		
