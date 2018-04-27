#include "NpdmProcess.h"
#include <fnd/SimpleFile.h>
#include <fnd/MemoryBlob.h>

const std::string kInstructionType[2] = { "32Bit", "64Bit" };
const std::string kProcAddrSpace[4] = { "Unknown", "64Bit", "32Bit", "32Bit no reserved" };
const std::string kAciType[2] = { "ACI0", "ACID" };
const std::string kMiscFlag[15] = { "EnableDebug", "ForceDebug", "bit2", "bit3", "bit4", "bit5", "bit6", "bit7", "bit8", "bit9", "bit10", "bit11", "bit12", "bit13", "bit14"};
const std::string kFsaFlag[64] =
{
	"ApplicationInfo",
	"BootModeControl",
	"Calibration",
	"SystemSaveData",
	"GameCard",
	"SaveDataBackUp",
	"SaveDataManagement",
	"BisAllRaw",
	"GameCardRaw",
	"GameCardPrivate",
	"SetTime",
	"ContentManager",
	"ImageManager",
	"CreateSaveData",
	"SystemSaveDataManagement",
	"BisFileSystem",
	"SystemUpdate",
	"SaveDataMeta",
	"DeviceSaveData",
	"SettingsControl",
	"Bit20",
	"Bit21",
	"Bit22",
	"Bit23",
	"Bit24",
	"Bit25",
	"Bit26",
	"Bit27",
	"Bit28",
	"Bit29",
	"Bit30",
	"Bit31",
	"Bit32",
	"Bit33",
	"Bit34",
	"Bit35",
	"Bit36",
	"Bit37",
	"Bit38",
	"Bit39",
	"Bit40",
	"Bit41",
	"Bit42",
	"Bit43",
	"Bit44",
	"Bit45",
	"Bit46",
	"Bit47",
	"Bit48",
	"Bit49",
	"Bit50",
	"Bit51",
	"Bit52",
	"Bit53",
	"Bit54",
	"Bit55",
	"Bit56",
	"Bit57",
	"Bit58",
	"Bit59",
	"Bit60",
	"Bit61",
	"Debug",
	"FullPermission"
};
const std::string kSysCall[0x80] = 
{
	"svc00",
	"SetHeapSize",
	"SetMemoryPermission",
	"SetMemoryAttribute",
	"MapMemory",
	"UnmapMemory",
	"QueryMemory",
	"ExitProcess",
	"CreateThread",
	"StartThread",
	"ExitThread",
	"SleepThread",
	"GetThreadPriority",
	"SetThreadPriority",
	"GetThreadCoreMask",
	"SetThreadCoreMask",
	"GetCurrentProcessorNumber",
	"SignalEvent",
	"ClearEvent",
	"MapSharedMemory",
	"UnmapSharedMemory",
	"CreateTransferMemory",
	"CloseHandle",
	"ResetSignal",
	"WaitSynchronization",
	"CancelSynchronization",
	"ArbitrateLock",
	"ArbitrateUnlock",
	"WaitProcessWideKeyAtomic",
	"SignalProcessWideKey",
	"GetSystemTick",
	"ConnectToNamedPort",
	"SendSyncRequestLight",
	"SendSyncRequest",
	"SendSyncRequestWithUserBuffer",
	"SendAsyncRequestWithUserBuffer",
	"GetProcessId",
	"GetThreadId",
	"Break",
	"OutputDebugString",
	"ReturnFromException",
	"GetInfo",
	"FlushEntireDataCache",
	"FlushDataCache",
	"MapPhysicalMemory",
	"UnmapPhysicalMemory",
	"svc2E",
	"GetLastThreadInfo",
	"GetResourceLimitLimitValue",
	"GetResourceLimitCurrentValue",
	"SetThreadActivity",
	"GetThreadContext3",
	"svc34",
	"svc35",
	"svc36",
	"svc37",
	"svc38",
	"svc39",
	"svc3A",
	"svc3B",
	"DumpInfo",
	"svc3D",
	"svc3E",
	"svc3F",
	"CreateSession",
	"AcceptSession",
	"ReplyAndReceiveLight",
	"ReplyAndReceive",
	"ReplyAndReceiveWithUserBuffer",
	"CreateEvent",
	"svc46",
	"svc47",
	"svc48",
	"svc49",
	"svc4A",
	"svc4B",
	"svc4C",
	"SleepSystem",
	"ReadWriteRegister",
	"SetProcessActivity",
	"CreateSharedMemory",
	"MapTransferMemory",
	"UnmapTransferMemory",
	"CreateInterruptEvent",
	"QueryPhysicalAddress",
	"QueryIoMapping",
	"CreateDeviceAddressSpace",
	"AttachDeviceAddressSpace",
	"DetachDeviceAddressSpace",
	"MapDeviceAddressSpaceByForce",
	"MapDeviceAddressSpaceAligned",
	"MapDeviceAddressSpace",
	"UnmapDeviceAddressSpace",
	"InvalidateProcessDataCache",
	"StoreProcessDataCache",
	"FlushProcessDataCache",
	"DebugActiveProcess",
	"BreakDebugProcess",
	"TerminateDebugProcess",
	"GetDebugEvent",
	"ContinueDebugEvent",
	"GetProcessList",
	"GetThreadList",
	"GetDebugThreadContext",
	"SetDebugThreadContext",
	"QueryDebugProcessMemory",
	"ReadDebugProcessMemory",
	"WriteDebugProcessMemory",
	"SetHardwareBreakPoint",
	"GetDebugThreadParam",
	"svc6E",
	"svc6F",
	"CreatePort",
	"ManageNamedPort",
	"ConnectToPort",
	"SetProcessMemoryPermission",
	"MapProcessMemory",
	"UnmapProcessMemory",
	"QueryProcessMemory",
	"MapProcessCodeMemory",
	"UnmapProcessCodeMemory",
	"CreateProcess",
	"StartProcess",
	"TerminateProcess",
	"GetProcessInfo",
	"CreateResourceLimit",
	"SetResourceLimitLimitValue",
	"CallSecureMonitor"
};

