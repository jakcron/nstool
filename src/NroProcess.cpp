#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>
#include <fnd/lz4.h>
#include <nn/hac/define/nro-hb.h>
#include "NroProcess.h"

nstool::NroProcess::NroProcess():
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::NroProcess::process()
{
	importHeader();
	importCodeSegments();

	if (mCliOutputMode.show_basic_info)
		displayHeader();

	processRoMeta();

	if (mIsHomebrewNro)
		mAssetProc.process();
}

void nstool::NroProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::NroProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::NroProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::NroProcess::setIs64BitInstruction(bool flag)
{
	mRoMeta.setIs64BitInstruction(flag);
}

void nstool::NroProcess::setListApi(bool listApi)
{
	mRoMeta.setListApi(listApi);
}

void nstool::NroProcess::setListSymbols(bool listSymbols)
{
	mRoMeta.setListSymbols(listSymbols);
}

void nstool::NroProcess::setAssetListFs(bool list)
{
	mAssetProc.setListFs(list);
}

void nstool::NroProcess::setAssetIconExtractPath(const std::string& path)
{
	mAssetProc.setIconExtractPath(path);
}

void nstool::NroProcess::setAssetNacpExtractPath(const std::string& path)
{
	mAssetProc.setNacpExtractPath(path);
}

void nstool::NroProcess::setAssetRomfsExtractPath(const std::string& path)
{
	mAssetProc.setRomfsExtractPath(path);
}

const RoMetadataProcess& nstool::NroProcess::getRoMetadataProcess() const
{
	return mRoMeta;
}

void nstool::NroProcess::importHeader()
{
	tc::ByteData scratch;

	if (*mFile == nullptr)
	{
		throw tc::Exception(kModuleName, "No file reader set.");
	}

	if ((*mFile)->size() < sizeof(nn::hac::sNroHeader))
	{
		throw tc::Exception(kModuleName, "Corrupt NRO: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sNroHeader));
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());

	// setup homebrew extension
	nn::hac::sNroHeader* raw_hdr = (nn::hac::sNroHeader*)scratch.data();
	if (((tc::bn::le64<uint64_t>*)raw_hdr->reserved_0)->get() == nn::hac::nro::kNroHomebrewStructMagic && (*mFile)->size() > mHdr.getNroSize())
	{
		mIsHomebrewNro = true;
		mAssetProc.setInputFile(new fnd::OffsetAdjustedIFile(mFile, mHdr.getNroSize(), (*mFile)->size() - mHdr.getNroSize()));
		mAssetProc.setCliOutputMode(mCliOutputMode);
		mAssetProc.setVerifyMode(mVerify);
	}
	else
		mIsHomebrewNro = false;
}

void nstool::NroProcess::importCodeSegments()
{
	mTextBlob.alloc(mHdr.getTextInfo().size);
	(*mFile)->read(mTextBlob.data(), mHdr.getTextInfo().memory_offset, mTextBlob.size());
	mRoBlob.alloc(mHdr.getRoInfo().size);
	(*mFile)->read(mRoBlob.data(), mHdr.getRoInfo().memory_offset, mRoBlob.size());
	mDataBlob.alloc(mHdr.getDataInfo().size);
	(*mFile)->read(mDataBlob.data(), mHdr.getDataInfo().memory_offset, mDataBlob.size());
}

void nstool::NroProcess::displayHeader()
{
	std::cout << "[NRO Header]" << std::endl;
	std::cout << "  RoCrt:       " << std::endl;
	std::cout << "    EntryPoint: 0x" << std::hex << mHdr.getRoCrtEntryPoint() << std::endl;
	std::cout << "    ModOffset:  0x" << std::hex << mHdr.getRoCrtModOffset() << std::endl;
	std::cout << "  ModuleId:    " << fnd::SimpleTextOutput::arrayToString(mHdr.getModuleId().data, nn::hac::nro::kModuleIdSize, false, "") << std::endl;
	std::cout << "  NroSize:     0x" << std::hex << mHdr.getNroSize() << std::endl;
	std::cout << "  Program Sections:" << std::endl;
	std::cout << "     .text:" << std::endl;
	std::cout << "      Offset:     0x" << std::hex << mHdr.getTextInfo().memory_offset << std::endl;
	std::cout << "      Size:       0x" << std::hex << mHdr.getTextInfo().size << std::endl;
	std::cout << "    .ro:" << std::endl;
	std::cout << "      Offset:     0x" << std::hex << mHdr.getRoInfo().memory_offset << std::endl;
	std::cout << "      Size:       0x" << std::hex << mHdr.getRoInfo().size << std::endl;
	if (mCliOutputMode.show_extended_info)
	{
		std::cout << "    .api_info:" << std::endl;
		std::cout << "      Offset:     0x" << std::hex <<  mHdr.getRoEmbeddedInfo().memory_offset << std::endl;
		std::cout << "      Size:       0x" << std::hex << mHdr.getRoEmbeddedInfo().size << std::endl;
		std::cout << "    .dynstr:" << std::endl;
		std::cout << "      Offset:     0x" << std::hex << mHdr.getRoDynStrInfo().memory_offset << std::endl;
		std::cout << "      Size:       0x" << std::hex << mHdr.getRoDynStrInfo().size << std::endl;
		std::cout << "    .dynsym:" << std::endl;
		std::cout << "      Offset:     0x" << std::hex << mHdr.getRoDynSymInfo().memory_offset << std::endl;
		std::cout << "      Size:       0x" << std::hex << mHdr.getRoDynSymInfo().size << std::endl;
	}
	std::cout << "    .data:" << std::endl;
	std::cout << "      Offset:     0x" << std::hex << mHdr.getDataInfo().memory_offset << std::endl;
	std::cout << "      Size:       0x" << std::hex << mHdr.getDataInfo().size << std::endl;
	std::cout << "    .bss:" << std::endl;
	std::cout << "      Size:       0x" << std::hex << mHdr.getBssSize() << std::endl;
}

void nstool::NroProcess::processRoMeta()
{
	if (mRoBlob.size())
	{
		// setup ro metadata
		mRoMeta.setApiInfo(mHdr.getRoEmbeddedInfo().memory_offset, mHdr.getRoEmbeddedInfo().size);
		mRoMeta.setDynSym(mHdr.getRoDynSymInfo().memory_offset, mHdr.getRoDynSymInfo().size);
		mRoMeta.setDynStr(mHdr.getRoDynStrInfo().memory_offset, mHdr.getRoDynStrInfo().size);
		mRoMeta.setRoBinary(mRoBlob);
		mRoMeta.setCliOutputMode(mCliOutputMode);
		mRoMeta.process();
	}
}