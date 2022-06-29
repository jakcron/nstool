#include "KipProcess.h"

#include <pietendo/hac/KernelCapabilityUtil.h>

#include <tc/NotImplementedException.h>

nstool::KipProcess::KipProcess() :
	mModuleName("nstool::KipProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::KipProcess::process()
{
	importHeader();
	//importCodeSegments(); // code segments not imported because compression not supported yet
	if (mCliOutputMode.show_basic_info)
	{
		displayHeader();
		displayKernelCap(mHdr.getKernelCapabilities());
	}
}

void nstool::KipProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::KipProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::KipProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::KipProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is smaller than KIP header size
	if (tc::io::IOUtil::castInt64ToSize(mFile->length()) < sizeof(pie::hac::sKipHeader))
	{
		throw tc::Exception(mModuleName, "Corrupt KIP: file too small.");
	}

	// read kip
	tc::ByteData scratch = tc::ByteData(sizeof(pie::hac::sKipHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// parse kip header
	mHdr.fromBytes(scratch.data(), scratch.size());
}

void nstool::KipProcess::importCodeSegments()
{
	tc::ByteData scratch;

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
			throw tc::Exception(mModuleName, "KIP text segment failed to decompress");
		}
	}
	else
	{
		// read text segment directly (not compressed)
		mTextBlob = tc::ByteData(mHdr.getTextSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getTextSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(mTextBlob.data(), mTextBlob.size());
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
			throw tc::Exception(mModuleName, "KIP ro segment failed to decompress");
		}
	}
	else
	{
		// read ro segment directly (not compressed)
		mRoBlob = tc::ByteData(mHdr.getRoSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getRoSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(mRoBlob.data(), mRoBlob.size());
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
			throw tc::Exception(mModuleName, "KIP data segment failed to decompress");
		}
	}
	else
	{
		// read ro segment directly (not compressed)
		mDataBlob = tc::ByteData(mHdr.getDataSegmentInfo().file_layout.size);
		mFile->seek(mHdr.getDataSegmentInfo().file_layout.offset, tc::io::SeekOrigin::Begin);
		mFile->read(mDataBlob.data(), mDataBlob.size());
	}
}

size_t nstool::KipProcess::decompressData(const byte_t* src, size_t src_len, byte_t* dst, size_t dst_capacity)
{
	throw tc::NotImplementedException(mModuleName, "KIP decompression not implemented yet.");
}

void nstool::KipProcess::displayHeader()
{
	fmt::print("[KIP Header]\n");
	fmt::print("  Meta:\n");
	fmt::print("    Name:                {:s}\n", mHdr.getName());
	fmt::print("    TitleId:             0x{:016x}\n", mHdr.getTitleId());
	fmt::print("    Version:             v{:d}\n", mHdr.getVersion());
	fmt::print("    Is64BitInstruction:  {}\n", mHdr.getIs64BitInstructionFlag());
	fmt::print("    Is64BitAddressSpace: {}\n", mHdr.getIs64BitAddressSpaceFlag());
	fmt::print("    UseSecureMemory:     {}\n", mHdr.getUseSecureMemoryFlag());
	fmt::print("  Program Sections:\n");
	fmt::print("     .text:\n");
	if (mCliOutputMode.show_layout)
	{
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getTextSegmentInfo().file_layout.offset);
		fmt::print("      FileSize:       0x{:x}{:s}\n", mHdr.getTextSegmentInfo().file_layout.size, (mHdr.getTextSegmentInfo().is_compressed? " (COMPRESSED)" : ""));
	}
	fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getTextSegmentInfo().memory_layout.offset);
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getTextSegmentInfo().memory_layout.size);
	fmt::print("    .ro:\n");
	if (mCliOutputMode.show_layout)
	{
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getRoSegmentInfo().file_layout.offset);
		fmt::print("      FileSize:       0x{:x}{:s}\n", mHdr.getRoSegmentInfo().file_layout.size, (mHdr.getRoSegmentInfo().is_compressed? " (COMPRESSED)" : ""));
	}
	fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getRoSegmentInfo().memory_layout.offset);
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getRoSegmentInfo().memory_layout.size);
	fmt::print("    .data:\n");
	if (mCliOutputMode.show_layout)
	{
		fmt::print("      FileOffset:     0x{:x}\n", mHdr.getDataSegmentInfo().file_layout.offset);
		fmt::print("      FileSize:       0x{:x}{:s}\n", mHdr.getDataSegmentInfo().file_layout.size, (mHdr.getDataSegmentInfo().is_compressed? " (COMPRESSED)" : ""));
	}
	fmt::print("      MemoryOffset:   0x{:x}\n", mHdr.getDataSegmentInfo().memory_layout.offset);
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getDataSegmentInfo().memory_layout.size);
	fmt::print("    .bss:\n");
	fmt::print("      MemorySize:     0x{:x}\n", mHdr.getBssSize());

}