const std::string kMemMapPerm[2] = { "RW", "RO" };
const std::string kMemMapType[2] = { "Io", "Static" };
 
const std::string kAcidTarget[2] = { "Development", "Production" };

void NpdmProcess::validateAcidSignature(const nx::AcidBinary& acid)
{
	try {
		acid.verifyBinary(mKeyset->acid_sign_key);
	}
	catch (...) {
		// this is minimal even though it's a warning because it's a validation method
		if (mCliOutputType >= OUTPUT_MINIMAL)
			printf("[WARNING] ACID Signature: FAIL\n");
	}
	
}

void NpdmProcess::validateAciFromAcid(const nx::AciBinary& aci, const nx::AcidBinary& acid)
{
	// check Program ID
	if (acid.getProgramIdMin() > 0 && aci.getProgramId() < acid.getProgramIdMin())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
			printf("[WARNING] ACI ProgramId: FAIL (Outside Legal Range)\n");
	}
	else if (acid.getProgramIdMax() > 0 && aci.getProgramId() > acid.getProgramIdMax())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
			printf("[WARNING] ACI ProgramId: FAIL (Outside Legal Range)\n");
	}

	// Check FAC
	if (aci.getFac().getFormatVersion() !=  acid.getFac().getFormatVersion())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
			printf("[WARNING] ACI/FAC FormatVersion: FAIL (%d != %d (expected))\n", aci.getFac().getFormatVersion(),acid.getFac().getFormatVersion());
	}

	for (size_t i = 0; i < aci.getFac().getFsaRightsList().getSize(); i++)
	{
		bool fsaRightFound = false;
		for (size_t j = 0; j < acid.getFac().getFsaRightsList().getSize() && fsaRightFound == false; j++)
		{
			if (aci.getFac().getFsaRightsList()[i] == acid.getFac().getFsaRightsList()[j])
				fsaRightFound = true;
		}

		if (fsaRightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/FAC FsaRights: FAIL (%s not permitted)\n", kFsaFlag[aci.getFac().getFsaRightsList()[i]].c_str());
		}
	}

	for (size_t i = 0; i < aci.getFac().getContentOwnerIdList().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getFac().getContentOwnerIdList().getSize() && rightFound == false; j++)
		{
			if (aci.getFac().getContentOwnerIdList()[i] == acid.getFac().getContentOwnerIdList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/FAC ContentOwnerId: FAIL (%08x not permitted)\n", aci.getFac().getContentOwnerIdList()[i]);
		}
	}

	for (size_t i = 0; i < aci.getFac().getSaveDataOwnerIdList().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getFac().getSaveDataOwnerIdList().getSize() && rightFound == false; j++)
		{
			if (aci.getFac().getSaveDataOwnerIdList()[i] == acid.getFac().getSaveDataOwnerIdList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/FAC ContentOwnerId: FAIL (%08x not permitted)\n", aci.getFac().getSaveDataOwnerIdList()[i]);
		}
	}

	// check SAC
	for (size_t i = 0; i < aci.getSac().getServiceList().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getSac().getServiceList().getSize() && rightFound == false; j++)
		{
			if (aci.getSac().getServiceList()[i] == acid.getSac().getServiceList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/SAC ServiceList: FAIL (%s%s not permitted)\n", aci.getSac().getServiceList()[i].getName().c_str(), aci.getSac().getServiceList()[i].isServer()? " (Server)" : "");
		}
	}

	// check KC
	// check thread info
	if (aci.getKc().getThreadInfo().getMaxCpuId() != acid.getKc().getThreadInfo().getMaxCpuId())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC ThreadInfo/MaxCpuId: FAIL (%d not permitted)\n", aci.getKc().getThreadInfo().getMaxCpuId());
	}
	if (aci.getKc().getThreadInfo().getMinCpuId() != acid.getKc().getThreadInfo().getMinCpuId())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC ThreadInfo/MinCpuId: FAIL (%d not permitted)\n", aci.getKc().getThreadInfo().getMinCpuId());
	}
	if (aci.getKc().getThreadInfo().getMaxPriority() != acid.getKc().getThreadInfo().getMaxPriority())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC ThreadInfo/MaxPriority: FAIL (%d not permitted)\n", aci.getKc().getThreadInfo().getMaxPriority());
	}
	if (aci.getKc().getThreadInfo().getMinPriority() != acid.getKc().getThreadInfo().getMinPriority())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC ThreadInfo/MinPriority: FAIL (%d not permitted)\n", aci.getKc().getThreadInfo().getMinPriority());
	}
	// check system calls
	for (size_t i = 0; i < aci.getKc().getSystemCalls().getSystemCalls().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKc().getSystemCalls().getSystemCalls().getSize() && rightFound == false; j++)
		{
			if (aci.getKc().getSystemCalls().getSystemCalls()[i] == acid.getKc().getSystemCalls().getSystemCalls()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC SystemCallList: FAIL (%s not permitted)\n", kSysCall[aci.getKc().getSystemCalls().getSystemCalls()[i]].c_str());
		}
	}
	// check memory maps
	for (size_t i = 0; i < aci.getKc().getMemoryMaps().getMemoryMaps().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKc().getMemoryMaps().getMemoryMaps().getSize() && rightFound == false; j++)
		{
			if (aci.getKc().getMemoryMaps().getMemoryMaps()[i] == acid.getKc().getMemoryMaps().getMemoryMaps()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			const nx::MemoryMappingHandler::sMemoryMapping& map = aci.getKc().getMemoryMaps().getMemoryMaps()[i];
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC MemoryMap: FAIL (0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s) not permitted)\n", (uint64_t)map.addr << 12, ((uint64_t)(map.addr + map.size) << 12) - 1, kMemMapPerm[map.perm].c_str(), kMemMapType[map.type].c_str());
		}
	}
	for (size_t i = 0; i < aci.getKc().getMemoryMaps().getIoMemoryMaps().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKc().getMemoryMaps().getIoMemoryMaps().getSize() && rightFound == false; j++)
		{
			if (aci.getKc().getMemoryMaps().getIoMemoryMaps()[i] == acid.getKc().getMemoryMaps().getIoMemoryMaps()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			const nx::MemoryMappingHandler::sMemoryMapping& map = aci.getKc().getMemoryMaps().getIoMemoryMaps()[i];
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC IoMemoryMap: FAIL (0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s) not permitted)\n", (uint64_t)map.addr << 12, ((uint64_t)(map.addr + map.size) << 12) - 1, kMemMapPerm[map.perm].c_str(), kMemMapType[map.type].c_str());
		}
	}
	// check interupts
	for (size_t i = 0; i < aci.getKc().getInterupts().getInteruptList().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKc().getInterupts().getInteruptList().getSize() && rightFound == false; j++)
		{
			if (aci.getKc().getInterupts().getInteruptList()[i] == acid.getKc().getInterupts().getInteruptList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC InteruptsList: FAIL (0x%0x not permitted)\n", aci.getKc().getInterupts().getInteruptList()[i]);
		}
	}
	// check misc params
	if (aci.getKc().getMiscParams().getProgramType() != acid.getKc().getMiscParams().getProgramType())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC ProgramType: FAIL (%d not permitted)\n",  aci.getKc().getMiscParams().getProgramType());
	}
	// check kernel version
	uint32_t aciKernelVersion = (uint32_t)aci.getKc().getKernelVersion().getVerMajor() << 16 |  (uint32_t)aci.getKc().getKernelVersion().getVerMinor();
	uint32_t acidKernelVersion =  (uint32_t)acid.getKc().getKernelVersion().getVerMajor() << 16 |  (uint32_t)acid.getKc().getKernelVersion().getVerMinor();
	if (aciKernelVersion < acidKernelVersion)
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC RequiredKernelVersion: FAIL (%d.%d not permitted)\n", aci.getKc().getKernelVersion().getVerMajor(), aci.getKc().getKernelVersion().getVerMinor());
	}
	// check handle table size
	if (aci.getKc().getHandleTableSize().getHandleTableSize() > acid.getKc().getHandleTableSize().getHandleTableSize())
	{
		if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC HandleTableSize: FAIL (0x%x too large)\n", aci.getKc().getHandleTableSize().getHandleTableSize());
	}
	// check misc flags
	for (size_t i = 0; i < aci.getKc().getMiscFlags().getFlagList().getSize(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKc().getMiscFlags().getFlagList().getSize() && rightFound == false; j++)
		{
			if (aci.getKc().getMiscFlags().getFlagList()[i] == acid.getKc().getMiscFlags().getFlagList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] ACI/KC MiscFlag: FAIL (%s not permitted)\n", kMiscFlag[aci.getKc().getMiscFlags().getFlagList()[i]].c_str());
		}
	}
}

