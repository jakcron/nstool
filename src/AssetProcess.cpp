#include "AssetProcess.h"

#include "utils.h"

nstool::AssetProcess::AssetProcess() :
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

void nstool::AssetProcess::setListFs(bool list)
{
	mRomfs.setListFs(list);
}

void nstool::AssetProcess::setIconExtractPath(const std::string& path)
{
	mIconExtractPath = path;
}

void nstool::AssetProcess::setNacpExtractPath(const std::string& path)
{
	mNacpExtractPath = path;
}

void nstool::AssetProcess::setRomfsExtractPath(const std::string& path)
{
	mRomfs.setExtractPath(path);
}


void nstool::AssetProcess::importHeader()
{
	tc::ByteData scratch;

	if (mFile == nullptr)
	{
		throw tc::Exception(kModuleName, "No file reader set.");
	}

	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());

	if (file_size < sizeof(nn::hac::sAssetHeader))
	{
		throw tc::Exception(kModuleName, "Corrupt ASET: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sAssetHeader));
	mFile->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());
}

void nstool::AssetProcess::processSections()
{
	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());

	if (mHdr.getIconInfo().size > 0 && mIconExtractPath.isSet())
	{
		if ((mHdr.getIconInfo().size + mHdr.getIconInfo().offset) > file_size) 
			throw tc::Exception(kModuleName, "ASET geometry for icon beyond file size");

		writeSubStreamToFile(mFile, tc::io::IOUtil::castSizeToInt64(mHdr.getIconInfo().offset), tc::io::IOUtil::castSizeToInt64(mHdr.getIconInfo().size), mIconExtractPath.get());
	}

	if (mHdr.getNacpInfo().size > 0)
	{
		if ((mHdr.getNacpInfo().size + mHdr.getNacpInfo().offset) > file_size) 
			throw tc::Exception(kModuleName, "ASET geometry for nacp beyond file size");

		if (mNacpExtractPath.isSet())
		{
			writeSubStreamToFile(mFile, tc::io::IOUtil::castSizeToInt64(mHdr.getNacpInfo().offset), tc::io::IOUtil::castSizeToInt64(mHdr.getNacpInfo().size), mNacpExtractPath.get());
		}
		
		mNacp.setInputFile(std::make_shared<tc::io::SubStream>(mFile, tc::io::IOUtil::castSizeToInt64(mHdr.getNacpInfo().offset), tc::io::IOUtil::castSizeToInt64(mHdr.getNacpInfo().size)));
		mNacp.setCliOutputMode(mCliOutputMode);
		mNacp.setVerifyMode(mVerify);

		mNacp.process();
	}

	if (mHdr.getRomfsInfo().size > 0)
	{
		if ((mHdr.getRomfsInfo().size + mHdr.getRomfsInfo().offset) > file_size) 
			throw tc::Exception(kModuleName, "ASET geometry for romfs beyond file size");

		mRomfs.setInputFile(std::make_shared<tc::io::SubStream>(mFile, tc::io::IOUtil::castSizeToInt64(mHdr.getRomfsInfo().offset), tc::io::IOUtil::castSizeToInt64(mHdr.getRomfsInfo().size)));
		mRomfs.setCliOutputMode(mCliOutputMode);
		mRomfs.setVerifyMode(mVerify);

		mRomfs.process();
	}
}

void nstool::AssetProcess::displayHeader()
{
	if (mCliOutputMode.show_layout)
	{
		std::cout << "[ASET Header]" << std::endl;
		std::cout << "  Icon:" << std::endl;
		std::cout << "    Offset:       0x" << std::hex << mHdr.getIconInfo().offset << std::endl;
		std::cout << "    Size:         0x" << std::hex << mHdr.getIconInfo().size << std::endl;
		std::cout << "  NACP:" << std::endl;
		std::cout << "    Offset:       0x" << std::hex << mHdr.getNacpInfo().offset << std::endl;
		std::cout << "    Size:         0x" << std::hex << mHdr.getNacpInfo().size << std::endl;
		std::cout << "  RomFS:" << std::endl;
		std::cout << "    Offset:       0x" << std::hex << mHdr.getRomfsInfo().offset << std::endl;
		std::cout << "    Size:         0x" << std::hex << mHdr.getRomfsInfo().size << std::endl;
	}	
}
		
