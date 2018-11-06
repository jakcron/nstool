#include <iostream>
#include <iomanip>
#include <fnd/io.h>
#include <fnd/SimpleFile.h>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>
#include "IniProcess.h"
#include "KipProcess.h"


IniProcess::IniProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false),
	mDoExtractKip(false),
	mKipExtractPath()
{
}

void IniProcess::process()
{
	importHeader();
	importKipList();
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
	{
		displayHeader();
		displayKipList();
	}
	if (mDoExtractKip)
	{
		extractKipList();
	}
}

void IniProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void IniProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void IniProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void IniProcess::setKipExtractPath(const std::string& path)
{
	mDoExtractKip = true;
	mKipExtractPath = path;
}

void IniProcess::importHeader()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	if ((*mFile)->size() < sizeof(nn::hac::sIniHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt INI: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sIniHeader));
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());
}

void IniProcess::importKipList()
{
	// kip pos info
	size_t kip_pos = sizeof(nn::hac::sIniHeader);
	size_t kip_size = 0;

	// tmp data to determine size
	fnd::Vec<byte_t> hdr_raw;
	nn::hac::KernelInitialProcessHeader hdr;

	hdr_raw.alloc(sizeof(nn::hac::sKipHeader));
	for (size_t i = 0; i < mHdr.getKipNum(); i++)
	{
		(*mFile)->read(hdr_raw.data(), kip_pos, hdr_raw.size());
		hdr.fromBytes(hdr_raw.data(), hdr_raw.size());		
		kip_size = getKipSizeFromHeader(hdr);
		mKipList.addElement(new fnd::OffsetAdjustedIFile(mFile, kip_pos, kip_size));
		kip_pos += kip_size;
	}
}

void IniProcess::displayHeader()
{
	std::cout << "[INI Header]" << std::endl;
	std::cout << "  Size:         0x" << std::hex << mHdr.getSize() << std::endl;
	std::cout << "  KIP Num:      " << std::dec << (uint32_t)mHdr.getKipNum() << std::endl;
}

void IniProcess::displayKipList()
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

void IniProcess::extractKipList()
{
	fnd::Vec<byte_t> cache;
	nn::hac::KernelInitialProcessHeader hdr;
	

	// allocate cache memory
	cache.alloc(kCacheSize);

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
		if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
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

size_t IniProcess::getKipSizeFromHeader(const nn::hac::KernelInitialProcessHeader& hdr) const
{
	return sizeof(nn::hac::sKipHeader) + hdr.getTextSegmentInfo().file_layout.size + hdr.getRoSegmentInfo().file_layout.size + hdr.getDataSegmentInfo().file_layout.size;
}