void NpdmProcess::displayNpdmHeader(const nx::NpdmHeader& hdr)
{
	printf("[NPDM HEADER]\n");
	printf("  Process Architecture Params:\n");
	printf("    Ins. Type:     %s\n", kInstructionType[hdr.getInstructionType()].c_str());
	printf("    Addr Space:    %s\n", kProcAddrSpace[hdr.getProcAddressSpaceType()].c_str());
	printf("  Main Thread Params:\n");
	printf("    Priority:      %d\n", hdr.getMainThreadPriority());
	printf("    CpuId:         %d\n", hdr.getMainThreadCpuId());
	printf("    StackSize:     0x%x\n", hdr.getMainThreadStackSize());
	printf("  TitleInfo:\n");
	printf("    Version:       v%" PRIu32 "\n", hdr.getVersion());
	printf("    Name:          %s\n", hdr.getName().c_str());
	if (hdr.getProductCode().length())
	{
		printf("    ProductCode:   %s\n", hdr.getProductCode().c_str());
	}
}

void NpdmProcess::displayAciHdr(const nx::AciHeader& aci)
{
	printf("[Access Control Info]\n");
	printf("  ACI Type:        %s\n", kAciType[aci.getAciType()].c_str());
	if (aci.getAciType() == nx::AciBinary::TYPE_ACI0)
	{
		printf("  ProgramID:       %016" PRIx64 "\n", aci.getProgramId());
	}
	else if (aci.getAciType() == nx::AciBinary::TYPE_ACID)
	{

		printf("  ACID Size:       %" PRIx64 "\n", aci.getAcidSize());
		printf("  Flags:           \n");
		printf("    Production:          %s\n", aci.isProduction() ? "TRUE" : "FALSE");
		printf("    UnqualifiedApproval: %s\n", aci.isUnqualifiedApproval() ? "TRUE" : "FALSE");
		printf("  ProgramID Restriction\n");
		printf("    Min:           %016" PRIx64 "\n", aci.getProgramIdMin());
		printf("    Max:           %016" PRIx64 "\n", aci.getProgramIdMax());
		
	}
}

