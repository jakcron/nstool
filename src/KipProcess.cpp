#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <fnd/Vec.h>
#include "KipProcess.h"

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
	std::cout << "    ProcessCategory:   " << getProcessCategoryStr(mHdr.getProcessCategory()) << std::endl;
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
		fnd::List<uint8_t> syscalls = kern.getSystemCalls().getSystemCalls();
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
			std::cout << getSystemCallStr(syscalls[i]);
			if (syscalls[i] != syscalls.atBack())
				std::cout << ", ";
			lineLen += strlen(getSystemCallStr(syscalls[i]));
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
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)maps[i].addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(maps[i].addr + maps[i].size) << 12) - 1) << " (perm=" << getMemMapPermStr(maps[i].perm) << ") (type=" << getMemMapTypeStr(maps[i].type) << ")" << std::endl;
		}
		//std::cout << "  IoMaps:" << std::endl;
		for (size_t i = 0; i < ioMaps.size(); i++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)ioMaps[i].addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(ioMaps[i].addr + ioMaps[i].size) << 12) - 1) << " (perm=" << getMemMapPermStr(ioMaps[i].perm) << ") (type=" << getMemMapTypeStr(ioMaps[i].type) << ")" << std::endl;
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
		fnd::List<nn::hac::MiscFlagsHandler::Flags> flagList = kern.getMiscFlags().getFlagList();

		std::cout << "  Misc Flags:" << std::endl;
		for (uint32_t i = 0; i < flagList.size(); i++)
		{
			if (i % 10 == 0)
			{
				if (i != 0)
					std::cout << std::endl;
				std::cout << "    ";
			}
			std::cout << getMiscFlagStr(flagList[i]);
			if (flagList[i] != flagList.atBack())
				std::cout << ", ";
			std::cout << std::endl;
		}
	}
}

