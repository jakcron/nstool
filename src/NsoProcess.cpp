#include "NsoProcess.h"

#include <lz4.h>

nstool::NsoProcess::NsoProcess() :
	mModuleName("nstool::NsoProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mIs64BitInstruction(true),
	mListApi(false),
	mListSymbols(false)
{
}

void nstool::NsoProcess::process()
{
	importHeader();
	importCodeSegments();
	if (mCliOutputMode.show_basic_info)
		displayNsoHeader();

	processRoMeta();
}

void nstool::NsoProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::NsoProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::NsoProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::NsoProcess::setIs64BitInstruction(bool flag)
{
	mRoMeta.setIs64BitInstruction(flag);
}

void nstool::NsoProcess::setListApi(bool listApi)
{
	mRoMeta.setListApi(listApi);
}

void nstool::NsoProcess::setListSymbols(bool listSymbols)
{
	mRoMeta.setListSymbols(listSymbols);
}

const nstool::RoMetadataProcess& nstool::NsoProcess::getRoMetadataProcess() const
{
	return mRoMeta;
}

void nstool::NsoProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is smaller than NSO header size
	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());
	if (file_size < sizeof(pie::hac::sNsoHeader))
	{
		throw tc::Exception(mModuleName, "Corrupt NSO: file too small.");
	}

	// read nso
	tc::ByteData scratch = tc::ByteData(sizeof(pie::hac::sNsoHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// parse nso header
	mHdr.fromBytes(scratch.data(), scratch.size());
}

void nstool::NsoProcess::importCodeSegments()
{
	tc::ByteData scratch;
	pie::hac::detail::sha256_hash_t calc_hash;

	// process text segment
	if (mHdr.getTextSegmentInfo().is_compressed)
	{
		// allocate/read compressed text
		scratch = tc::ByteData(mHdr.getTextSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getTextSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(scratch.data(), scratch.size());

		// allocate for decompressed text segment
		mTextBlob = tc::ByteData(mHdr.getTextSegmentInfo().memory_layout.size);

		// decompress text segment
		if (decompressData(scratch.data(), scratch.size(), mTextBlob.data(), mTextBlob.size()) != mTextBlob.size())
		{
			throw tc::Exception(mModuleName, "NSO text segment failed to decompress");
		}
	}
	else
	{
		// read text segment directly (not compressed)
		mTextBlob = tc::ByteData(mHdr.getTextSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getTextSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(mTextBlob.data(), mTextBlob.size());
	}
	if (mHdr.getTextSegmentInfo().is_hashed)
	{
		tc::crypto::GenerateSha2256Hash(calc_hash.data(), mTextBlob.data(), mTextBlob.size());
		if (calc_hash != mHdr.getTextSegmentInfo().hash)
		{
			throw tc::Exception(mModuleName, "NSO text segment failed SHA256 verification");
		}
	}

	// process ro segment
	if (mHdr.getRoSegmentInfo().is_compressed)
	{
		// allocate/read compressed ro segment
		scratch = tc::ByteData(mHdr.getRoSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getRoSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(scratch.data(), scratch.size());

		// allocate for decompressed ro segment
		mRoBlob = tc::ByteData(mHdr.getRoSegmentInfo().memory_layout.size);

		// decompress ro segment
		if (decompressData(scratch.data(), scratch.size(), mRoBlob.data(), mRoBlob.size()) != mRoBlob.size())
		{
			throw tc::Exception(mModuleName, "NSO ro segment failed to decompress");
		}
	}
	else
	{
		// read ro segment directly (not compressed)
		mRoBlob = tc::ByteData(mHdr.getRoSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getRoSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(mRoBlob.data(), mRoBlob.size());
	}
	if (mHdr.getRoSegmentInfo().is_hashed)
	{
		tc::crypto::GenerateSha2256Hash(calc_hash.data(), mRoBlob.data(), mRoBlob.size());
		if (calc_hash != mHdr.getRoSegmentInfo().hash)
		{
			throw tc::Exception(mModuleName, "NSO ro segment failed SHA256 verification");
		}
	}

	// process ro segment
	if (mHdr.getDataSegmentInfo().is_compressed)
	{
		// allocate/read compressed ro segment
		scratch = tc::ByteData(mHdr.getDataSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getDataSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(scratch.data(), scratch.size());

		// allocate for decompressed ro segment
		mDataBlob = tc::ByteData(mHdr.getDataSegmentInfo().memory_layout.size);

		// decompress ro segment
		if (decompressData(scratch.data(), scratch.size(), mDataBlob.data(), mDataBlob.size()) != mDataBlob.size())
		{
			throw tc::Exception(mModuleName, "NSO data segment failed to decompress");
		}
	}
	else
	{
		// read ro segment directly (not compressed)
		mDataBlob = tc::ByteData(mHdr.getDataSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getDataSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(mDataBlob.data(), mDataBlob.size());
	}
	if (mHdr.getDataSegmentInfo().is_hashed)
	{
		tc::crypto::GenerateSha2256Hash(calc_hash.data(), mDataBlob.data(), mDataBlob.size());
		if (calc_hash != mHdr.getDataSegmentInfo().hash)
		{
			throw tc::Exception(mModuleName, "NSO data segment failed SHA256 verification");
		}
	}
}

void nstool::NsoProcess::displayNsoHeader()
{
	fmt::print("[NSO Header]\n");
	fmt::print("  ModuleId:           {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getModuleId().data(), mHdr.getModuleId().size(), false, ""));
	if (mCliOutputMode.show_layout)
	{
		fmt::print("  Program Segments:\n");
		fmt::print("     .module_name:\n");
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getModuleNameInfo().offset);
		fmt::print("      FileSize:       0x{:x}\n", mHdr.getModuleNameInfo().size);
		fmt::print("    .text:\n");
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getTextSegmentInfo().file_layout.offset);
		fmt::print("      FileSize:       0x{:x}{:s}\n", mHdr.getTextSegmentInfo().file_layout.size, (mHdr.getTextSegmentInfo().is_compressed? " (COMPRESSED)" : ""));
		fmt::print("    .ro:\n");
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getRoSegmentInfo().file_layout.offset);
		fmt::print("      FileSize:       0x{:x}{:s}\n", mHdr.getRoSegmentInfo().file_layout.size, (mHdr.getRoSegmentInfo().is_compressed? " (COMPRESSED)" : ""));
		fmt::print("    .data:\n");
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getDataSegmentInfo().file_layout.offset);
		fmt::print("      FileSize:       0x{:x}{:s}\n", mHdr.getDataSegmentInfo().file_layout.size, (mHdr.getDataSegmentInfo().is_compressed? " (COMPRESSED)" : ""));
	}
	fmt::print("  Program Sections:\n");
	fmt::print("     .text:\n");
	fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getTextSegmentInfo().memory_layout.offset);
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getTextSegmentInfo().memory_layout.size);
	if (mHdr.getTextSegmentInfo().is_hashed && mCliOutputMode.show_extended_info)
	{
		fmt::print("      Hash:           {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getTextSegmentInfo().hash.data(), mHdr.getTextSegmentInfo().hash.size(), false, ""));
	}
	fmt::print("    .ro:\n");
	fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getRoSegmentInfo().memory_layout.offset);
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getRoSegmentInfo().memory_layout.size);
	if (mHdr.getRoSegmentInfo().is_hashed && mCliOutputMode.show_extended_info)
	{
		fmt::print("      Hash:           {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getRoSegmentInfo().hash.data(), mHdr.getRoSegmentInfo().hash.size(), false, ""));
	}
	if (mCliOutputMode.show_extended_info)
	{
		fmt::print("    .api_info:\n");
		fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getRoEmbeddedInfo().offset);
		fmt::print("      MemorySize:     0x{:x}\n", mHdr.getRoEmbeddedInfo().size);
		fmt::print("    .dynstr:\n");
		fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getRoDynStrInfo().offset);
		fmt::print("      MemorySize:     0x{:x}\n", mHdr.getRoDynStrInfo().size);
		fmt::print("    .dynsym:\n");
		fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getRoDynSymInfo().offset);
		fmt::print("      MemorySize:     0x{:x}\n", mHdr.getRoDynSymInfo().size);
	}
	
	fmt::print("    .data:\n");
	fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getDataSegmentInfo().memory_layout.offset);
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getDataSegmentInfo().memory_layout.size);
	if (mHdr.getDataSegmentInfo().is_hashed && mCliOutputMode.show_extended_info)
	{
		fmt::print("      Hash:           {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getDataSegmentInfo().hash.data(), mHdr.getDataSegmentInfo().hash.size(), false, ""));
	}
	fmt::print("    .bss:\n");
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getBssSize());
}