void NpdmProcess::displayFac(const nx::FacBinary& fac)
{
	printf("[FS Access Control]\n");
	printf("  Format Version:   %d\n", fac.getFormatVersion());

	if (fac.getFsaRightsList().getSize())
	{
		printf("  FS Rights:\n");
		for (size_t i = 0; i < fac.getFsaRightsList().getSize(); i++)
		{
			if (i % 10 == 0)
			{
				printf("%s    ", i != 0 ? "\n" : "");
			}
			printf("%s%s", kFsaFlag[fac.getFsaRightsList()[i]].c_str(), fac.getFsaRightsList()[i] != fac.getFsaRightsList().atBack() ? ", " : "\n");
		}
	}
	else
	{
		printf("  FS Rights: NONE\n");
	}
	
	if (fac.getContentOwnerIdList().getSize())
	{
		printf("  Content Owner IDs:\n");
		for (size_t i = 0; i < fac.getContentOwnerIdList().getSize(); i++)
		{
			printf("    0x%08x\n", fac.getContentOwnerIdList()[i]);
		}
	}
	if (fac.getSaveDataOwnerIdList().getSize())
	{
		printf("  Save Data Owner IDs:\n");
		for (size_t i = 0; i < fac.getSaveDataOwnerIdList().getSize(); i++)
		{
			printf("    0x%08x\n", fac.getSaveDataOwnerIdList()[i]);
		}
	}
	
}

