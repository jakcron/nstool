#include <fnd/SimpleTextOutput.h>
#include <fnd/MemoryBlob.h>
#include "OffsetAdjustedIFile.h"
#include "NsoProcess.h"

inline const char* getBoolStr(bool isTrue)
{
	return isTrue? "TRUE" : "FALSE";
}

NsoProcess::NsoProcess():
	mReader(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false)
{

}

NsoProcess::~NsoProcess()
{
	if (mReader != nullptr)
	{
		delete mReader;
	}
}

void NsoProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	if (mReader->size() < sizeof(nx::sNsoHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt NSO file too small");
	}

	scratch.alloc(sizeof(nx::sNsoHeader));
	mReader->read(scratch.getBytes(), 0, scratch.getSize());

	mHdr.importBinary(scratch.getBytes(), scratch.getSize());

	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		displayHeader();
	}
}

void NsoProcess::setInputFile(fnd::IFile* file, size_t offset, size_t size)
{
	mReader = new OffsetAdjustedIFile(file, offset, size);
}

void NsoProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void NsoProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void NsoProcess::displayHeader()
{
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	printf("[NSO Header]\n");
	printf("  ModuleId:           ");
	_HEXDUMP_L(mHdr.getModuleId().data, nx::nso::kModuleIdLen);
	printf("\n");
	printf("  Program Segments:\n");
	printf("     .module_id:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mHdr.getModuleNameInfo().offset);
	printf("      FileSize:       0x%" PRIx32 "\n", mHdr.getModuleNameInfo().size);
	printf("    .text:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mHdr.getTextSegmentInfo().file_layout.offset);
	printf("      FileSize:       0x%" PRIx32 "%s\n", mHdr.getTextSegmentInfo().file_layout.size, mHdr.getTextSegmentInfo().is_compressed? " (COMPRESSED)" : "");
	printf("    .ro:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().file_layout.offset);
	printf("      FileSize:       0x%" PRIx32 "%s\n", mHdr.getRoSegmentInfo().file_layout.size, mHdr.getRoSegmentInfo().is_compressed? " (COMPRESSED)" : "");
	printf("    .data:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mHdr.getDataSegmentInfo().file_layout.offset);
	printf("      FileSize:       0x%" PRIx32 "%s\n", mHdr.getDataSegmentInfo().file_layout.size, mHdr.getDataSegmentInfo().is_compressed? " (COMPRESSED)" : "");
	printf("  Program Sections:\n");
	printf("     .text:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getTextSegmentInfo().memory_layout.offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getTextSegmentInfo().memory_layout.size);
	if (mHdr.getTextSegmentInfo().is_hashed)
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mHdr.getTextSegmentInfo().hash.bytes, 32);
		printf("\n");
	}
	printf("    .ro:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.size);
	if (mHdr.getRoSegmentInfo().is_hashed)
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mHdr.getRoSegmentInfo().hash.bytes, 32);
		printf("\n");
	}
	printf("    .api_info:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.offset + mHdr.getRoEmbeddedInfo().offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoEmbeddedInfo().size);
	printf("    .dynstr:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.offset + mHdr.getRoDynStrInfo().offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoDynStrInfo().size);
	printf("    .dynsym:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.offset + mHdr.getRoDynSymInfo().offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoDynSymInfo().size);
	printf("    .data:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getDataSegmentInfo().memory_layout.offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getDataSegmentInfo().memory_layout.size);
	if (mHdr.getDataSegmentInfo().is_hashed)
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mHdr.getDataSegmentInfo().hash.bytes, 32);
		printf("\n");
	}
	printf("    .bss:\n");
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getBssSize());

#undef _HEXDUMP_L
}