void nstool::NsoProcess::processRoMeta()
{
	if (mRoBlob.size())
	{
		// setup ro metadata
		mRoMeta.setApiInfo(mHdr.getRoEmbeddedInfo().offset, mHdr.getRoEmbeddedInfo().size);
		mRoMeta.setDynSym(mHdr.getRoDynSymInfo().offset, mHdr.getRoDynSymInfo().size);
		mRoMeta.setDynStr(mHdr.getRoDynStrInfo().offset, mHdr.getRoDynStrInfo().size);
		mRoMeta.setRoBinary(mRoBlob);
		mRoMeta.setCliOutputMode(mCliOutputMode);
		mRoMeta.process();
	}
}

size_t nstool::NsoProcess::decompressData(const byte_t* src, size_t src_len, byte_t* dst, size_t dst_capacity)
{
	if (src_len >= LZ4_MAX_INPUT_SIZE)
	{
		return 0;
	}

	int32_t src_len_input = int32_t(src_len);
	int32_t dst_capcacity_input = (dst_capacity < LZ4_MAX_INPUT_SIZE) ? int32_t(dst_capacity) : LZ4_MAX_INPUT_SIZE;

	int32_t decomp_size = LZ4_decompress_safe((const char*)src, (char*)dst, src_len_input, dst_capcacity_input);

	if (decomp_size < 0)
	{
		memset(dst, 0, dst_capacity);
		return 0;
	}

	return size_t(decomp_size);
}