void NpdmProcess::displaySac(const nx::SacBinary& sac)
{
	printf("[Service Access Control]\n");
	printf("  Service List:\n");
	for (size_t i = 0; i < sac.getServiceList().getSize(); i++)
	{
		if (i % 10 == 0)
		{
			printf("%s    ", i != 0 ? "\n" : "");
		}
		printf("%s%s%s", sac.getServiceList()[i].getName().c_str(), sac.getServiceList()[i].isServer() ? "(isSrv)" : "", sac.getServiceList()[i] != sac.getServiceList().atBack() ? ", " : "\n");
	}
}

void NpdmProcess::displayKernelCap(const nx::KcBinary& kern)
{
	printf("[Kernel Capabilities]\n");
	if (kern.getThreadInfo().isSet())
	{
		nx::ThreadInfoHandler threadInfo = kern.getThreadInfo();
		printf("  Thread Priority:\n");
		printf("    Min:     %d\n", threadInfo.getMinPriority());
		printf("    Max:     %d\n", threadInfo.getMaxPriority());
		printf("  CpuId:\n");
		printf("    Min:     %d\n", threadInfo.getMinCpuId());
		printf("    Max:     %d\n", threadInfo.getMaxCpuId());
	}
	if (kern.getSystemCalls().isSet())
	{
		fnd::List<uint8_t> syscalls = kern.getSystemCalls().getSystemCalls();
		printf("  SystemCalls:");
		printf("\n    ");
		size_t lineLen = 0;
		for (size_t i = 0; i < syscalls.getSize(); i++)
		{
			if (lineLen > 60)
			{
				lineLen = 0;
				printf("\n    ");
			}
			printf("%s%s", kSysCall[syscalls[i]].c_str(), syscalls[i] != syscalls.atBack() ? ", " : "\n");
			lineLen += kSysCall[syscalls[i]].length();
		}
	}
	if (kern.getMemoryMaps().isSet())
	{
		fnd::List<nx::MemoryMappingHandler::sMemoryMapping> maps = kern.getMemoryMaps().getMemoryMaps();
		fnd::List<nx::MemoryMappingHandler::sMemoryMapping> ioMaps = kern.getMemoryMaps().getIoMemoryMaps();

		printf("  MemoryMaps:\n");
		for (size_t i = 0; i < maps.getSize(); i++)
		{
			printf("    0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s)\n", (uint64_t)maps[i].addr << 12, ((uint64_t)(maps[i].addr + maps[i].size) << 12) - 1, kMemMapPerm[maps[i].perm].c_str(), kMemMapType[maps[i].type].c_str());
		}
		//printf("  IoMaps:\n");
		for (size_t i = 0; i < ioMaps.getSize(); i++)
		{
			printf("    0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s)\n", (uint64_t)ioMaps[i].addr << 12, ((uint64_t)(ioMaps[i].addr + ioMaps[i].size) << 12) - 1, kMemMapPerm[ioMaps[i].perm].c_str(), kMemMapType[ioMaps[i].type].c_str());
		}
	}
	if (kern.getInterupts().isSet())
	{
		fnd::List<uint16_t> interupts = kern.getInterupts().getInteruptList();
		printf("  Interupts Flags:\n");
		for (uint32_t i = 0; i < interupts.getSize(); i++)
		{
			if (i % 10 == 0)
			{
				printf("%s    ", i != 0 ? "\n" : "");
			}
			printf("0x%x%s", interupts[i], interupts[i] != interupts.atBack() ? ", " : "\n");
		}
	}
	if (kern.getMiscParams().isSet())
	{
		printf("  ProgramType:        %d\n", kern.getMiscParams().getProgramType());
	}
	if (kern.getKernelVersion().isSet())
	{
		printf("  Kernel Version:     %d.%d\n", kern.getKernelVersion().getVerMajor(), kern.getKernelVersion().getVerMinor());
	}
	if (kern.getHandleTableSize().isSet())
	{
		printf("  Handle Table Size:  0x%x\n", kern.getHandleTableSize().getHandleTableSize());
	}
	if (kern.getMiscFlags().isSet())
	{
		fnd::List<nx::MiscFlagsHandler::Flags> flagList = kern.getMiscFlags().getFlagList();

		printf("  Misc Flags:\n");
		for (uint32_t i = 0; i < flagList.getSize(); i++)
		{
			if (i % 10 == 0)
			{
				printf("%s    ", i != 0 ? "\n" : "");
			}
			printf("%s%s", kMiscFlag[flagList[i]].c_str(), flagList[i] != flagList.atBack() ? ", " : "\n");
		}
	}
}

