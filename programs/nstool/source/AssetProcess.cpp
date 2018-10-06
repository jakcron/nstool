#include <iostream>
#include <iomanip>
#include <fnd/SimpleFile.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>
#include "AssetProcess.h"


AssetProcess::AssetProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void AssetProcess::process()
{
	importHeader();
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayHeader();
	processSections();
}

void AssetProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void AssetProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void AssetProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void AssetProcess::setListFs(bool list)
{
	mRomfs.setListFs(list);
}

void AssetProcess::setIconExtractPath(const std::string& path)
{
	mIconExtractPath = path;
}

void AssetProcess::setNacpExtractPath(const std::string& path)
{
	mNacpExtractPath = path;
}

void AssetProcess::setRomfsExtractPath(const std::string& path)
{
	mRomfs.setExtractPath(path);
}


void AssetProcess::importHeader()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	if ((*mFile)->size() < sizeof(nn::hac::sAssetHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt ASET: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sAssetHeader));
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());
}

void AssetProcess::processSections()
{
	if (mHdr.getIconInfo().size > 0 && mIconExtractPath.isSet)
	{
		if ((mHdr.getIconInfo().size + mHdr.getIconInfo().offset) > (*mFile)->size()) 
			throw fnd::Exception(kModuleName, "ASET geometry for icon beyond file size");

		fnd::SimpleFile outfile(mIconExtractPath.var, fnd::SimpleFile::Create);
		fnd::Vec<byte_t> cache;

		cache.alloc(mHdr.getIconInfo().size);
		(*mFile)->read(cache.data(), mHdr.getIconInfo().offset, cache.size());
		outfile.write(cache.data(), cache.size());
		outfile.close();
	}

	if (mHdr.getNacpInfo().size > 0)
	{
		if ((mHdr.getNacpInfo().size + mHdr.getNacpInfo().offset) > (*mFile)->size()) 
			throw fnd::Exception(kModuleName, "ASET geometry for nacp beyond file size");

		if (mNacpExtractPath.isSet)
		{
			fnd::SimpleFile outfile(mNacpExtractPath.var, fnd::SimpleFile::Create);
			fnd::Vec<byte_t> cache;

			cache.alloc(mHdr.getNacpInfo().size);
			(*mFile)->read(cache.data(), mHdr.getNacpInfo().offset, cache.size());
			outfile.write(cache.data(), cache.size());
			outfile.close();
		}
		
		mNacp.setInputFile(new fnd::OffsetAdjustedIFile(mFile, mHdr.getNacpInfo().offset, mHdr.getNacpInfo().size));
		mNacp.setCliOutputMode(mCliOutputMode);
		mNacp.setVerifyMode(mVerify);

		mNacp.process();
	}

	if (mHdr.getRomfsInfo().size > 0)
	{
		if ((mHdr.getRomfsInfo().size + mHdr.getRomfsInfo().offset) > (*mFile)->size()) 
			throw fnd::Exception(kModuleName, "ASET geometry for romfs beyond file size");

		mRomfs.setInputFile(new fnd::OffsetAdjustedIFile(mFile, mHdr.getRomfsInfo().offset, mHdr.getRomfsInfo().size));
		mRomfs.setCliOutputMode(mCliOutputMode);
		mRomfs.setVerifyMode(mVerify);

		mRomfs.process();
	}
}

void AssetProcess::displayHeader()
{
	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
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
		
