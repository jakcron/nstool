#include "NpdmProcess.h"

NpdmProcess::NpdmProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mKeyset(nullptr),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

NpdmProcess::~NpdmProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void NpdmProcess::process()
{
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	mNpdm.fromBytes(scratch.data(), scratch.size());

	if (mVerify)
	{
		validateAcidSignature(mNpdm.getAcid());
		validateAciFromAcid(mNpdm.getAci(), mNpdm.getAcid());
	}

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
	{
		// npdm binary
		displayNpdmHeader(mNpdm);

		// aci binary
		displayAciHdr(mNpdm.getAci());
		displayFac(mNpdm.getAci().getFileSystemAccessControl());
		displaySac(mNpdm.getAci().getServiceAccessControl());
		displayKernelCap(mNpdm.getAci().getKernelCapabilities());

		// acid binary
		if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		{
			displayAciDescHdr(mNpdm.getAcid());
			displayFac(mNpdm.getAcid().getFileSystemAccessControl());
			displaySac(mNpdm.getAcid().getServiceAccessControl());
			displayKernelCap(mNpdm.getAcid().getKernelCapabilities());
		}
	}
}

void NpdmProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void NpdmProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void NpdmProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void NpdmProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nn::hac::NpdmBinary& NpdmProcess::getNpdmBinary() const
{
	return mNpdm;
}

const std::string kInstructionType[2] = { "32Bit", "64Bit" };
const std::string kProcAddrSpace[4] = { "Unknown", "64Bit", "32Bit", "32Bit no reserved" };
const std::string kAcidFlag[32] = 
{
	"Production",
	"UnqualifiedApproval",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown"
};
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

