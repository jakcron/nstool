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

	memcpy(&mNso, scratch.getBytes(), sizeof(nx::sNsoHeader));
	if (std::string(mNso.signature, 4) != nx::nso::kNsoSig)
	{
		throw fnd::Exception(kModuleName, "Corrupt NSO header");
	}

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
	printf("  Format Version:     %" PRId32 "\n", mNso.version.get());
	printf("  Flags:              0x%" PRIx32 "\n", mNso.flags.get());
	printf("  ModuleId:           ");
	_HEXDUMP_L(mNso.module_id, 32);
	printf("\n");
	printf("  Program Segments:\n");
	printf("     .module_id:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mNso.module_name_offset.get());
	printf("      FileSize:       0x%" PRIx32 "\n", mNso.module_name_size.get());
	printf("    .text:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mNso.text.file_offset.get());
	printf("      FileSize:       0x%" PRIx32 "%s\n", mNso.text_file_size.get(), _HAS_BIT(mNso.flags.get(), nx::nso::FLAG_TEXT_COMPRESS)? " (COMPRESSED)" : "");
	//printf("      Compressed:     %s\n", getBoolStr(_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_TEXT_COMPRESS)));
	printf("    .ro:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mNso.ro.file_offset.get());
	printf("      FileSize:       0x%" PRIx32 "%s\n", mNso.ro_file_size.get(), _HAS_BIT(mNso.flags.get(), nx::nso::FLAG_RO_COMPRESS)? " (COMPRESSED)" : "");
	//printf("      Compressed:     %s\n", getBoolStr(_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_RO_COMPRESS)));
	printf("    .data:\n");
	printf("      FileOffset:     0x%" PRIx32 "\n", mNso.data.file_offset.get());
	printf("      FileSize:       0x%" PRIx32 "%s\n", mNso.data_file_size.get(), _HAS_BIT(mNso.flags.get(), nx::nso::FLAG_DATA_COMPRESS)? " (COMPRESSED)" : "");
	//printf("      Compressed:     %s\n", getBoolStr(_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_DATA_COMPRESS)));
	printf("  Program Sections:\n");
	printf("     .text:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mNso.text.memory_offset.get());
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.text.size.get());
	//printf("      Hashed:         %s\n", getBoolStr(_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_TEXT_HASH)));
	if (_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_TEXT_HASH))
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mNso.text_hash, 32);
		printf("\n");
	}
	printf("    .ro:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mNso.ro.memory_offset.get());
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.ro.size.get());
	//printf("      Hashed:           %s\n", getBoolStr(_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_RO_HASH)));
	if (_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_RO_HASH))
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mNso.ro_hash, 32);
		printf("\n");
	}
	printf("    .api_info:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mNso.ro.memory_offset.get() + mNso.embedded.offset.get());
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.embedded.size.get());
	printf("    .dynstr:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mNso.ro.memory_offset.get() + mNso.dyn_str.offset.get());
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.dyn_str.size.get());
	printf("    .dynsym:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mNso.ro.memory_offset.get() + mNso.dyn_sym.offset.get());
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.dyn_sym.size.get());
	printf("    .data:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mNso.data.memory_offset.get());
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.data.size.get());
	//printf("      Hashed:         %s\n", getBoolStr(_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_DATA_HASH)));
	if (_HAS_BIT(mNso.flags.get(), nx::nso::FLAG_DATA_HASH))
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mNso.data_hash, 32);
		printf("\n");
	}
	printf("    .bss:\n");
	printf("      MemorySize:     0x%" PRIx32 "\n", mNso.bss_size.get());

#undef _HEXDUMP_L
}

