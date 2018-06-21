#include <fnd/SimpleTextOutput.h>
#include <fnd/MemoryBlob.h>
#include <compress/lz4.h>
#include <nx/nro-hb.h>
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

void NroProcess::setInstructionType(nx::npdm::InstructionType type)
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

void NroProcess::importHeader()
{
	fnd::MemoryBlob scratch;
	if (mFile->size() < sizeof(nx::sNroHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt NRO: file too small");
	}

	scratch.alloc(sizeof(nx::sNroHeader));
	mFile->read(scratch.getBytes(), 0, scratch.getSize());

	mHdr.importBinary(scratch.getBytes(), scratch.getSize());

	// setup homebrew extension
	nx::sNroHeader* raw_hdr = (nx::sNroHeader*)scratch.getBytes();
	if (((le_uint64_t*)raw_hdr->reserved_0)->get() == nx::nro::kNroHomebrewSig && mFile->size() > mHdr.getNroSize())
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
	mFile->read(mTextBlob.getBytes(), mHdr.getTextInfo().memory_offset, mTextBlob.getSize());
	mRoBlob.alloc(mHdr.getRoInfo().size);
	mFile->read(mRoBlob.getBytes(), mHdr.getRoInfo().memory_offset, mRoBlob.getSize());
	mDataBlob.alloc(mHdr.getDataInfo().size);
	mFile->read(mDataBlob.getBytes(), mHdr.getDataInfo().memory_offset, mDataBlob.getSize());
}

void NroProcess::displayHeader()
{
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)
	printf("[NRO Header]\n");
	printf("  RoCrt:       ");
	_HEXDUMP_L(mHdr.getRoCrt().data, nx::nro::kRoCrtSize);
	printf("\n");
	printf("  ModuleId:    ");
	_HEXDUMP_L(mHdr.getModuleId().data, nx::nro::kModuleIdSize);
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
	if (mRoBlob.getSize())
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