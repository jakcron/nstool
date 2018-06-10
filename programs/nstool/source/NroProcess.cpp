#include <sstream>
#include <fnd/SimpleTextOutput.h>
#include <fnd/MemoryBlob.h>
#include <compress/lz4.h>
#include <nx/nro-hb.h>
#include "OffsetAdjustedIFile.h"
#include "NroProcess.h"

NroProcess::NroProcess():
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false),
	mInstructionType(nx::npdm::INSTR_64BIT),
	mListApi(false),
	mListSymbols(false)
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
	importApiList();
	displayHeader();
	displayRoMetaData();
	if (mIsHomebrewNro)
		mAssetProc.process();
}

void NroProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void NroProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void NroProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void NroProcess::setInstructionType(nx::npdm::InstructionType type)
{
	mInstructionType = type;
}

void NroProcess::setListApi(bool listApi)
{
	mListApi = listApi;
}

void NroProcess::setListSymbols(bool listSymbols)
{
	mListSymbols = listSymbols;
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

	nx::sNroHeader* raw_hdr = (nx::sNroHeader*)scratch.getBytes();

	if (((le_uint64_t*)raw_hdr->reserved_0)->get() == nx::nro::kNroHomebrewSig && mFile->size() > mHdr.getNroSize())
	{
		mIsHomebrewNro = true;
		mAssetProc.setInputFile(new OffsetAdjustedIFile(mFile, false, mHdr.getNroSize(), mFile->size() - mHdr.getNroSize()), true);
		mAssetProc.setCliOutputMode(mCliOutputType);
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

void NroProcess::importApiList()
{
	struct sLayout { size_t offset; size_t size; } api_info, dyn_str, dyn_sym;

	api_info.offset = mHdr.getRoEmbeddedInfo().memory_offset;
	api_info.size = mHdr.getRoEmbeddedInfo().size;
	dyn_str.offset = mHdr.getRoDynStrInfo().memory_offset;
	dyn_str.size = mHdr.getRoDynStrInfo().size;
	dyn_sym.offset = mHdr.getRoDynSymInfo().memory_offset;
	dyn_sym.size = mHdr.getRoDynSymInfo().size;

	if (api_info.size > 0)
	{
		std::stringstream list_stream(std::string((char*)mRoBlob.getBytes() + api_info.offset, api_info.size));
		std::string api;

		while(std::getline(list_stream, api, (char)0x00))
		{
			mApiList.push_back(api);
		}
	}
	else
	{
		mApiList.clear();
	}

	if (dyn_sym.size > 0)
	{
		mDynSymbolList.parseData(mRoBlob.getBytes() + dyn_sym.offset, dyn_sym.size, mRoBlob.getBytes() + dyn_str.offset, dyn_str.size, mInstructionType == nx::npdm::INSTR_64BIT);
	}
}

void NroProcess::displayHeader()
{
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)
	if (mCliOutputType >= OUTPUT_NORMAL)
	{
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
		if (mCliOutputType >= OUTPUT_VERBOSE)
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
	}
	
#undef _HEXDUMP_L
}

void NroProcess::displayRoMetaData()
{
	if (mApiList.size() > 0 && (mListApi || mCliOutputType > OUTPUT_NORMAL))
	{
		printf("[SDK API List]\n");
		for (size_t i = 0; i < mApiList.size(); i++)
		{
			printf("  API %d:\n", (int)i);
			printf("    Type:     %s\n", getApiTypeStr(mApiList[i].getApiType()));
			printf("    Vender:   %s\n", mApiList[i].getVenderName().c_str());
			printf("    Module:   %s\n", mApiList[i].getModuleName().c_str());
		}
	}
	if (mDynSymbolList.getDynamicSymbolList().getSize() > 0 && (mListSymbols || mCliOutputType > OUTPUT_NORMAL))
	{
		printf("[Symbol List]\n");
		for (size_t i = 0; i < mDynSymbolList.getDynamicSymbolList().getSize(); i++)
		{
			const DynamicSymbolParser::sDynSymbol& symbol = mDynSymbolList.getDynamicSymbolList()[i];
			printf("  %s [SHN=%s (%04x)][STT=%s]\n", symbol.name.c_str(), getSectionIndexStr(symbol.shn_index), symbol.shn_index, getSymbolTypeStr(symbol.symbol_type));
		}
	}
	
}

const char* NroProcess::getApiTypeStr(SdkApiString::ApiType type) const
{
	const char* str;
	switch (type)
	{
		case (SdkApiString::API_MIDDLEWARE):
			str = "Middleware";
			break;
		case (SdkApiString::API_DEBUG):
			str = "Debug";
			break;
		case (SdkApiString::API_PRIVATE):
			str = "Private";
			break;
		case (SdkApiString::API_SDK_VERSION):
			str = "SDK Version";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}

const char* NroProcess::getSectionIndexStr(nx::dynsym::SpecialSectionIndex shn_index) const
{
	const char* str;
	switch (shn_index)
	{
		case (nx::dynsym::SHN_UNDEF):
			str = "UNDEF";
			break;
		case (nx::dynsym::SHN_EXPORT):
			str = "EXPORT";
			break;
		case (nx::dynsym::SHN_LOPROC):
			str = "LOPROC";
			break;
		case (nx::dynsym::SHN_HIPROC):
			str = "HIPROC";
			break;
		case (nx::dynsym::SHN_LOOS):
			str = "LOOS";
			break;
		case (nx::dynsym::SHN_HIOS):
			str = "HIOS";
			break;
		case (nx::dynsym::SHN_ABS):
			str = "ABS";
			break;
		case (nx::dynsym::SHN_COMMON):
			str = "COMMON";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}

const char* NroProcess::getSymbolTypeStr(nx::dynsym::SymbolType symbol_type) const
{
	const char* str;
	switch (symbol_type)
	{
		case (nx::dynsym::STT_NOTYPE):
			str = "NOTYPE";
			break;
		case (nx::dynsym::STT_OBJECT):
			str = "OBJECT";
			break;
		case (nx::dynsym::STT_FUNC):
			str = "FUNC";
			break;
		case (nx::dynsym::STT_SECTION):
			str = "SECTION";
			break;
		case (nx::dynsym::STT_FILE):
			str = "FILE";
			break;
		case (nx::dynsym::STT_LOOS):
			str = "LOOS";
			break;
		case (nx::dynsym::STT_HIOS):
			str = "HIOS";
			break;
		case (nx::dynsym::STT_LOPROC):
			str = "LOPROC";
			break;
		case (nx::dynsym::STT_HIPROC):
			str = "HIPROC";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}