NpdmProcess::NpdmProcess() :
	mReader(nullptr),
	mOffset(0),
	mKeyset(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false)
{
}

void NpdmProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mReader->size());
	mReader->read(scratch.getBytes(), 0, scratch.getSize());

	mNpdm.importBinary(scratch.getBytes(), scratch.getSize());

	if (mVerify)
	{
		validateAcidSignature(mNpdm.getAcid());
		validateAciFromAcid(mNpdm.getAci(), mNpdm.getAcid());
	}

	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		// npdm binary
		displayNpdmHeader(mNpdm);

		// aci binary
		displayAciHdr(mNpdm.getAci());
		displayFac(mNpdm.getAci().getFac());
		displaySac(mNpdm.getAci().getSac());
		displayKernelCap(mNpdm.getAci().getKc());

		// acid binary
		displayAciHdr(mNpdm.getAcid());
		displayFac(mNpdm.getAcid().getFac());
		displaySac(mNpdm.getAcid().getSac());
		displayKernelCap(mNpdm.getAcid().getKc());
	}
}

void NpdmProcess::setInputFile(fnd::IFile& reader)
{
	mReader = &reader;
}

void NpdmProcess::setInputFileOffset(size_t offset)
{
	mOffset = offset;
}

void NpdmProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void NpdmProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void NpdmProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}