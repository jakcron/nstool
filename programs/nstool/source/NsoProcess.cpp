#include <sstream>
#include <fnd/SimpleTextOutput.h>
#include <fnd/MemoryBlob.h>
#include <compress/lz4.h>
#include "OffsetAdjustedIFile.h"
#include "NsoProcess.h"

NsoProcess::NsoProcess():
	mReader(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false)
{
	mArchType.isSet = false;
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
	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	importHeader();
	importCodeSegments();
	importApiList();

	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		displayHeader();
		displayRoApiList();
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

void NsoProcess::setArchType(nx::npdm::InstructionType type)
{
	mArchType = type;
}

const nx::NsoHeader& NsoProcess::getNsoHeader() const
{
	return mHdr;
}

const fnd::MemoryBlob& NsoProcess::getTextBlob() const
{
	return mTextBlob;
}

const fnd::MemoryBlob& NsoProcess::getRoBlob() const
{
	return mRoBlob;
}

const fnd::MemoryBlob& NsoProcess::getDataBlob() const
{
	return mDataBlob;
}

const std::vector<std::string>& NsoProcess::getApiList() const
{
	return mApiList;
}


void NsoProcess::importHeader()
{
	fnd::MemoryBlob scratch;
	if (mReader->size() < sizeof(nx::sNsoHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt NSO file too small");
	}

	scratch.alloc(sizeof(nx::sNsoHeader));
	mReader->read(scratch.getBytes(), 0, scratch.getSize());

	mHdr.importBinary(scratch.getBytes(), scratch.getSize());
}

void NsoProcess::importCodeSegments()
{
	fnd::MemoryBlob scratch;
	uint32_t decompressed_len;
	crypto::sha::sSha256Hash calc_hash;

	// process text segment
	if (mHdr.getTextSegmentInfo().is_compressed)
	{
		scratch.alloc(mHdr.getTextSegmentInfo().file_layout.size);
		mReader->read(scratch.getBytes(), mHdr.getTextSegmentInfo().file_layout.offset, scratch.getSize());
		mTextBlob.alloc(mHdr.getTextSegmentInfo().memory_layout.size);
		compress::lz4::decompressData(scratch.getBytes(), scratch.getSize(), mTextBlob.getBytes(), mTextBlob.getSize(), decompressed_len);
		if (decompressed_len != mTextBlob.getSize())
		{
			throw fnd::Exception(kModuleName, "NSO text segment failed to decompress");
		}
	}
	else
	{
		mTextBlob.alloc(mHdr.getTextSegmentInfo().file_layout.size);
		mReader->read(mTextBlob.getBytes(), mHdr.getTextSegmentInfo().file_layout.offset, mTextBlob.getSize());
	}
	if (mHdr.getTextSegmentInfo().is_hashed)
	{
		crypto::sha::Sha256(mTextBlob.getBytes(), mTextBlob.getSize(), calc_hash.bytes);
		if (calc_hash != mHdr.getTextSegmentInfo().hash)
		{
			throw fnd::Exception(kModuleName, "NSO text segment failed SHA256 verification");
		}
	}

	// process ro segment
	if (mHdr.getRoSegmentInfo().is_compressed)
	{
		scratch.alloc(mHdr.getRoSegmentInfo().file_layout.size);
		mReader->read(scratch.getBytes(), mHdr.getRoSegmentInfo().file_layout.offset, scratch.getSize());
		mRoBlob.alloc(mHdr.getRoSegmentInfo().memory_layout.size);
		compress::lz4::decompressData(scratch.getBytes(), scratch.getSize(), mRoBlob.getBytes(), mRoBlob.getSize(), decompressed_len);
		if (decompressed_len != mRoBlob.getSize())
		{
			throw fnd::Exception(kModuleName, "NSO ro segment failed to decompress");
		}
	}
	else
	{
		mRoBlob.alloc(mHdr.getRoSegmentInfo().file_layout.size);
		mReader->read(mRoBlob.getBytes(), mHdr.getRoSegmentInfo().file_layout.offset, mRoBlob.getSize());
	}
	if (mHdr.getRoSegmentInfo().is_hashed)
	{
		crypto::sha::Sha256(mRoBlob.getBytes(), mRoBlob.getSize(), calc_hash.bytes);
		if (calc_hash != mHdr.getRoSegmentInfo().hash)
		{
			throw fnd::Exception(kModuleName, "NSO ro segment failed SHA256 verification");
		}
	}

	// process data segment
	if (mHdr.getDataSegmentInfo().is_compressed)
	{
		scratch.alloc(mHdr.getDataSegmentInfo().file_layout.size);
		mReader->read(scratch.getBytes(), mHdr.getDataSegmentInfo().file_layout.offset, scratch.getSize());
		mDataBlob.alloc(mHdr.getDataSegmentInfo().memory_layout.size);
		compress::lz4::decompressData(scratch.getBytes(), scratch.getSize(), mDataBlob.getBytes(), mDataBlob.getSize(), decompressed_len);
		if (decompressed_len != mDataBlob.getSize())
		{
			throw fnd::Exception(kModuleName, "NSO data segment failed to decompress");
		}
	}
	else
	{
		mDataBlob.alloc(mHdr.getDataSegmentInfo().file_layout.size);
		mReader->read(mDataBlob.getBytes(), mHdr.getDataSegmentInfo().file_layout.offset, mDataBlob.getSize());
	}
	if (mHdr.getDataSegmentInfo().is_hashed)
	{
		crypto::sha::Sha256(mDataBlob.getBytes(), mDataBlob.getSize(), calc_hash.bytes);
		if (calc_hash != mHdr.getDataSegmentInfo().hash)
		{
			throw fnd::Exception(kModuleName, "NSO data segment failed SHA256 verification");
		}
	}
	
}

void NsoProcess::importApiList()
{
	if (mHdr.getRoEmbeddedInfo().size > 0)
	{
		std::stringstream list_stream(std::string((char*)mRoBlob.getBytes() + mHdr.getRoEmbeddedInfo().offset, mHdr.getRoEmbeddedInfo().size));
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

	if (mHdr.getRoDynSymInfo().size > 0 && mArchType.isSet == true)
	{
		mDynSymbolList.parseData(mRoBlob.getBytes() + mHdr.getRoDynSymInfo().offset, mHdr.getRoDynSymInfo().size, mRoBlob.getBytes() + mHdr.getRoDynStrInfo().offset, mHdr.getRoDynStrInfo().size, *mArchType == nx::npdm::INSTR_64BIT);
	}
}

void NsoProcess::displayHeader()
{
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	printf("[NSO Header]\n");
	printf("  ModuleId:           ");
	_HEXDUMP_L(mHdr.getModuleId().data, nx::nso::kModuleIdLen);
	printf("\n");
	printf("  Program Segments:\n");
	printf("     .module_name:\n");
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
	if (mHdr.getTextSegmentInfo().is_hashed && mCliOutputType >= OUTPUT_VERBOSE)
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mHdr.getTextSegmentInfo().hash.bytes, 32);
		printf("\n");
	}
	printf("    .ro:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoSegmentInfo().memory_layout.size);
	if (mHdr.getRoSegmentInfo().is_hashed && mCliOutputType >= OUTPUT_VERBOSE)
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mHdr.getRoSegmentInfo().hash.bytes, 32);
		printf("\n");
	}
	if (mCliOutputType >= OUTPUT_VERBOSE)
	{
		printf("    .api_info:\n");
		printf("      MemoryOffset:   0x%" PRIx32 "\n",  mHdr.getRoEmbeddedInfo().offset);
		printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoEmbeddedInfo().size);
		printf("    .dynstr:\n");
		printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoDynStrInfo().offset);
		printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoDynStrInfo().size);
		printf("    .dynsym:\n");
		printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getRoDynSymInfo().offset);
		printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getRoDynSymInfo().size);
	}
	
	printf("    .data:\n");
	printf("      MemoryOffset:   0x%" PRIx32 "\n", mHdr.getDataSegmentInfo().memory_layout.offset);
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getDataSegmentInfo().memory_layout.size);
	if (mHdr.getDataSegmentInfo().is_hashed && mCliOutputType >= OUTPUT_VERBOSE)
	{
		printf("      Hash:           ");
		_HEXDUMP_L(mHdr.getDataSegmentInfo().hash.bytes, 32);
		printf("\n");
	}
	printf("    .bss:\n");
	printf("      MemorySize:     0x%" PRIx32 "\n", mHdr.getBssSize());
	
#undef _HEXDUMP_L
}

void NsoProcess::displayRoApiList()
{
	if (mApiList.size() > 0 || mDynSymbolList.getDynamicSymbolList().getSize() > 0)
	{
		printf("[NSO RO Segment]\n");
		if (mApiList.size() > 0)
		{
			printf("  API List:\n");
			for (size_t i = 0; i < mApiList.size(); i++)
			{
				printf("    %s\n", mApiList[i].c_str());
			}
		}
		if (mDynSymbolList.getDynamicSymbolList().getSize() > 0)
		{
			printf("  Undefined Symbol List:\n");
			for (size_t i = 0; i < mDynSymbolList.getDynamicSymbolList().getSize(); i++)
			{
				if (mDynSymbolList.getDynamicSymbolList()[i].shn_index == nx::dynsym::SHN_UNDEF && (mDynSymbolList.getDynamicSymbolList()[i].symbol_type == nx::dynsym::STT_FUNC || mDynSymbolList.getDynamicSymbolList()[i].symbol_type == nx::dynsym::STT_NOTYPE))
					printf("    %s\n", mDynSymbolList.getDynamicSymbolList()[i].name.c_str());
			}
		}
	}
}