const char* KipProcess::getProcessCategoryStr(nn::hac::kip::ProcessCategory var) const
{
	const char* str = nullptr;

	switch(var)
	{
	case (nn::hac::kip::PROCCAT_REGULAR):
		str = "NormalProcess";
		break;
	case (nn::hac::kip::PROCCAT_KERNAL_KIP):
		str = "KernelInitalProcess";
		break;
	default:
		str = "Unknown";
	}

	return str;
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

const char* KipProcess::getMiscFlagStr(nn::hac::MiscFlagsHandler::Flags flag) const
{
	const char* str = nullptr;

	switch(flag)
	{
	case (nn::hac::MiscFlagsHandler::FLAG_ENABLE_DEBUG):
		str = "EnableDebug";
		break;
	case (nn::hac::MiscFlagsHandler::FLAG_FORCE_DEBUG):
		str = "ForceDebug";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* KipProcess::getSystemCallStr(byte_t syscall_id) const
{
	const char* str = nullptr;

	switch(syscall_id)
	{
	case (0x01):
		str = "SetHeapSize";
		break;
	case (0x02):
		str = "SetMemoryPermission";
		break;
	case (0x03):
		str = "SetMemoryAttribute";
		break;
	case (0x04):
		str = "MapMemory";
		break;
	case (0x05):
		str = "UnmapMemory";
		break;
	case (0x06):
		str = "QueryMemory";
		break;
	case (0x07):
		str = "ExitProcess";
		break;
	case (0x08):
		str = "CreateThread";
		break;
	case (0x09):
		str = "StartThread";
		break;
	case (0x0a):
		str = "ExitThread";
		break;
	case (0x0b):
		str = "SleepThread";
		break;
	case (0x0c):
		str = "GetThreadPriority";
		break;
	case (0x0d):
		str = "SetThreadPriority";
		break;
	case (0x0e):
		str = "GetThreadCoreMask";
		break;
	case (0x0f):
		str = "SetThreadCoreMask";
		break;
	case (0x10):
		str = "GetCurrentProcessorNumber";
		break;
	case (0x11):
		str = "SignalEvent";
		break;
	case (0x12):
		str = "ClearEvent";
		break;
	case (0x13):
		str = "MapSharedMemory";
		break;
	case (0x14):
		str = "UnmapSharedMemory";
		break;
	case (0x15):
		str = "CreateTransferMemory";
		break;
	case (0x16):
		str = "CloseHandle";
		break;
	case (0x17):
		str = "ResetSignal";
		break;
	case (0x18):
		str = "WaitSynchronization";
		break;
	case (0x19):
		str = "CancelSynchronization";
		break;
	case (0x1a):
		str = "ArbitrateLock";
		break;
	case (0x1b):
		str = "ArbitrateUnlock";
		break;
	case (0x1c):
		str = "WaitProcessWideKeyAtomic";
		break;
	case (0x1d):
		str = "SignalProcessWideKey";
		break;
	case (0x1e):
		str = "GetSystemTick";
		break;
	case (0x1f):
		str = "ConnectToNamedPort";
		break;
	case (0x20):
		str = "SendSyncRequestLight";
		break;
	case (0x21):
		str = "SendSyncRequest";
		break;
	case (0x22):
		str = "SendSyncRequestWithUserBuffer";
		break;
	case (0x23):
		str = "SendAsyncRequestWithUserBuffer";
		break;
	case (0x24):
		str = "GetProcessId";
		break;
	case (0x25):
		str = "GetThreadId";
		break;
	case (0x26):
		str = "Break";
		break;
	case (0x27):
		str = "OutputDebugString";
		break;
	case (0x28):
		str = "ReturnFromException";
		break;
	case (0x29):
		str = "GetInfo";
		break;
	case (0x2a):
		str = "FlushEntireDataCache";
		break;
	case (0x2b):
		str = "FlushDataCache";
		break;
	case (0x2c):
		str = "MapPhysicalMemory";
		break;
	case (0x2d):
		str = "UnmapPhysicalMemory";
		break;
	case (0x2e):
		str = "GetFutureThreadInfo";
		break;
	case (0x2f):
		str = "GetLastThreadInfo";
		break;
	case (0x30):
		str = "GetResourceLimitLimitValue";
		break;
	case (0x31):
		str = "GetResourceLimitCurrentValue";
		break;
	case (0x32):
		str = "SetThreadActivity";
		break;
	case (0x33):
		str = "GetThreadContext3";
		break;
	case (0x34):
		str = "WaitForAddress";
		break;
	case (0x35):
		str = "SignalToAddress";
		break;
	case (0x36):
		str = "svc36";
		break;
	case (0x37):
		str = "svc37";
		break;
	case (0x38):
		str = "svc38";
		break;
	case (0x39):
		str = "svc39";
		break;
	case (0x3a):
		str = "svc3A";
		break;
	case (0x3b):
		str = "svc3B";
		break;
	case (0x3c):
		str = "DumpInfo";
		break;
	case (0x3d):
		str = "DumpInfoNew";
		break;
	case (0x3e):
		str = "svc3E";
		break;
	case (0x3f):
		str = "svc3F";
		break;
	case (0x40):
		str = "CreateSession";
		break;
	case (0x41):
		str = "AcceptSession";
		break;
	case (0x42):
		str = "ReplyAndReceiveLight";
		break;
	case (0x43):
		str = "ReplyAndReceive";
		break;
	case (0x44):
		str = "ReplyAndReceiveWithUserBuffer";
		break;
	case (0x45):
		str = "CreateEvent";
		break;
	case (0x46):
		str = "svc46";
		break;
	case (0x47):
		str = "svc47";
		break;
	case (0x48):
		str = "MapPhysicalMemoryUnsafe";
		break;
	case (0x49):
		str = "UnmapPhysicalMemoryUnsafe";
		break;
	case (0x4a):
		str = "SetUnsafeLimit";
		break;
	case (0x4b):
		str = "CreateCodeMemory";
		break;
	case (0x4c):
		str = "ControlCodeMemory";
		break;
	case (0x4d):
		str = "SleepSystem";
		break;
	case (0x4e):
		str = "ReadWriteRegister";
		break;
	case (0x4f):
		str = "SetProcessActivity";
		break;
	case (0x50):
		str = "CreateSharedMemory";
		break;
	case (0x51):
		str = "MapTransferMemory";
		break;
	case (0x52):
		str = "UnmapTransferMemory";
		break;
	case (0x53):
		str = "CreateInterruptEvent";
		break;
	case (0x54):
		str = "QueryPhysicalAddress";
		break;
	case (0x55):
		str = "QueryIoMapping";
		break;
	case (0x56):
		str = "CreateDeviceAddressSpace";
		break;
	case (0x57):
		str = "AttachDeviceAddressSpace";
		break;
	case (0x58):
		str = "DetachDeviceAddressSpace";
		break;
	case (0x59):
		str = "MapDeviceAddressSpaceByForce";
		break;
	case (0x5a):
		str = "MapDeviceAddressSpaceAligned";
		break;
	case (0x5b):
		str = "MapDeviceAddressSpace";
		break;
	case (0x5c):
		str = "UnmapDeviceAddressSpace";
		break;
	case (0x5d):
		str = "InvalidateProcessDataCache";
		break;
	case (0x5e):
		str = "StoreProcessDataCache";
		break;
	case (0x5f):
		str = "FlushProcessDataCache";
		break;
	case (0x60):
		str = "DebugActiveProcess";
		break;
	case (0x61):
		str = "BreakDebugProcess";
		break;
	case (0x62):
		str = "TerminateDebugProcess";
		break;
	case (0x63):
		str = "GetDebugEvent";
		break;
	case (0x64):
		str = "ContinueDebugEvent";
		break;
	case (0x65):
		str = "GetProcessList";
		break;
	case (0x66):
		str = "GetThreadList";
		break;
	case (0x67):
		str = "GetDebugThreadContext";
		break;
	case (0x68):
		str = "SetDebugThreadContext";
		break;
	case (0x69):
		str = "QueryDebugProcessMemory";
		break;
	case (0x6a):
		str = "ReadDebugProcessMemory";
		break;
	case (0x6b):
		str = "WriteDebugProcessMemory";
		break;
	case (0x6c):
		str = "SetHardwareBreakPoint";
		break;
	case (0x6d):
		str = "GetDebugThreadParam";
		break;
	case (0x6e):
		str = "svc6E";
		break;
	case (0x6f):
		str = "GetSystemInfo";
		break;
	case (0x70):
		str = "CreatePort";
		break;
	case (0x71):
		str = "ManageNamedPort";
		break;
	case (0x72):
		str = "ConnectToPort";
		break;
	case (0x73):
		str = "SetProcessMemoryPermission";
		break;
	case (0x74):
		str = "MapProcessMemory";
		break;
	case (0x75):
		str = "UnmapProcessMemory";
		break;
	case (0x76):
		str = "QueryProcessMemory";
		break;
	case (0x77):
		str = "MapProcessCodeMemory";
		break;
	case (0x78):
		str = "UnmapProcessCodeMemory";
		break;
	case (0x79):
		str = "CreateProcess";
		break;
	case (0x7a):
		str = "StartProcess";
		break;
	case (0x7b):
		str = "TerminateProcess";
		break;
	case (0x7c):
		str = "GetProcessInfo";
		break;
	case (0x7d):
		str = "CreateResourceLimit";
		break;
	case (0x7e):
		str = "SetResourceLimitLimitValue";
		break;
	case (0x7f):
		str = "CallSecureMonitor";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* KipProcess::getMemMapPermStr(nn::hac::MemoryMappingHandler::MemoryPerm type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::MemoryMappingHandler::MEM_RW):
		str = "RW";
		break;
	case (nn::hac::MemoryMappingHandler::MEM_RO):
		str = "RO";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* KipProcess::getMemMapTypeStr(nn::hac::MemoryMappingHandler::MappingType type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::MemoryMappingHandler::MAP_IO):
		str = "Io";
		break;
	case (nn::hac::MemoryMappingHandler::MAP_STATIC):
		str = "Static";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}