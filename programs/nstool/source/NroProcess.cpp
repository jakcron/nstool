#include <fnd/SimpleTextOutput.h>
#include <fnd/Vec.h>
#include <fnd/lz4.h>
#include <nn/hac/nro-hb.h>
#include "OffsetAdjustedIFile.h"
#include "NroProcess.h"

NroProcess::NroProcess():
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

NroProcess::~NroProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void NroProcess::process()
{
	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	importHeader();
	importCodeSegments();

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayHeader();

	processRoMeta();

	if (mIsHomebrewNro)
		mAssetProc.process();
}

void NroProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
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
	if (mFile->size() < sizeof(nn::hac::sNroHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt NRO: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sNroHeader));
	mFile->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());

	// setup homebrew extension
	nn::hac::sNroHeader* raw_hdr = (nn::hac::sNroHeader*)scratch.data();
	if (((le_uint64_t*)raw_hdr->reserved_0)->get() == nn::hac::nro::kNroHomebrewStructMagic && mFile->size() > mHdr.getNroSize())
	{
		mIsHomebrewNro = true;
		mAssetProc.setInputFile(new OffsetAdjustedIFile(mFile, false, mHdr.getNroSize(), mFile->size() - mHdr.getNroSize()), true);
		mAssetProc.setCliOutputMode(mCliOutputMode);
		mAssetProc.setVerifyMode(mVerify);
	}
	else
		mIsHomebrewNro = false;
}

void NroProcess::importCodeSegments()
{
	mTextBlob.alloc(mHdr.getTextInfo().size);
	mFile->read(mTextBlob.data(), mHdr.getTextInfo().memory_offset, mTextBlob.size());
	mRoBlob.alloc(mHdr.getRoInfo().size);
	mFile->read(mRoBlob.data(), mHdr.getRoInfo().memory_offset, mRoBlob.size());
	mDataBlob.alloc(mHdr.getDataInfo().size);
	mFile->read(mDataBlob.data(), mHdr.getDataInfo().memory_offset, mDataBlob.size());
}

void NroProcess::displayHeader()
{
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)
	printf("[NRO Header]\n");
	printf("  RoCrt:       ");
	_HEXDUMP_L(mHdr.getRoCrt().data, nn::hac::nro::kRoCrtSize);
	printf("\n");
	printf("  ModuleId:    ");
	_HEXDUMP_L(mHdr.getModuleId().data, nn::hac::nro::kModuleIdSize);
	printf("\n");
	printf("  NroSize:     0x%" PRIx32 "\n", mHdr.getNroSize());
	printf("  Program Sections:\n");
	printf("     .text:\n");
	printf("      Offset:     0x%" PRIx32 "\n", mHdr.getTextInfo().memory_offset);
	printf("      Size:       0x%" PRIx32 "\n", mHdr.getTextInfo().size);
	printf("    .ro:\n");
	printf("      Offset:     0x%" PRIx32 "\n", mHdr.getRoInfo().memory_offset);
	printf("      Size:       0x%" PRIx32 "\n", mHdr.getRoInfo().size);
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("    .api_info:\n");
		printf("      Offset:     0x%" PRIx32 "\n",  mHdr.getRoEmbeddedInfo().memory_offset);
		printf("      Size:       0x%" PRIx32 "\n", mHdr.getRoEmbeddedInfo().size);
		printf("    .dynstr:\n");
		printf("      Offset:     0x%" PRIx32 "\n", mHdr.getRoDynStrInfo().memory_offset);
		printf("      Size:       0x%" PRIx32 "\n", mHdr.getRoDynStrInfo().size);
		printf("    .dynsym:\n");
		printf("      Offset:     0x%" PRIx32 "\n", mHdr.getRoDynSymInfo().memory_offset);
		printf("      Size:       0x%" PRIx32 "\n", mHdr.getRoDynSymInfo().size);
	}
	printf("    .data:\n");
	printf("      Offset:     0x%" PRIx32 "\n", mHdr.getDataInfo().memory_offset);
	printf("      Size:       0x%" PRIx32 "\n", mHdr.getDataInfo().size);
	printf("    .bss:\n");
	printf("      Size:       0x%" PRIx32 "\n", mHdr.getBssSize());
	
#undef _HEXDUMP_L
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