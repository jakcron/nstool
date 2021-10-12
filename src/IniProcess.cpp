#include <iostream>
#include <iomanip>
#include <fnd/io.h>
#include <fnd/SimpleFile.h>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>
#include "IniProcess.h"
#include "KipProcess.h"


nstool::IniProcess::IniProcess() :
	mModuleName("nstool::IniProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mDoExtractKip(false),
	mKipExtractPath()
{
}

void nstool::IniProcess::process()
{
	importHeader();
	importKipList();
	if (mCliOutputMode.show_basic_info)
	{
		displayHeader();
		displayKipList();
	}
	if (mKipExtractPath.isSet())
	{
		extractKipList();
	}
}

void nstool::IniProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::IniProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::IniProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::IniProcess::setKipExtractPath(const tc::io::Path& path)
{
	mKipExtractPath = path;
}

void nstool::IniProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is smaller than INI header size
	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());
	if (file_size < sizeof(nn::hac::sIniHeader))
	{
		throw tc::Exception(mModuleName, "Corrupt NSO: file too small.");
	}

	// read ini
	tc::ByteData scratch = tc::ByteData(sizeof(nn::hac::sIniHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// parse ini header
	mHdr.fromBytes(scratch.data(), scratch.size());
}

void nstool::IniProcess::importKipList()
{
	// kip pos info
	size_t kip_pos = sizeof(nn::hac::sIniHeader);
	size_t kip_size = 0;

	// tmp data to determine size
	tc::ByteData hdr_raw;
	nn::hac::KernelInitialProcessHeader hdr;

	hdr_raw.alloc(sizeof(nn::hac::sKipHeader));
	for (size_t i = 0; i < mHdr.getKipNum(); i++)
	{
		(*mFile)->read(hdr_raw.data(), kip_pos, hdr_raw.size());
		hdr.fromBytes(hdr_raw.data(), hdr_raw.size());		
		kip_size = getKipSizeFromHeader(hdr);
		mKipList.push_back(new fnd::OffsetAdjustedIFile(mFile, kip_pos, kip_size));
		kip_pos += kip_size;
	}
}

void nstool::IniProcess::displayHeader()
{
	fmt::print("[INI Header]\n");
	fmt::print("  Size:         0x{:x}\n", mHdr.getSize());
	fmt::print("  KIP Num:      {:d}\n", mHdr.getKipNum());
}

void nstool::IniProcess::displayKipList()
{
	for (size_t i = 0; i < mKipList.size(); i++)
	{
		KipProcess obj;

		obj.setInputFile(mKipList[i]);
		obj.setCliOutputMode(mCliOutputMode);
		obj.setVerifyMode(mVerify);

		obj.process();
	}
}

void nstool::IniProcess::extractKipList()
{
	tc::ByteData cache;
	nn::hac::KernelInitialProcessHeader hdr;
	

	// allocate cache memory
	cache = tc::ByteData(kCacheSize);

	// make extract dir
	fnd::io::makeDirectory(mKipExtractPath);

	
	// outfile object for writing KIP
	fnd::SimpleFile out_file;
	std::string out_path;
	size_t out_size;

	for (size_t i = 0; i < mKipList.size(); i++)
	{
		// read header
		(*mKipList[i])->read(cache.data(), 0, cache.size());
		hdr.fromBytes(cache.data(), cache.size());		

		// generate path
		out_path.clear();
		fnd::io::appendToPath(out_path, mKipExtractPath);
		fnd::io::appendToPath(out_path, hdr.getName() + kKipExtention);

		// open file
		out_file.open(out_path, fnd::SimpleFile::Create);

		// get kip file size
		out_size = (*mKipList[i])->size();
		// extract kip
		if (mCliOutputMode.show_basic_info)
			printf("extract=[%s]\n", out_path.c_str());

		(*mKipList[i])->seek(0);
		for (size_t j = 0; j < ((out_size / kCacheSize) + ((out_size % kCacheSize) != 0)); j++)
		{
			(*mKipList[i])->read(cache.data(), _MIN(out_size - (kCacheSize * j), kCacheSize));
			out_file.write(cache.data(), _MIN(out_size - (kCacheSize * j), kCacheSize));
		}			
		out_file.close();
	}
}

size_t nstool::IniProcess::getKipSizeFromHeader(const nn::hac::KernelInitialProcessHeader& hdr) const
{
	return sizeof(nn::hac::sKipHeader) + hdr.getTextSegmentInfo().file_layout.size + hdr.getRoSegmentInfo().file_layout.size + hdr.getDataSegmentInfo().file_layout.size;
}