#include "KipProcess.h"

#include <iostream>
#include <iomanip>

#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>

#include <nn/hac/KernelCapabilityUtil.h>
#include <nn/hac/KernelInitialProcessUtil.h>


KipProcess::KipProcess():
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void KipProcess::process()
{
	importHeader();
	//importCodeSegments();
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
	{
		displayHeader();
		displayKernelCap(mHdr.getKernelCapabilities());
	}
}

void KipProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void KipProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void KipProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void KipProcess::importHeader()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	if ((*mFile)->size() < sizeof(nn::hac::sKipHeader))
	{
		throw fnd::Exception(kModuleName, "Corrupt KIP: file too small");
	}

	scratch.alloc(sizeof(nn::hac::sKipHeader));
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mHdr.fromBytes(scratch.data(), scratch.size());
}

void KipProcess::importCodeSegments()
{
#ifdef _KIP_COMPRESSION_IMPLEMENTED
	fnd::Vec<byte_t> scratch;
	uint32_t decompressed_len;
#endif

	// process text segment
#ifdef _KIP_COMPRESSION_IMPLEMENTED
	if (mHdr.getTextSegmentInfo().is_compressed)
	{
		scratch.alloc(mHdr.getTextSegmentInfo().file_layout.size);
		(*mFile)->read(scratch.data(), mHdr.getTextSegmentInfo().file_layout.offset, scratch.size());
		mTextBlob.alloc(mHdr.getTextSegmentInfo().memory_layout.size);
		fnd::lz4::decompressData(scratch.data(), (uint32_t)scratch.size(), mTextBlob.data(), (uint32_t)mTextBlob.size(), decompressed_len);
		if (decompressed_len != mTextBlob.size())
		{
			throw fnd::Exception(kModuleName, "KIP text segment failed to decompress");
		}
	}
	else
	{
		mTextBlob.alloc(mHdr.getTextSegmentInfo().file_layout.size);
		(*mFile)->read(mTextBlob.data(), mHdr.getTextSegmentInfo().file_layout.offset, mTextBlob.size());
	}
#else
	mTextBlob.alloc(mHdr.getTextSegmentInfo().file_layout.size);
	(*mFile)->read(mTextBlob.data(), mHdr.getTextSegmentInfo().file_layout.offset, mTextBlob.size());
#endif

	// process ro segment
#ifdef _KIP_COMPRESSION_IMPLEMENTED
	if (mHdr.getRoSegmentInfo().is_compressed)
	{
		scratch.alloc(mHdr.getRoSegmentInfo().file_layout.size);
		(*mFile)->read(scratch.data(), mHdr.getRoSegmentInfo().file_layout.offset, scratch.size());
		mRoBlob.alloc(mHdr.getRoSegmentInfo().memory_layout.size);
		fnd::lz4::decompressData(scratch.data(), (uint32_t)scratch.size(), mRoBlob.data(), (uint32_t)mRoBlob.size(), decompressed_len);
		if (decompressed_len != mRoBlob.size())
		{
			throw fnd::Exception(kModuleName, "KIP ro segment failed to decompress");
		}
	}
	else
	{
		mRoBlob.alloc(mHdr.getRoSegmentInfo().file_layout.size);
		(*mFile)->read(mRoBlob.data(), mHdr.getRoSegmentInfo().file_layout.offset, mRoBlob.size());
	}
#else
	mRoBlob.alloc(mHdr.getRoSegmentInfo().file_layout.size);
	(*mFile)->read(mRoBlob.data(), mHdr.getRoSegmentInfo().file_layout.offset, mRoBlob.size());
#endif

	// process data segment
#ifdef _KIP_COMPRESSION_IMPLEMENTED
	if (mHdr.getDataSegmentInfo().is_compressed)
	{
		scratch.alloc(mHdr.getDataSegmentInfo().file_layout.size);
		(*mFile)->read(scratch.data(), mHdr.getDataSegmentInfo().file_layout.offset, scratch.size());
		mDataBlob.alloc(mHdr.getDataSegmentInfo().memory_layout.size);
		fnd::lz4::decompressData(scratch.data(), (uint32_t)scratch.size(), mDataBlob.data(), (uint32_t)mDataBlob.size(), decompressed_len);
		if (decompressed_len != mDataBlob.size())
		{
			throw fnd::Exception(kModuleName, "KIP data segment failed to decompress");
		}
	}
	else
	{
		mDataBlob.alloc(mHdr.getDataSegmentInfo().file_layout.size);
		(*mFile)->read(mDataBlob.data(), mHdr.getDataSegmentInfo().file_layout.offset, mDataBlob.size());
	}
#else
	mDataBlob.alloc(mHdr.getDataSegmentInfo().file_layout.size);
	(*mFile)->read(mDataBlob.data(), mHdr.getDataSegmentInfo().file_layout.offset, mDataBlob.size());
#endif
}

