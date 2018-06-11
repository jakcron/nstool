#include <sstream>
#include <fnd/SimpleTextOutput.h>
#include <fnd/MemoryBlob.h>
#include <compress/lz4.h>
#include "OffsetAdjustedIFile.h"
#include "NsoProcess.h"

NsoProcess::NsoProcess():
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false),
	mInstructionType(nx::npdm::INSTR_64BIT),
	mListApi(false),
	mListSymbols(false)
{
}

NsoProcess::~NsoProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void NsoProcess::process()
{
	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	importHeader();
	importCodeSegments();
	importApiList();
	displayNsoHeader();
	displayRoMetaData();
}

void NsoProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void NsoProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void NsoProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void NsoProcess::setInstructionType(nx::npdm::InstructionType type)
{
	mInstructionType = type;
}

void NsoProcess::setListApi(bool listApi)
{
	mListApi = listApi;
}

void NsoProcess::setListSymbols(bool listSymbols)
{
	mListSymbols = listSymbols;
}

void NsoProcess::importHeader()
{
	fnd::MemoryBlob scratch;
	if (mFile->size() < sizeof(nx::sNsoHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt NSO: file too small");
	}

	scratch.alloc(sizeof(nx::sNsoHeader));
	mFile->read(scratch.getBytes(), 0, scratch.getSize());

	mNsoHdr.importBinary(scratch.getBytes(), scratch.getSize());
}

void NsoProcess::importCodeSegments()
{
	fnd::MemoryBlob scratch;
	uint32_t decompressed_len;
	crypto::sha::sSha256Hash calc_hash;

	// process text segment
	if (mNsoHdr.getTextSegmentInfo().is_compressed)
	{
		scratch.alloc(mNsoHdr.getTextSegmentInfo().file_layout.size);
		mFile->read(scratch.getBytes(), mNsoHdr.getTextSegmentInfo().file_layout.offset, scratch.getSize());
		mTextBlob.alloc(mNsoHdr.getTextSegmentInfo().memory_layout.size);
		compress::lz4::decompressData(scratch.getBytes(), (uint32_t)scratch.getSize(), mTextBlob.getBytes(), (uint32_t)mTextBlob.getSize(), decompressed_len);
		if (decompressed_len != mTextBlob.getSize())
		{
			throw fnd::Exception(kModuleName, "NSO text segment failed to decompress");
		}
	}
	else
	{
		mTextBlob.alloc(mNsoHdr.getTextSegmentInfo().file_layout.size);
		mFile->read(mTextBlob.getBytes(), mNsoHdr.getTextSegmentInfo().file_layout.offset, mTextBlob.getSize());
	}
	if (mNsoHdr.getTextSegmentInfo().is_hashed)
	{
		crypto::sha::Sha256(mTextBlob.getBytes(), mTextBlob.getSize(), calc_hash.bytes);
		if (calc_hash != mNsoHdr.getTextSegmentInfo().hash)
		{
			throw fnd::Exception(kModuleName, "NSO text segment failed SHA256 verification");
		}
	}

	// process ro segment
	if (mNsoHdr.getRoSegmentInfo().is_compressed)
	{
		scratch.alloc(mNsoHdr.getRoSegmentInfo().file_layout.size);
		mFile->read(scratch.getBytes(), mNsoHdr.getRoSegmentInfo().file_layout.offset, scratch.getSize());
		mRoBlob.alloc(mNsoHdr.getRoSegmentInfo().memory_layout.size);
		compress::lz4::decompressData(scratch.getBytes(), (uint32_t)scratch.getSize(), mRoBlob.getBytes(), (uint32_t)mRoBlob.getSize(), decompressed_len);
		if (decompressed_len != mRoBlob.getSize())
		{
			throw fnd::Exception(kModuleName, "NSO ro segment failed to decompress");
		}
	}
	else
	{
		mRoBlob.alloc(mNsoHdr.getRoSegmentInfo().file_layout.size);
		mFile->read(mRoBlob.getBytes(), mNsoHdr.getRoSegmentInfo().file_layout.offset, mRoBlob.getSize());
	}
	if (mNsoHdr.getRoSegmentInfo().is_hashed)
	{
		crypto::sha::Sha256(mRoBlob.getBytes(), mRoBlob.getSize(), calc_hash.bytes);
		if (calc_hash != mNsoHdr.getRoSegmentInfo().hash)
		{
			throw fnd::Exception(kModuleName, "NSO ro segment failed SHA256 verification");
		}
	}

	// process data segment
	if (mNsoHdr.getDataSegmentInfo().is_compressed)
	{
		scratch.alloc(mNsoHdr.getDataSegmentInfo().file_layout.size);
		mFile->read(scratch.getBytes(), mNsoHdr.getDataSegmentInfo().file_layout.offset, scratch.getSize());
		mDataBlob.alloc(mNsoHdr.getDataSegmentInfo().memory_layout.size);
		compress::lz4::decompressData(scratch.getBytes(), (uint32_t)scratch.getSize(), mDataBlob.getBytes(), (uint32_t)mDataBlob.getSize(), decompressed_len);
		if (decompressed_len != mDataBlob.getSize())
		{
			throw fnd::Exception(kModuleName, "NSO data segment failed to decompress");
		}
	}
	else
	{
		mDataBlob.alloc(mNsoHdr.getDataSegmentInfo().file_layout.size);
		mFile->read(mDataBlob.getBytes(), mNsoHdr.getDataSegmentInfo().file_layout.offset, mDataBlob.getSize());
	}
	if (mNsoHdr.getDataSegmentInfo().is_hashed)
	{
		crypto::sha::Sha256(mDataBlob.getBytes(), mDataBlob.getSize(), calc_hash.bytes);
		if (calc_hash != mNsoHdr.getDataSegmentInfo().hash)
		{
			throw fnd::Exception(kModuleName, "NSO data segment failed SHA256 verification");
		}
	}
}

void NsoProcess::importApiList()
{
	struct sLayout { size_t offset; size_t size; } api_info, dyn_str, dyn_sym;

	api_info.offset = mNsoHdr.getRoEmbeddedInfo().offset;
	api_info.size = mNsoHdr.getRoEmbeddedInfo().size;
	dyn_str.offset = mNsoHdr.getRoDynStrInfo().offset;
	dyn_str.size = mNsoHdr.getRoDynStrInfo().size;
	dyn_sym.offset = mNsoHdr.getRoDynSymInfo().offset;
	dyn_sym.size = mNsoHdr.getRoDynSymInfo().size;

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

void NsoProcess::displayNsoHeader()
{
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		printf("[NSO Header]\n");
		printf("  ModuleId:           ");
		_HEXDUMP_L(mNsoHdr.getModuleId().data, nx::nso::kModuleIdSize);
		printf("\n");
		printf("  Program Segments:\n");
		printf("     .module_name:\n");
		printf("      FileOffset:     0x%" PRIx32 "\n", mNsoHdr.getModuleNameInfo().offset);
		printf("      FileSize:       0x%" PRIx32 "\n", mNsoHdr.getModuleNameInfo().size);
		printf("    .text:\n");
		printf("      FileOffset:     0x%" PRIx32 "\n", mNsoHdr.getTextSegmentInfo().file_layout.offset);
		printf("      FileSize:       0x%" PRIx32 "%s\n", mNsoHdr.getTextSegmentInfo().file_layout.size, mNsoHdr.getTextSegmentInfo().is_compressed? " (COMPRESSED)" : "");
		printf("    .ro:\n");
		printf("      FileOffset:     0x%" PRIx32 "\n", mNsoHdr.getRoSegmentInfo().file_layout.offset);
		printf("      FileSize:       0x%" PRIx32 "%s\n", mNsoHdr.getRoSegmentInfo().file_layout.size, mNsoHdr.getRoSegmentInfo().is_compressed? " (COMPRESSED)" : "");
		printf("    .data:\n");
		printf("      FileOffset:     0x%" PRIx32 "\n", mNsoHdr.getDataSegmentInfo().file_layout.offset);
		printf("      FileSize:       0x%" PRIx32 "%s\n", mNsoHdr.getDataSegmentInfo().file_layout.size, mNsoHdr.getDataSegmentInfo().is_compressed? " (COMPRESSED)" : "");
		printf("  Program Sections:\n");
		printf("     .text:\n");
		printf("      MemoryOffset:   0x%" PRIx32 "\n", mNsoHdr.getTextSegmentInfo().memory_layout.offset);
		printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getTextSegmentInfo().memory_layout.size);
		if (mNsoHdr.getTextSegmentInfo().is_hashed && mCliOutputType >= OUTPUT_VERBOSE)
		{
			printf("      Hash:           ");
			_HEXDUMP_L(mNsoHdr.getTextSegmentInfo().hash.bytes, 32);
			printf("\n");
		}
		printf("    .ro:\n");
		printf("      MemoryOffset:   0x%" PRIx32 "\n", mNsoHdr.getRoSegmentInfo().memory_layout.offset);
		printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getRoSegmentInfo().memory_layout.size);
		if (mNsoHdr.getRoSegmentInfo().is_hashed && mCliOutputType >= OUTPUT_VERBOSE)
		{
			printf("      Hash:           ");
			_HEXDUMP_L(mNsoHdr.getRoSegmentInfo().hash.bytes, 32);
			printf("\n");
		}
		if (mCliOutputType >= OUTPUT_VERBOSE)
		{
			printf("    .api_info:\n");
			printf("      MemoryOffset:   0x%" PRIx32 "\n",  mNsoHdr.getRoEmbeddedInfo().offset);
			printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getRoEmbeddedInfo().size);
			printf("    .dynstr:\n");
			printf("      MemoryOffset:   0x%" PRIx32 "\n", mNsoHdr.getRoDynStrInfo().offset);
			printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getRoDynStrInfo().size);
			printf("    .dynsym:\n");
			printf("      MemoryOffset:   0x%" PRIx32 "\n", mNsoHdr.getRoDynSymInfo().offset);
			printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getRoDynSymInfo().size);
		}
		
		printf("    .data:\n");
		printf("      MemoryOffset:   0x%" PRIx32 "\n", mNsoHdr.getDataSegmentInfo().memory_layout.offset);
		printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getDataSegmentInfo().memory_layout.size);
		if (mNsoHdr.getDataSegmentInfo().is_hashed && mCliOutputType >= OUTPUT_VERBOSE)
		{
			printf("      Hash:           ");
			_HEXDUMP_L(mNsoHdr.getDataSegmentInfo().hash.bytes, 32);
			printf("\n");
		}
		printf("    .bss:\n");
		printf("      MemorySize:     0x%" PRIx32 "\n", mNsoHdr.getBssSize());
	}
#undef _HEXDUMP_L
}

void NsoProcess::displayRoMetaData()
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

const char* NsoProcess::getApiTypeStr(SdkApiString::ApiType type) const
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

const char* NsoProcess::getSectionIndexStr(nx::dynsym::SpecialSectionIndex shn_index) const
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

const char* NsoProcess::getSymbolTypeStr(nx::dynsym::SymbolType symbol_type) const
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