const std::string kSaveDataOwnerAccessMode[4] =
{
	"IllegalAccessCondition",
	"Read",
	"Write",
	"ReadWrite"
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

void NpdmProcess::validateAcidSignature(const nn::hac::AccessControlInfoDescBinary& acid)
{
	try {
		acid.validateSignature(mKeyset->acid_sign_key);
	}
	catch (...) {
		printf("[WARNING] ACID Signature: FAIL\n");
	}
	
}

void NpdmProcess::validateAciFromAcid(const nn::hac::AccessControlInfoBinary& aci, const nn::hac::AccessControlInfoDescBinary& acid)
{
	// check Program ID
	if (acid.getProgramIdRestrict().min > 0 && aci.getProgramId() < acid.getProgramIdRestrict().min)
	{
		printf("[WARNING] ACI ProgramId: FAIL (Outside Legal Range)\n");
	}
	else if (acid.getProgramIdRestrict().max > 0 && aci.getProgramId() > acid.getProgramIdRestrict().max)
	{
		printf("[WARNING] ACI ProgramId: FAIL (Outside Legal Range)\n");
	}

	for (size_t i = 0; i < aci.getFileSystemAccessControl().getFsaRightsList().size(); i++)
	{
		bool fsaRightFound = false;
		for (size_t j = 0; j < acid.getFileSystemAccessControl().getFsaRightsList().size() && fsaRightFound == false; j++)
		{
			if (aci.getFileSystemAccessControl().getFsaRightsList()[i] == acid.getFileSystemAccessControl().getFsaRightsList()[j])
				fsaRightFound = true;
		}

		if (fsaRightFound == false)
		{

			printf("[WARNING] ACI/FAC FsaRights: FAIL (%s not permitted)\n", kFsaFlag[aci.getFileSystemAccessControl().getFsaRightsList()[i]].c_str());
		}
	}

	for (size_t i = 0; i < aci.getFileSystemAccessControl().getContentOwnerIdList().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getFileSystemAccessControl().getContentOwnerIdList().size() && rightFound == false; j++)
		{
			if (aci.getFileSystemAccessControl().getContentOwnerIdList()[i] == acid.getFileSystemAccessControl().getContentOwnerIdList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{

			printf("[WARNING] ACI/FAC ContentOwnerId: FAIL (%016" PRIx64 " not permitted)\n", aci.getFileSystemAccessControl().getContentOwnerIdList()[i]);
		}
	}

	for (size_t i = 0; i < aci.getFileSystemAccessControl().getSaveDataOwnerIdList().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getFileSystemAccessControl().getSaveDataOwnerIdList().size() && rightFound == false; j++)
		{
			if (aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i] == acid.getFileSystemAccessControl().getSaveDataOwnerIdList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{

			printf("[WARNING] ACI/FAC ContentOwnerId: FAIL (%016" PRIx64 "(%d) not permitted)\n", aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i].id, aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i].access_type);
		}
	}

	// check SAC
	for (size_t i = 0; i < aci.getServiceAccessControl().getServiceList().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getServiceAccessControl().getServiceList().size() && rightFound == false; j++)
		{
			if (aci.getServiceAccessControl().getServiceList()[i] == acid.getServiceAccessControl().getServiceList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			printf("[WARNING] ACI/SAC ServiceList: FAIL (%s%s not permitted)\n", aci.getServiceAccessControl().getServiceList()[i].getName().c_str(), aci.getServiceAccessControl().getServiceList()[i].isServer()? " (Server)" : "");
		}
	}

	// check KC
	// check thread info
	if (aci.getKernelCapabilities().getThreadInfo().getMaxCpuId() != acid.getKernelCapabilities().getThreadInfo().getMaxCpuId())
	{
		printf("[WARNING] ACI/KC ThreadInfo/MaxCpuId: FAIL (%d not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMaxCpuId());
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMinCpuId() != acid.getKernelCapabilities().getThreadInfo().getMinCpuId())
	{
		printf("[WARNING] ACI/KC ThreadInfo/MinCpuId: FAIL (%d not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMinCpuId());
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMaxPriority() != acid.getKernelCapabilities().getThreadInfo().getMaxPriority())
	{
		printf("[WARNING] ACI/KC ThreadInfo/MaxPriority: FAIL (%d not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMaxPriority());
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMinPriority() != acid.getKernelCapabilities().getThreadInfo().getMinPriority())
	{
		printf("[WARNING] ACI/KC ThreadInfo/MinPriority: FAIL (%d not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMinPriority());
	}
	// check system calls
	for (size_t i = 0; i < aci.getKernelCapabilities().getSystemCalls().getSystemCalls().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKernelCapabilities().getSystemCalls().getSystemCalls().size() && rightFound == false; j++)
		{
			if (aci.getKernelCapabilities().getSystemCalls().getSystemCalls()[i] == acid.getKernelCapabilities().getSystemCalls().getSystemCalls()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			printf("[WARNING] ACI/KC SystemCallList: FAIL (%s not permitted)\n", kSysCall[aci.getKernelCapabilities().getSystemCalls().getSystemCalls()[i]].c_str());
		}
	}
	// check memory maps
	for (size_t i = 0; i < aci.getKernelCapabilities().getMemoryMaps().getMemoryMaps().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKernelCapabilities().getMemoryMaps().getMemoryMaps().size() && rightFound == false; j++)
		{
			if (aci.getKernelCapabilities().getMemoryMaps().getMemoryMaps()[i] == acid.getKernelCapabilities().getMemoryMaps().getMemoryMaps()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			const nn::hac::MemoryMappingHandler::sMemoryMapping& map = aci.getKernelCapabilities().getMemoryMaps().getMemoryMaps()[i];

			printf("[WARNING] ACI/KC MemoryMap: FAIL (0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s) not permitted)\n", (uint64_t)map.addr << 12, ((uint64_t)(map.addr + map.size) << 12) - 1, kMemMapPerm[map.perm].c_str(), kMemMapType[map.type].c_str());
		}
	}
	for (size_t i = 0; i < aci.getKernelCapabilities().getMemoryMaps().getIoMemoryMaps().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKernelCapabilities().getMemoryMaps().getIoMemoryMaps().size() && rightFound == false; j++)
		{
			if (aci.getKernelCapabilities().getMemoryMaps().getIoMemoryMaps()[i] == acid.getKernelCapabilities().getMemoryMaps().getIoMemoryMaps()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			const nn::hac::MemoryMappingHandler::sMemoryMapping& map = aci.getKernelCapabilities().getMemoryMaps().getIoMemoryMaps()[i];

			printf("[WARNING] ACI/KC IoMemoryMap: FAIL (0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s) not permitted)\n", (uint64_t)map.addr << 12, ((uint64_t)(map.addr + map.size) << 12) - 1, kMemMapPerm[map.perm].c_str(), kMemMapType[map.type].c_str());
		}
	}
	// check interupts
	for (size_t i = 0; i < aci.getKernelCapabilities().getInterupts().getInteruptList().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKernelCapabilities().getInterupts().getInteruptList().size() && rightFound == false; j++)
		{
			if (aci.getKernelCapabilities().getInterupts().getInteruptList()[i] == acid.getKernelCapabilities().getInterupts().getInteruptList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			printf("[WARNING] ACI/KC InteruptsList: FAIL (0x%0x not permitted)\n", aci.getKernelCapabilities().getInterupts().getInteruptList()[i]);
		}
	}
	// check misc params
	if (aci.getKernelCapabilities().getMiscParams().getProgramType() != acid.getKernelCapabilities().getMiscParams().getProgramType())
	{
		printf("[WARNING] ACI/KC ProgramType: FAIL (%d not permitted)\n",  aci.getKernelCapabilities().getMiscParams().getProgramType());
	}
	// check kernel version
	uint32_t aciKernelVersion = (uint32_t)aci.getKernelCapabilities().getKernelVersion().getVerMajor() << 16 |  (uint32_t)aci.getKernelCapabilities().getKernelVersion().getVerMinor();
	uint32_t acidKernelVersion =  (uint32_t)acid.getKernelCapabilities().getKernelVersion().getVerMajor() << 16 |  (uint32_t)acid.getKernelCapabilities().getKernelVersion().getVerMinor();
	if (aciKernelVersion < acidKernelVersion)
	{
		printf("[WARNING] ACI/KC RequiredKernelVersion: FAIL (%d.%d not permitted)\n", aci.getKernelCapabilities().getKernelVersion().getVerMajor(), aci.getKernelCapabilities().getKernelVersion().getVerMinor());
	}
	// check handle table size
	if (aci.getKernelCapabilities().getHandleTableSize().getHandleTableSize() > acid.getKernelCapabilities().getHandleTableSize().getHandleTableSize())
	{
		printf("[WARNING] ACI/KC HandleTableSize: FAIL (0x%x too large)\n", aci.getKernelCapabilities().getHandleTableSize().getHandleTableSize());
	}
	// check misc flags
	for (size_t i = 0; i < aci.getKernelCapabilities().getMiscFlags().getFlagList().size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < acid.getKernelCapabilities().getMiscFlags().getFlagList().size() && rightFound == false; j++)
		{
			if (aci.getKernelCapabilities().getMiscFlags().getFlagList()[i] == acid.getKernelCapabilities().getMiscFlags().getFlagList()[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			printf("[WARNING] ACI/KC MiscFlag: FAIL (%s not permitted)\n", kMiscFlag[aci.getKernelCapabilities().getMiscFlags().getFlagList()[i]].c_str());
		}
	}
}

void NpdmProcess::displayNpdmHeader(const nn::hac::NpdmBinary& hdr)
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

void NpdmProcess::displayAciHdr(const nn::hac::AccessControlInfoBinary& aci)
{
	printf("[Access Control Info]\n");
	printf("  ProgramID:       0x%016" PRIx64 "\n", aci.getProgramId());
}

void NpdmProcess::displayAciDescHdr(const nn::hac::AccessControlInfoDescBinary& acid)
{
	printf("[Access Control Info Desc]\n");
	if (acid.getFlagList().size() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		printf("  Flags:           \n");
		for (size_t i = 0; i < acid.getFlagList().size(); i++)
		{
			printf("    %s (%d)\n", kAcidFlag[acid.getFlagList()[i]].c_str(), acid.getFlagList()[i]);
		}
	}
	printf("  ProgramID Restriction\n");
	printf("    Min:           0x%016" PRIx64 "\n", acid.getProgramIdRestrict().min);
	printf("    Max:           0x%016" PRIx64 "\n", acid.getProgramIdRestrict().max);
}

void NpdmProcess::displayFac(const nn::hac::FileSystemAccessControlBinary& fac)
{
	printf("[FS Access Control]\n");
	printf("  Format Version:  %d\n", fac.getFormatVersion());

	if (fac.getFsaRightsList().size())
	{
		printf("  FS Rights:\n");
		for (size_t i = 0; i < fac.getFsaRightsList().size(); i++)
		{
			if (i % 10 == 0)
			{
				printf("%s    ", i != 0 ? "\n" : "");
			}
			printf("%s", kFsaFlag[fac.getFsaRightsList()[i]].c_str());
			if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
				printf(" (bit %" PRId32 ")", fac.getFsaRightsList()[i]);
			printf("%s", fac.getFsaRightsList()[i] != fac.getFsaRightsList().atBack() ? ", " : "\n");			
		}
	}
	
	if (fac.getContentOwnerIdList().size())
	{
		printf("  Content Owner IDs:\n");
		for (size_t i = 0; i < fac.getContentOwnerIdList().size(); i++)
		{
			printf("    0x%016" PRIx64 "\n", fac.getContentOwnerIdList()[i]);
		}
	}

	if (fac.getSaveDataOwnerIdList().size())
	{
		printf("  Save Data Owner IDs:\n");
		for (size_t i = 0; i < fac.getSaveDataOwnerIdList().size(); i++)
		{
			printf("    0x%016" PRIx64 " (%s)\n", fac.getSaveDataOwnerIdList()[i].id, kSaveDataOwnerAccessMode[fac.getSaveDataOwnerIdList()[i].access_type].c_str());
		}
	}
	
}

void NpdmProcess::displaySac(const nn::hac::ServiceAccessControlBinary& sac)
{
	printf("[Service Access Control]\n");
	printf("  Service List:\n");
	for (size_t i = 0; i < sac.getServiceList().size(); i++)
	{
		if (i % 10 == 0)
		{
			printf("%s    ", i != 0 ? "\n" : "");
		}
		printf("%s%s%s", sac.getServiceList()[i].getName().c_str(), sac.getServiceList()[i].isServer() ? "(isSrv)" : "", sac.getServiceList()[i] != sac.getServiceList().atBack() ? ", " : "\n");
	}
}

void NpdmProcess::displayKernelCap(const nn::hac::KernelCapabilityBinary& kern)
{
	printf("[Kernel Capabilities]\n");
	if (kern.getThreadInfo().isSet())
	{
		nn::hac::ThreadInfoHandler threadInfo = kern.getThreadInfo();
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
		for (size_t i = 0; i < syscalls.size(); i++)
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
		fnd::List<nn::hac::MemoryMappingHandler::sMemoryMapping> maps = kern.getMemoryMaps().getMemoryMaps();
		fnd::List<nn::hac::MemoryMappingHandler::sMemoryMapping> ioMaps = kern.getMemoryMaps().getIoMemoryMaps();

		printf("  MemoryMaps:\n");
		for (size_t i = 0; i < maps.size(); i++)
		{
			printf("    0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s)\n", (uint64_t)maps[i].addr << 12, ((uint64_t)(maps[i].addr + maps[i].size) << 12) - 1, kMemMapPerm[maps[i].perm].c_str(), kMemMapType[maps[i].type].c_str());
		}
		//printf("  IoMaps:\n");
		for (size_t i = 0; i < ioMaps.size(); i++)
		{
			printf("    0x%016" PRIx64 " - 0x%016" PRIx64 " (perm=%s) (type=%s)\n", (uint64_t)ioMaps[i].addr << 12, ((uint64_t)(ioMaps[i].addr + ioMaps[i].size) << 12) - 1, kMemMapPerm[ioMaps[i].perm].c_str(), kMemMapType[ioMaps[i].type].c_str());
		}
	}
	if (kern.getInterupts().isSet())
	{
		fnd::List<uint16_t> interupts = kern.getInterupts().getInteruptList();
		printf("  Interupts Flags:\n");
		for (uint32_t i = 0; i < interupts.size(); i++)
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
		fnd::List<nn::hac::MiscFlagsHandler::Flags> flagList = kern.getMiscFlags().getFlagList();

		printf("  Misc Flags:\n");
		for (uint32_t i = 0; i < flagList.size(); i++)
		{
			if (i % 10 == 0)
			{
				printf("%s    ", i != 0 ? "\n" : "");
			}
			printf("%s%s", kMiscFlag[flagList[i]].c_str(), flagList[i] != flagList.atBack() ? ", " : "\n");
		}
	}
}
