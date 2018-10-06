#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>
#include <fnd/lz4.h>
#include <nn/hac/nro-hb.h>
#include "NroProcess.h"

NroProcess::NroProcess():
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void NroProcess::process()
{
	importHeader();
	importCodeSegments();

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayHeader();

	processRoMeta();

	if (mIsHomebrewNro)
		mAssetProc.process();
}

void NroProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void NroProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void NroProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void NroProcess::setInstructionType(nn::hac::npdm::InstructionType type)
{
	mRoMeta.setInstructionType(type);
}

void NroProcess::setListApi(bool listApi)
{
	mRoMeta.setListApi(listApi);
}

void NroProcess::setListSymbols(bool listSymbols)
{
	mRoMeta.setListSymbols(listSymbols);
}

void NroProcess::setAssetListFs(bool list)
{
	mAssetProc.setListFs(list);
}

void NroProcess::setAssetIconExtractPath(const std::string& path)
{
	mAssetProc.setIconExtractPath(path);
}

void NroProcess::setAssetNacpExtractPath(const std::string& path)
{
	mAssetProc.setNacpExtractPath(path);
}

void NroProcess::setAssetRomfsExtractPath(const std::string& path)
{
	mAssetProc.setRomfsExtractPath(path);
}

const RoMetadataProcess& NroProcess::getRoMetadataProcess() const
{
	return mRoMeta;
}

void NroProcess::importHeader()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	if ((*mFile)->size() < sizeof(nn::hac::sNroHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt NRO: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sNroHeader));
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());

	// setup homebrew extension
	nn::hac::sNroHeader* raw_hdr = (nn::hac::sNroHeader*)scratch.data();
	if (((le_uint64_t*)raw_hdr->reserved_0)->get() == nn::hac::nro::kNroHomebrewStructMagic && (*mFile)->size() > mHdr.getNroSize())
	{
		mIsHomebrewNro = true;
		mAssetProc.setInputFile(new fnd::OffsetAdjustedIFile(mFile, mHdr.getNroSize(), (*mFile)->size() - mHdr.getNroSize()));
		mAssetProc.setCliOutputMode(mCliOutputMode);
		mAssetProc.setVerifyMode(mVerify);
	}
	else
		mIsHomebrewNro = false;
}

void NroProcess::importCodeSegments()
{
	mTextBlob.alloc(mHdr.getTextInfo().size);
	(*mFile)->read(mTextBlob.data(), mHdr.getTextInfo().memory_offset, mTextBlob.size());
	mRoBlob.alloc(mHdr.getRoInfo().size);
	(*mFile)->read(mRoBlob.data(), mHdr.getRoInfo().memory_offset, mRoBlob.size());
	mDataBlob.alloc(mHdr.getDataInfo().size);
	(*mFile)->read(mDataBlob.data(), mHdr.getDataInfo().memory_offset, mDataBlob.size());
}

void NroProcess::displayHeader()
{
	std::cout << "[NRO Header]" << std::endl;
	std::cout << "  RoCrt:       " << fnd::SimpleTextOutput::arrayToString(mHdr.getRoCrt().data, nn::hac::nro::kRoCrtSize, false, "") << std::endl;
	std::cout << "  ModuleId:    " << fnd::SimpleTextOutput::arrayToString(mHdr.getModuleId().data, nn::hac::nro::kModuleIdSize, false, "") << std::endl;
	std::cout << "  NroSize:     0x" << std::hex << mHdr.getNroSize() << std::endl;
	std::cout << "  Program Sections:" << std::endl;
	std::cout << "     .text:" << std::endl;
	std::cout << "      Offset:     0x" << std::hex << mHdr.getTextInfo().memory_offset << std::endl;
	std::cout << "      Size:       0x" << std::hex << mHdr.getTextInfo().size << std::endl;
	std::cout << "    .ro:" << std::endl;
	std::cout << "      Offset:     0x" << std::hex << mHdr.getRoInfo().memory_offset << std::endl;
	std::cout << "      Size:       0x" << std::hex << mHdr.getRoInfo().size << std::endl;
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
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

void NroProcess::processRoMeta()
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