void KipProcess::displayHeader()
{
	std::cout << "[KIP Header]" << std::endl;
	std::cout << "  Meta:" << std::endl;
	std::cout << "    Name:              " << mHdr.getName() << std::endl;
	std::cout << "    TitleId:           0x" << std::hex << std::setw(16) << std::setfill('0') << mHdr.getTitleId() << std::endl;
	std::cout << "    ProcessCategory:   " << nn::hac::KernelInitialProcessUtil::getProcessCategoryAsString(mHdr.getProcessCategory()) << std::endl;
	std::cout << "    InstructionType:   " << getInstructionTypeStr(mHdr.getFlagList().hasElement(nn::hac::kip::FLAG_INSTRUCTION_64BIT)) << std::endl;
	std::cout << "    AddrSpaceWidth:    " << getAddressSpaceStr(mHdr.getFlagList().hasElement(nn::hac::kip::FLAG_ADDR_SPACE_64BIT)) << std::endl;
	std::cout << "    MemoryPool:        " << getMemoryPoolStr(mHdr.getFlagList().hasElement(nn::hac::kip::FLAG_USE_SYSTEM_POOL_PARTITION)) << std::endl;
	std::cout << "  Program Sections:" << std::endl;
	std::cout << "     .text:" << std::endl;
	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		std::cout << "      FileOffset:     0x" << std::hex << mHdr.getTextSegmentInfo().file_layout.offset << std::endl;
		std::cout << "      FileSize:       0x" << std::hex << mHdr.getTextSegmentInfo().file_layout.size << (mHdr.getTextSegmentInfo().is_compressed? " (COMPRESSED)" : "") << std::endl;
	}
	std::cout << "      MemoryOffset:   0x" << std::hex << mHdr.getTextSegmentInfo().memory_layout.offset << std::endl;
	std::cout << "      MemorySize:     0x" << std::hex << mHdr.getTextSegmentInfo().memory_layout.size << std::endl;
	std::cout << "    .ro:" << std::endl;
	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		std::cout << "      FileOffset:     0x" << std::hex << mHdr.getRoSegmentInfo().file_layout.offset << std::endl;
		std::cout << "      FileSize:       0x" << std::hex << mHdr.getRoSegmentInfo().file_layout.size << (mHdr.getRoSegmentInfo().is_compressed? " (COMPRESSED)" : "") << std::endl;
	}
	std::cout << "      MemoryOffset:   0x" << std::hex << mHdr.getRoSegmentInfo().memory_layout.offset << std::endl;
	std::cout << "      MemorySize:     0x" << std::hex << mHdr.getRoSegmentInfo().memory_layout.size << std::endl;
	std::cout << "    .data:" << std::endl;
	if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
	{
		std::cout << "      FileOffset:     0x" << std::hex << mHdr.getDataSegmentInfo().file_layout.offset << std::endl;
		std::cout << "      FileSize:       0x" << std::hex << mHdr.getDataSegmentInfo().file_layout.size << (mHdr.getDataSegmentInfo().is_compressed? " (COMPRESSED)" : "") << std::endl;
	}
	std::cout << "      MemoryOffset:   0x" << std::hex << mHdr.getDataSegmentInfo().memory_layout.offset << std::endl;
	std::cout << "      MemorySize:     0x" << std::hex << mHdr.getDataSegmentInfo().memory_layout.size << std::endl;
	std::cout << "    .bss:" << std::endl;
	std::cout << "      MemorySize:     0x" << std::hex << mHdr.getBssSize() << std::endl;

}