void nstool::KipProcess::displayKernelCap(const pie::hac::KernelCapabilityControl& kern)
{
	fmt::print("[Kernel Capabilities]\n");
	if (kern.getThreadInfo().isSet())
	{
		pie::hac::ThreadInfoHandler threadInfo = kern.getThreadInfo();
		fmt::print("  Thread Priority:\n");
		fmt::print("    Min:     {:d}\n", threadInfo.getMinPriority());
		fmt::print("    Max:     {:d}\n", threadInfo.getMaxPriority());
		fmt::print("  CpuId:\n");
		fmt::print("    Min:     {:d}\n", threadInfo.getMinCpuId());
		fmt::print("    Max:     {:d}\n", threadInfo.getMaxCpuId());
	}

	if (kern.getSystemCalls().isSet())
	{
		auto syscall_ids = kern.getSystemCalls().getSystemCallIds();
		fmt::print("  SystemCalls:\n");
		std::vector<std::string> syscall_names;
		for (size_t syscall_id = 0; syscall_id < syscall_ids.size(); syscall_id++)
		{
			if (syscall_ids.test(syscall_id))
				syscall_names.push_back(pie::hac::KernelCapabilityUtil::getSystemCallIdAsString(pie::hac::kc::SystemCallId(syscall_id)));
		}
		fmt::print("{:s}", tc::cli::FormatUtil::formatListWithLineLimit(syscall_names, 60, 4));
	}
	if (kern.getMemoryMaps().isSet())
	{
		auto maps = kern.getMemoryMaps().getMemoryMaps();
		auto ioMaps = kern.getMemoryMaps().getIoMemoryMaps();

		fmt::print("  MemoryMaps:\n");
		for (size_t i = 0; i < maps.size(); i++)
		{
			fmt::print("    {:s}\n", formatMappingAsString(maps[i]));	
		}
		//fmt::print("  IoMaps:\n");
		for (size_t i = 0; i < ioMaps.size(); i++)
		{
			fmt::print("    {:s}\n", formatMappingAsString(ioMaps[i]));
		}
	}
	if (kern.getInterupts().isSet())
	{
		std::vector<std::string> interupts;
		for (auto itr = kern.getInterupts().getInteruptList().begin(); itr != kern.getInterupts().getInteruptList().end(); itr++)
		{
			interupts.push_back(fmt::format("0x{:x}", *itr));
		}
		fmt::print("  Interupts Flags:\n");
		fmt::print("{:s}", tc::cli::FormatUtil::formatListWithLineLimit(interupts, 60, 4));
	}
	if (kern.getMiscParams().isSet())
	{
		fmt::print("  ProgramType:        {:s} ({:d})\n", pie::hac::KernelCapabilityUtil::getProgramTypeAsString(kern.getMiscParams().getProgramType()), (uint32_t)kern.getMiscParams().getProgramType());
	}
	if (kern.getKernelVersion().isSet())
	{
		fmt::print("  Kernel Version:     {:d}.{:d}\n", kern.getKernelVersion().getVerMajor(), kern.getKernelVersion().getVerMinor());
	}
	if (kern.getHandleTableSize().isSet())
	{
		fmt::print("  Handle Table Size:  0x{:x}\n", kern.getHandleTableSize().getHandleTableSize());
	}
	if (kern.getMiscFlags().isSet())
	{
		auto misc_flags = kern.getMiscFlags().getMiscFlags();
		fmt::print("  Misc Flags:\n");
		std::vector<std::string> misc_flags_names;
		for (size_t misc_flags_bit = 0; misc_flags_bit < misc_flags.size(); misc_flags_bit++)
		{
			if (misc_flags.test(misc_flags_bit))
				misc_flags_names.push_back(pie::hac::KernelCapabilityUtil::getMiscFlagsBitAsString(pie::hac::kc::MiscFlagsBit(misc_flags_bit)));
		}
		fmt::print("{:s}", tc::cli::FormatUtil::formatListWithLineLimit(misc_flags_names, 60, 4));
	}
}

std::string nstool::KipProcess::formatMappingAsString(const pie::hac::MemoryMappingHandler::sMemoryMapping& map) const
{
	return fmt::format("0x{:016x} - 0x{:016x} (perm={:s}) (type={:s})", ((uint64_t)map.addr << 12), (((uint64_t)(map.addr + map.size) << 12) - 1), pie::hac::KernelCapabilityUtil::getMemoryPermissionAsString(map.perm), pie::hac::KernelCapabilityUtil::getMappingTypeAsString(map.type));
}