void KipProcess::displayKernelCap(const nn::hac::KernelCapabilityControl& kern)
{
	std::cout << "[Kernel Capabilities]" << std::endl;
	if (kern.getThreadInfo().isSet())
	{
		nn::hac::ThreadInfoHandler threadInfo = kern.getThreadInfo();
		std::cout << "  Thread Priority:" << std::endl;
		std::cout << "    Min:     " << std::dec << (uint32_t)threadInfo.getMinPriority() << std::endl;
		std::cout << "    Max:     " << std::dec << (uint32_t)threadInfo.getMaxPriority() << std::endl;
		std::cout << "  CpuId:" << std::endl;
		std::cout << "    Min:     " << std::dec << (uint32_t)threadInfo.getMinCpuId() << std::endl;
		std::cout << "    Max:     " << std::dec << (uint32_t)threadInfo.getMaxCpuId() << std::endl;
	}

	if (kern.getSystemCalls().isSet())
	{
		fnd::List<nn::hac::kc::SystemCall> syscalls = kern.getSystemCalls().getSystemCalls();
		std::cout << "  SystemCalls:" << std::endl;
		std::cout << "    ";
		size_t lineLen = 0;
		for (size_t i = 0; i < syscalls.size(); i++)
		{
			if (lineLen > 60)
			{
				lineLen = 0;
				std::cout << std::endl;
				std::cout << "    ";
			}
			std::string syscall_string = nn::hac::KernelCapabilityUtil::getSystemCallAsString(syscalls[i]);
			std::cout << syscall_string;
			if (syscalls[i] != syscalls.atBack())
				std::cout << ", ";
			lineLen += syscall_string.length();
		}
		std::cout << std::endl;
	}
	if (kern.getMemoryMaps().isSet())
	{
		fnd::List<nn::hac::MemoryMappingHandler::sMemoryMapping> maps = kern.getMemoryMaps().getMemoryMaps();
		fnd::List<nn::hac::MemoryMappingHandler::sMemoryMapping> ioMaps = kern.getMemoryMaps().getIoMemoryMaps();

		std::cout << "  MemoryMaps:" << std::endl;
		for (size_t i = 0; i < maps.size(); i++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)maps[i].addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(maps[i].addr + maps[i].size) << 12) - 1) << " (perm=" << nn::hac::KernelCapabilityUtil::getMemMapPermAsString(maps[i].perm) << ") (type=" << nn::hac::KernelCapabilityUtil::getMemMapTypeAsString(maps[i].type) << ")" << std::endl;
		}
		//std::cout << "  IoMaps:" << std::endl;
		for (size_t i = 0; i < ioMaps.size(); i++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)ioMaps[i].addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(ioMaps[i].addr + ioMaps[i].size) << 12) - 1) << " (perm=" << nn::hac::KernelCapabilityUtil::getMemMapPermAsString(ioMaps[i].perm) << ") (type=" << nn::hac::KernelCapabilityUtil::getMemMapTypeAsString(ioMaps[i].type) << ")" << std::endl;
		}
	}
	if (kern.getInterupts().isSet())
	{
		fnd::List<uint16_t> interupts = kern.getInterupts().getInteruptList();
		std::cout << "  Interupts Flags:" << std::endl;
		for (uint32_t i = 0; i < interupts.size(); i++)
		{
			if (i % 10 == 0)
			{
				if (i != 0)
					std::cout << std::endl;
				std::cout << "    ";
			}
			std::cout << "0x" << std::hex << (uint32_t)interupts[i];
			if (interupts[i] != interupts.atBack())
				std::cout << ", ";
		}
		std::cout << std::endl;
	}
	if (kern.getMiscParams().isSet())
	{
		std::cout << "  ProgramType:        " << std::dec << (uint32_t)kern.getMiscParams().getProgramType() << std::endl;
	}
	if (kern.getKernelVersion().isSet())
	{
		std::cout << "  Kernel Version:     " << std::dec << (uint32_t)kern.getKernelVersion().getVerMajor() << "." << (uint32_t)kern.getKernelVersion().getVerMinor() << std::endl;
	}
	if (kern.getHandleTableSize().isSet())
	{
		std::cout << "  Handle Table Size:  0x" << std::hex << kern.getHandleTableSize().getHandleTableSize() << std::endl;
	}
	if (kern.getMiscFlags().isSet())
	{
		fnd::List<nn::hac::kc::MiscFlags> flagList = kern.getMiscFlags().getFlagList();

		std::cout << "  Misc Flags:" << std::endl;
		for (uint32_t i = 0; i < flagList.size(); i++)
		{
			if (i % 10 == 0)
			{
				if (i != 0)
					std::cout << std::endl;
				std::cout << "    ";
			}
			std::cout << nn::hac::KernelCapabilityUtil::getMiscFlagAsString(flagList[i]);
			if (flagList[i] != flagList.atBack())
				std::cout << ", ";
			std::cout << std::endl;
		}
	}
}

const char* KipProcess::getInstructionTypeStr(bool is64Bit) const
{
	return is64Bit? "64Bit" : "32Bit";
}

const char* KipProcess::getAddressSpaceStr(bool is64Bit) const
{
	return is64Bit? "64Bit" : "32Bit";
}

const char* KipProcess::getMemoryPoolStr(bool isSystemPool) const
{
	return isSystemPool? "System" : "Application";
}