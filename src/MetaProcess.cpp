#include <iostream>
#include <iomanip>
#include "MetaProcess.h"
#include <nn/hac/define/svc.h>

MetaProcess::MetaProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void MetaProcess::process()
{
	importMeta();

	if (mVerify)
	{
		validateAcidSignature(mMeta.getAcid());
		validateAciFromAcid(mMeta.getAci(), mMeta.getAcid());
	}

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
	{
		// npdm binary
		displayMetaHeader(mMeta);

		// aci binary
		displayAciHdr(mMeta.getAci());
		displayFac(mMeta.getAci().getFileSystemAccessControl());
		displaySac(mMeta.getAci().getServiceAccessControl());
		displayKernelCap(mMeta.getAci().getKernelCapabilities());

		// acid binary
		if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		{
			displayAciDescHdr(mMeta.getAcid());
			displayFac(mMeta.getAcid().getFileSystemAccessControl());
			displaySac(mMeta.getAcid().getServiceAccessControl());
			displayKernelCap(mMeta.getAcid().getKernelCapabilities());
		}
	}
}

void MetaProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void MetaProcess::setKeyCfg(const KeyConfiguration& keycfg)
{
	mKeyCfg = keycfg;
}

void MetaProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void MetaProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nn::hac::Meta& MetaProcess::getMeta() const
{
	return mMeta;
}

void MetaProcess::importMeta()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mMeta.fromBytes(scratch.data(), scratch.size());
}

void MetaProcess::validateAcidSignature(const nn::hac::AccessControlInfoDesc& acid)
{
	try {
		fnd::rsa::sRsa2048Key acid_sign_key;
		if (mKeyCfg.getAcidSignKey(acid_sign_key) != true)
			throw fnd::Exception();

		acid.validateSignature(acid_sign_key);
	}
	catch (...) {
		std::cout << "[WARNING] ACID Signature: FAIL" << std::endl;
	}
	
}

void MetaProcess::validateAciFromAcid(const nn::hac::AccessControlInfo& aci, const nn::hac::AccessControlInfoDesc& acid)
{
	// check Program ID
	if (acid.getProgramIdRestrict().min > 0 && aci.getProgramId() < acid.getProgramIdRestrict().min)
	{
		std::cout << "[WARNING] ACI ProgramId: FAIL (Outside Legal Range)" << std::endl;
	}
	else if (acid.getProgramIdRestrict().max > 0 && aci.getProgramId() > acid.getProgramIdRestrict().max)
	{
		std::cout << "[WARNING] ACI ProgramId: FAIL (Outside Legal Range)" << std::endl;
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

			std::cout << "[WARNING] ACI/FAC FsaRights: FAIL (" << getFsaRightStr(aci.getFileSystemAccessControl().getFsaRightsList()[i]) << " not permitted)" << std::endl;
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

			std::cout << "[WARNING] ACI/FAC ContentOwnerId: FAIL (" << std::hex << std::setw(16) << std::setfill('0') << aci.getFileSystemAccessControl().getContentOwnerIdList()[i] << " not permitted)" << std::endl;
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

			std::cout << "[WARNING] ACI/FAC SaveDataOwnerId: FAIL (" << std::hex << std::setw(16) << std::setfill('0') << aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i].id << "(" << std::dec << (uint32_t)aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i].access_type << ") not permitted)" << std::endl;
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
			std::cout << "[WARNING] ACI/SAC ServiceList: FAIL (" << aci.getServiceAccessControl().getServiceList()[i].getName() << (aci.getServiceAccessControl().getServiceList()[i].isServer()? " (Server)" : "") << " not permitted)" << std::endl;
		}
	}

	// check KC
	// check thread info
	if (aci.getKernelCapabilities().getThreadInfo().getMaxCpuId() != acid.getKernelCapabilities().getThreadInfo().getMaxCpuId())
	{
		std::cout << "[WARNING] ACI/KC ThreadInfo/MaxCpuId: FAIL (" << std::dec << (uint32_t)aci.getKernelCapabilities().getThreadInfo().getMaxCpuId() << " not permitted)" << std::endl;
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMinCpuId() != acid.getKernelCapabilities().getThreadInfo().getMinCpuId())
	{
		std::cout << "[WARNING] ACI/KC ThreadInfo/MinCpuId: FAIL (" << std::dec << (uint32_t)aci.getKernelCapabilities().getThreadInfo().getMinCpuId() << " not permitted)" << std::endl;
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMaxPriority() != acid.getKernelCapabilities().getThreadInfo().getMaxPriority())
	{
		std::cout << "[WARNING] ACI/KC ThreadInfo/MaxPriority: FAIL (" << std::dec << (uint32_t)aci.getKernelCapabilities().getThreadInfo().getMaxPriority() << " not permitted)" << std::endl;
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMinPriority() != acid.getKernelCapabilities().getThreadInfo().getMinPriority())
	{
		std::cout << "[WARNING] ACI/KC ThreadInfo/MinPriority: FAIL (" << std::dec << (uint32_t)aci.getKernelCapabilities().getThreadInfo().getMinPriority() << " not permitted)" << std::endl;
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
			std::cout << "[WARNING] ACI/KC SystemCallList: FAIL (" << getSystemCallStr(aci.getKernelCapabilities().getSystemCalls().getSystemCalls()[i]) << " not permitted)" << std::endl;
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

			std::cout << "[WARNING] ACI/KC MemoryMap: FAIL (0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)map.addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(map.addr + map.size) << 12) - 1) << " (perm=" << getMemMapPermStr(map.perm) << ") (type=" << getMemMapTypeStr(map.type) << ") not permitted)" << std::endl;
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

			std::cout << "[WARNING] ACI/KC IoMemoryMap: FAIL (0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)map.addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(map.addr + map.size) << 12) - 1) << " (perm=" << getMemMapPermStr(map.perm) << ") (type=" << getMemMapTypeStr(map.type) << ") not permitted)" << std::endl;
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
			std::cout << "[WARNING] ACI/KC InteruptsList: FAIL (0x" << std::hex << (uint32_t)aci.getKernelCapabilities().getInterupts().getInteruptList()[i] << " not permitted)" << std::endl;
		}
	}
	// check misc params
	if (aci.getKernelCapabilities().getMiscParams().getProgramType() != acid.getKernelCapabilities().getMiscParams().getProgramType())
	{
		std::cout << "[WARNING] ACI/KC ProgramType: FAIL (" << std::dec << (uint32_t)aci.getKernelCapabilities().getMiscParams().getProgramType() << " not permitted)" << std::endl;
	}
	// check kernel version
	uint32_t aciKernelVersion = (uint32_t)aci.getKernelCapabilities().getKernelVersion().getVerMajor() << 16 |  (uint32_t)aci.getKernelCapabilities().getKernelVersion().getVerMinor();
	uint32_t acidKernelVersion =  (uint32_t)acid.getKernelCapabilities().getKernelVersion().getVerMajor() << 16 |  (uint32_t)acid.getKernelCapabilities().getKernelVersion().getVerMinor();
	if (aciKernelVersion < acidKernelVersion)
	{
		std::cout << "[WARNING] ACI/KC RequiredKernelVersion: FAIL (" << std::dec << aci.getKernelCapabilities().getKernelVersion().getVerMajor() << "." << aci.getKernelCapabilities().getKernelVersion().getVerMinor() << " not permitted)" << std::endl;
	}
	// check handle table size
	if (aci.getKernelCapabilities().getHandleTableSize().getHandleTableSize() > acid.getKernelCapabilities().getHandleTableSize().getHandleTableSize())
	{
		std::cout << "[WARNING] ACI/KC HandleTableSize: FAIL (0x" << std::hex << (uint32_t)aci.getKernelCapabilities().getHandleTableSize().getHandleTableSize() << " too large)" << std::endl;
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
			std::cout << "[WARNING] ACI/KC MiscFlag: FAIL (" << getMiscFlagStr(aci.getKernelCapabilities().getMiscFlags().getFlagList()[i]) << " not permitted)" << std::endl;
		}
	}
}

void MetaProcess::displayMetaHeader(const nn::hac::Meta& hdr)
{
	std::cout << "[Meta Header]" << std::endl;
	std::cout << "  Process Architecture Params:" << std::endl;
	std::cout << "    Ins. Type:     " << getInstructionTypeStr(hdr.getInstructionType()) << std::endl;
	std::cout << "    Addr Space:    " << getProcAddressSpaceTypeStr(hdr.getProcAddressSpaceType()) << std::endl;
	std::cout << "  Main Thread Params:" << std::endl;
	std::cout << "    Priority:      " << std::dec << (uint32_t)hdr.getMainThreadPriority() << std::endl;
	std::cout << "    CpuId:         " << std::dec << (uint32_t)hdr.getMainThreadCpuId() << std::endl;
	std::cout << "    StackSize:     0x" << std::hex << hdr.getMainThreadStackSize() << std::endl;
	std::cout << "  TitleInfo:" << std::endl;
	std::cout << "    Version:       v" << std::dec << hdr.getVersion() << std::endl;
	std::cout << "    Name:          " << hdr.getName() << std::endl;
	if (hdr.getProductCode().length())
	{
		std::cout << "    ProductCode:   " << hdr.getProductCode() << std::endl;
	}
}

void MetaProcess::displayAciHdr(const nn::hac::AccessControlInfo& aci)
{
	std::cout << "[Access Control Info]" << std::endl;
	std::cout << "  ProgramID:       0x" << std::hex << std::setw(16) << std::setfill('0') << aci.getProgramId() << std::endl;
}

void MetaProcess::displayAciDescHdr(const nn::hac::AccessControlInfoDesc& acid)
{
	std::cout << "[Access Control Info Desc]" << std::endl;
	if (acid.getFlagList().size() > 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Flags:           " << std::endl;
		for (size_t i = 0; i < acid.getFlagList().size(); i++)
		{
			std::cout << "    " << getAcidFlagStr(acid.getFlagList()[i]) << " (" << std::dec << (uint32_t)acid.getFlagList()[i] << ")" << std::endl;
		}
	}
	std::cout << "  ProgramID Restriction" << std::endl;
	std::cout << "    Min:           0x" << std::hex << std::setw(16) << std::setfill('0') << acid.getProgramIdRestrict().min << std::endl;
	std::cout << "    Max:           0x" << std::hex << std::setw(16) << std::setfill('0') << acid.getProgramIdRestrict().max << std::endl;
}

void MetaProcess::displayFac(const nn::hac::FileSystemAccessControl& fac)
{
	std::cout << "[FS Access Control]" << std::endl;
	std::cout << "  Format Version:  " << std::dec << (uint32_t)fac.getFormatVersion() << std::endl;

	if (fac.getFsaRightsList().size())
	{
		std::cout << "  FS Rights:" << std::endl;
		for (size_t i = 0; i < fac.getFsaRightsList().size(); i++)
		{
			if (i % 10 == 0)
			{
				if (i != 0)
					std::cout << std::endl;
				std::cout << "    ";
			}
			std::cout << getFsaRightStr(fac.getFsaRightsList()[i]);
			if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
				std::cout << " (bit " << std::dec << (uint32_t)fac.getFsaRightsList()[i] << ")";
			if (fac.getFsaRightsList()[i] != fac.getFsaRightsList().atBack())
				std::cout << ", ";
		}
		std::cout << std::endl;
	}
	
	if (fac.getContentOwnerIdList().size())
	{
		std::cout << "  Content Owner IDs:" << std::endl;
		for (size_t i = 0; i < fac.getContentOwnerIdList().size(); i++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << fac.getContentOwnerIdList()[i] << std::endl;
		}
	}

	if (fac.getSaveDataOwnerIdList().size())
	{
		std::cout << "  Save Data Owner IDs:" << std::endl;
		for (size_t i = 0; i < fac.getSaveDataOwnerIdList().size(); i++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << fac.getSaveDataOwnerIdList()[i].id << " (" << getSaveDataOwnerAccessModeStr(fac.getSaveDataOwnerIdList()[i].access_type) << ")" << std::endl;
		}
	}
	
}

void MetaProcess::displaySac(const nn::hac::ServiceAccessControl& sac)
{
	std::cout << "[Service Access Control]" << std::endl;
	std::cout << "  Service List:" << std::endl;
	for (size_t i = 0; i < sac.getServiceList().size(); i++)
	{
		if (i % 10 == 0)
		{
			if (i != 0)
				std::cout << std::endl;
			std::cout << "    ";
		}
		std::cout << sac.getServiceList()[i].getName();
		if (sac.getServiceList()[i].isServer())
			std::cout << "(isSrv)";
		if (sac.getServiceList()[i] != sac.getServiceList().atBack())
			std::cout << ", ";
	}
	std::cout << std::endl;
}

void MetaProcess::displayKernelCap(const nn::hac::KernelCapabilityControl& kern)
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
			std::cout << std::endl;
		}
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

const char* MetaProcess::getInstructionTypeStr(nn::hac::meta::InstructionType type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::meta::INSTR_32BIT):
		str = "32Bit";
		break;
	case (nn::hac::meta::INSTR_64BIT):
		str = "64Bit";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* MetaProcess::getProcAddressSpaceTypeStr(nn::hac::meta::ProcAddrSpaceType type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::meta::ADDR_SPACE_64BIT):
		str = "64Bit";
		break;
	case (nn::hac::meta::ADDR_SPACE_32BIT):
		str = "32Bit";
		break;
	case (nn::hac::meta::ADDR_SPACE_32BIT_NO_RESERVED):
		str = "32Bit no reserved";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* MetaProcess::getAcidFlagStr(nn::hac::aci::Flag flag) const
{
	const char* str = nullptr;

	switch(flag)
	{
	case (nn::hac::aci::FLAG_PRODUCTION):
		str = "Production";
		break;
	case (nn::hac::aci::FLAG_UNQUALIFIED_APPROVAL):
		str = "UnqualifiedApproval";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* MetaProcess::getMiscFlagStr(nn::hac::MiscFlagsHandler::Flags flag) const
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

const char* MetaProcess::getFsaRightStr(nn::hac::fac::FsAccessFlag flag) const
{
	const char* str = nullptr;

	switch(flag)
	{
	case (nn::hac::fac::FSA_APPLICATION_INFO):
		str = "ApplicationInfo";
		break;
	case (nn::hac::fac::FSA_BOOT_MODE_CONTROL):
		str = "BootModeControl";
		break;
	case (nn::hac::fac::FSA_CALIBRATION):
		str = "Calibration";
		break;
	case (nn::hac::fac::FSA_SYSTEM_SAVE_DATA):
		str = "SystemSaveData";
		break;
	case (nn::hac::fac::FSA_GAME_CARD):
		str = "GameCard";
		break;
	case (nn::hac::fac::FSA_SAVE_DATA_BACKUP):
		str = "SaveDataBackUp";
		break;
	case (nn::hac::fac::FSA_SAVE_DATA_MANAGEMENT):
		str = "SaveDataManagement";
		break;
	case (nn::hac::fac::FSA_BIS_ALL_RAW):
		str = "BisAllRaw";
		break;
	case (nn::hac::fac::FSA_GAME_CARD_RAW):
		str = "GameCardRaw";
		break;
	case (nn::hac::fac::FSA_GAME_CARD_PRIVATE):
		str = "GameCardPrivate";
		break;
	case (nn::hac::fac::FSA_SET_TIME):
		str = "SetTime";
		break;
	case (nn::hac::fac::FSA_CONTENT_MANAGER):
		str = "ContentManager";
		break;
	case (nn::hac::fac::FSA_IMAGE_MANAGER):
		str = "ImageManager";
		break;
	case (nn::hac::fac::FSA_CREATE_SAVE_DATA):
		str = "CreateSaveData";
		break;
	case (nn::hac::fac::FSA_SYSTEM_SAVE_DATA_MANAGEMENT):
		str = "SystemSaveDataManagement";
		break;
	case (nn::hac::fac::FSA_BIS_FILE_SYSTEM):
		str = "BisFileSystem";
		break;
	case (nn::hac::fac::FSA_SYSTEM_UPDATE):
		str = "SystemUpdate";
		break;
	case (nn::hac::fac::FSA_SAVE_DATA_META):
		str = "SaveDataMeta";
		break;
	case (nn::hac::fac::FSA_DEVICE_SAVE_CONTROL):
		str = "DeviceSaveData";
		break;
	case (nn::hac::fac::FSA_SETTINGS_CONTROL):
		str = "SettingsControl";
		break;
	case (nn::hac::fac::FSA_SYSTEM_DATA):
		str = "SystemData";
		break;
	case (nn::hac::fac::FSA_SD_CARD):
		str = "SdCard";
		break;
	case (nn::hac::fac::FSA_HOST):
		str = "Host";
		break;
	case (nn::hac::fac::FSA_FILL_BIS):
		str = "FillBis";
		break;
	case (nn::hac::fac::FSA_CORRUPT_SAVE_DATA):
		str = "CorruptSaveData";
		break;
	case (nn::hac::fac::FSA_SAVE_DATA_FOR_DEBUG):
		str = "SaveDataForDebug";
		break;
	case (nn::hac::fac::FSA_FORMAT_SD_CARD):
		str = "FormateSdCard";
		break;
	case (nn::hac::fac::FSA_GET_RIGHTS_ID):
		str = "GetRightsId";
		break;
	case (nn::hac::fac::FSA_REGISTER_EXTERNAL_KEY):
		str = "RegisterExternalKey";
		break;
	case (nn::hac::fac::FSA_REGISTER_UPDATE_PARTITION):
		str = "RegisterUpdatePartition";
		break;
	case (nn::hac::fac::FSA_SAVE_DATA_TRANSFER):
		str = "SaveDataTransfer";
		break;
	case (nn::hac::fac::FSA_DEVICE_DETECTION):
		str = "DeviceDetection";
		break;
	case (nn::hac::fac::FSA_ACCESS_FAILURE_RESOLUTION):
		str = "AccessFailureResolution";
		break;
	case (nn::hac::fac::FSA_SAVE_DATA_TRANSFER_V2):
		str = "SaveDataTransfer2";
		break;
	case (nn::hac::fac::FSA_DEBUG):
		str = "Debug";
		break;
	case (nn::hac::fac::FSA_FULL_PERMISSION):
		str = "FullPermission";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* MetaProcess::getSaveDataOwnerAccessModeStr(nn::hac::fac::SaveDataOwnerIdAccessType type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::fac::SDO_READ):
		str = "Read";
		break;
	case (nn::hac::fac::SDO_WRITE):
		str = "Write";
		break;
	case (nn::hac::fac::SDO_READWRITE):
		str = "ReadWrite";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* MetaProcess::getSystemCallStr(byte_t syscall_id) const
{
	const char* str = nullptr;

	switch(syscall_id)
	{
	case (nn::hac::svc::SVC_SET_HEAP_SIZE):
		str = "SetHeapSize";
		break;
	case (nn::hac::svc::SVC_SET_MEMORY_PERMISSION):
		str = "SetMemoryPermission";
		break;
	case (nn::hac::svc::SVC_SET_MEMORY_ATTRIBUTE):
		str = "SetMemoryAttribute";
		break;
	case (nn::hac::svc::SVC_MAP_MEMORY):
		str = "MapMemory";
		break;
	case (nn::hac::svc::SVC_UNMAP_MEMORY):
		str = "UnmapMemory";
		break;
	case (nn::hac::svc::SVC_QUERY_MEMORY):
		str = "QueryMemory";
		break;
	case (nn::hac::svc::SVC_EXIT_PROCESS):
		str = "ExitProcess";
		break;
	case (nn::hac::svc::SVC_CREATE_THREAD):
		str = "CreateThread";
		break;
	case (nn::hac::svc::SVC_START_THREAD):
		str = "StartThread";
		break;
	case (nn::hac::svc::SVC_EXIT_THREAD):
		str = "ExitThread";
		break;
	case (nn::hac::svc::SVC_SLEEP_THREAD):
		str = "SleepThread";
		break;
	case (nn::hac::svc::SVC_GET_THREAD_PRIORITY):
		str = "GetThreadPriority";
		break;
	case (nn::hac::svc::SVC_SET_THREAD_PRIORITY):
		str = "SetThreadPriority";
		break;
	case (nn::hac::svc::SVC_GET_THREAD_CORE_MASK):
		str = "GetThreadCoreMask";
		break;
	case (nn::hac::svc::SVC_SET_THREAD_CORE_MASK):
		str = "SetThreadCoreMask";
		break;
	case (nn::hac::svc::SVC_GET_CURRENT_PROCESSOR_NUMBER):
		str = "GetCurrentProcessorNumber";
		break;
	case (nn::hac::svc::SVC_SIGNAL_EVENT):
		str = "SignalEvent";
		break;
	case (nn::hac::svc::SVC_CLEAR_EVENT):
		str = "ClearEvent";
		break;
	case (nn::hac::svc::SVC_MAP_SHARED_MEMORY):
		str = "MapSharedMemory";
		break;
	case (nn::hac::svc::SVC_UNMAP_SHARED_MEMORY):
		str = "UnmapSharedMemory";
		break;
	case (nn::hac::svc::SVC_CREATE_TRANSFER_MEMORY):
		str = "CreateTransferMemory";
		break;
	case (nn::hac::svc::SVC_CLOSE_HANDLE):
		str = "CloseHandle";
		break;
	case (nn::hac::svc::SVC_RESET_SIGNAL):
		str = "ResetSignal";
		break;
	case (nn::hac::svc::SVC_WAIT_SYNCHRONIZATION):
		str = "WaitSynchronization";
		break;
	case (nn::hac::svc::SVC_CANCEL_SYNCHRONIZATION):
		str = "CancelSynchronization";
		break;
	case (nn::hac::svc::SVC_ARBIRATE_LOCK):
		str = "ArbitrateLock";
		break;
	case (nn::hac::svc::SVC_ARBIRATE_UNLOCK):
		str = "ArbitrateUnlock";
		break;
	case (nn::hac::svc::SVC_WAIT_PROCESS_WIDE_KEY_ATOMIC):
		str = "WaitProcessWideKeyAtomic";
		break;
	case (nn::hac::svc::SVC_SIGNAL_PROCESS_WIDE_KEY):
		str = "SignalProcessWideKey";
		break;
	case (nn::hac::svc::SVC_GET_SYSTEM_TICK):
		str = "GetSystemTick";
		break;
	case (nn::hac::svc::SVC_CONNECT_TO_NAMED_PORT):
		str = "ConnectToNamedPort";
		break;
	case (nn::hac::svc::SVC_SEND_SYNC_REQUEST_LIGHT):
		str = "SendSyncRequestLight";
		break;
	case (nn::hac::svc::SVC_SEND_SYNC_REQUEST):
		str = "SendSyncRequest";
		break;
	case (nn::hac::svc::SVC_SEND_SYNC_REQUEST_WITH_USER_BUFFER):
		str = "SendSyncRequestWithUserBuffer";
		break;
	case (nn::hac::svc::SVC_SEND_ASYNC_REQUEST_WITH_USER_BUFFER):
		str = "SendAsyncRequestWithUserBuffer";
		break;
	case (nn::hac::svc::SVC_GET_PROCESS_ID):
		str = "GetProcessId";
		break;
	case (nn::hac::svc::SVC_GET_THREAD_ID):
		str = "GetThreadId";
		break;
	case (nn::hac::svc::SVC_BREAK):
		str = "Break";
		break;
	case (nn::hac::svc::SVC_OUTPUT_DEBUG_STRING):
		str = "OutputDebugString";
		break;
	case (nn::hac::svc::SVC_RETURN_FROM_EXCEPTION):
		str = "ReturnFromException";
		break;
	case (nn::hac::svc::SVC_GET_INFO):
		str = "GetInfo";
		break;
	case (nn::hac::svc::SVC_FLUSH_ENTIRE_DATA_CACHE):
		str = "FlushEntireDataCache";
		break;
	case (nn::hac::svc::SVC_FLUSH_DATA_CACHE):
		str = "FlushDataCache";
		break;
	case (nn::hac::svc::SVC_MAP_PHYSICAL_MEMORY):
		str = "MapPhysicalMemory";
		break;
	case (nn::hac::svc::SVC_UNMAP_PHYSICAL_MEMORY):
		str = "UnmapPhysicalMemory";
		break;
	case (nn::hac::svc::SVC_GET_FUTURE_THREAD_INFO):
		str = "GetFutureThreadInfo";
		break;
	case (nn::hac::svc::SVC_GET_LAST_THREAD_INFO):
		str = "GetLastThreadInfo";
		break;
	case (nn::hac::svc::SVC_GET_RESOURCE_LIMIT_LIMIT_VALUE):
		str = "GetResourceLimitLimitValue";
		break;
	case (nn::hac::svc::SVC_GET_RESOURCE_LIMIT_CURRENT_VALUE):
		str = "GetResourceLimitCurrentValue";
		break;
	case (nn::hac::svc::SVC_SET_THREAD_ACTIVITY):
		str = "SetThreadActivity";
		break;
	case (nn::hac::svc::SVC_GET_THREAD_CONTEXT3):
		str = "GetThreadContext3";
		break;
	case (nn::hac::svc::SVC_WAIT_FOR_ADDRESS):
		str = "WaitForAddress";
		break;
	case (nn::hac::svc::SVC_SIGNAL_TO_ADDRESS):
		str = "SignalToAddress";
		break;
	case (nn::hac::svc::SVC_SYNCHRONIZE_PREEMPTION_STATE):
		str = "SynchronizePreemptionState";
		break;
	case (nn::hac::svc::SVC_UNK_0x37):
		str = "svc37";
		break;
	case (nn::hac::svc::SVC_UNK_0x38):
		str = "svc38";
		break;
	case (nn::hac::svc::SVC_UNK_0x39):
		str = "svc39";
		break;
	case (nn::hac::svc::SVC_UNK_0x3A):
		str = "svc3A";
		break;
	case (nn::hac::svc::SVC_UNK_0x3B):
		str = "svc3B";
		break;
	case (nn::hac::svc::SVC_DUMP_INFO):
		str = "DumpInfo";
		break;
	case (nn::hac::svc::SVC_DUMP_INFO_NEW):
		str = "DumpInfoNew";
		break;
	case (nn::hac::svc::SVC_UNK_0x3E):
		str = "svc3E";
		break;
	case (nn::hac::svc::SVC_UNK_0x3F):
		str = "svc3F";
		break;
	case (nn::hac::svc::SVC_CREATE_SESSION):
		str = "CreateSession";
		break;
	case (nn::hac::svc::SVC_ACCEPT_SESSION):
		str = "AcceptSession";
		break;
	case (nn::hac::svc::SVC_REPLY_AND_RECEIVE_LIGHT):
		str = "ReplyAndReceiveLight";
		break;
	case (nn::hac::svc::SVC_REPLY_AND_RECEIVE):
		str = "ReplyAndReceive";
		break;
	case (nn::hac::svc::SVC_REPLY_AND_RECEIVE_WITH_USER_BUFFER):
		str = "ReplyAndReceiveWithUserBuffer";
		break;
	case (nn::hac::svc::SVC_CREATE_EVENT):
		str = "CreateEvent";
		break;
	case (nn::hac::svc::SVC_UNK_0x46):
		str = "svc46";
		break;
	case (nn::hac::svc::SVC_UNK_0x47):
		str = "svc47";
		break;
	case (nn::hac::svc::SVC_MAP_PHYSICAL_MEMORY_UNSAFE):
		str = "MapPhysicalMemoryUnsafe";
		break;
	case (nn::hac::svc::SVC_UNMAP_PHYSICAL_MEMORY_UNSAFE):
		str = "UnmapPhysicalMemoryUnsafe";
		break;
	case (nn::hac::svc::SVC_SET_UNSAFE_LIMIT):
		str = "SetUnsafeLimit";
		break;
	case (nn::hac::svc::SVC_CREATE_CODE_MEMORY):
		str = "CreateCodeMemory";
		break;
	case (nn::hac::svc::SVC_CONTROL_CODE_MEMORY):
		str = "ControlCodeMemory";
		break;
	case (nn::hac::svc::SVC_SLEEP_SYSTEM):
		str = "SleepSystem";
		break;
	case (nn::hac::svc::SVC_READ_WRITE_REGISTER):
		str = "ReadWriteRegister";
		break;
	case (nn::hac::svc::SVC_SET_PROCESS_ACTIVITY):
		str = "SetProcessActivity";
		break;
	case (nn::hac::svc::SVC_CREATE_SHARED_MEMORY):
		str = "CreateSharedMemory";
		break;
	case (nn::hac::svc::SVC_MAP_TRANSFER_MEMORY):
		str = "MapTransferMemory";
		break;
	case (nn::hac::svc::SVC_UNMAP_TRANSFER_MEMORY):
		str = "UnmapTransferMemory";
		break;
	case (nn::hac::svc::SVC_CREATE_INTERRUPT_EVENT):
		str = "CreateInterruptEvent";
		break;
	case (nn::hac::svc::SVC_QUERY_PHYSICAL_ADDRESS):
		str = "QueryPhysicalAddress";
		break;
	case (nn::hac::svc::SVC_QUERY_IO_MAPPING):
		str = "QueryIoMapping";
		break;
	case (nn::hac::svc::SVC_CREATE_DEVICE_ADDRESS_SPACE):
		str = "CreateDeviceAddressSpace";
		break;
	case (nn::hac::svc::SVC_ATTACH_DEVICE_ADDRESS_SPACE):
		str = "AttachDeviceAddressSpace";
		break;
	case (nn::hac::svc::SVC_DETACH_DEVICE_ADDRESS_SPACE):
		str = "DetachDeviceAddressSpace";
		break;
	case (nn::hac::svc::SVC_MAP_DEVICE_ADDRESS_SPACE_BY_FORCE):
		str = "MapDeviceAddressSpaceByForce";
		break;
	case (nn::hac::svc::SVC_MAP_DEVICE_ADDRESS_SPACE_ALIGNED):
		str = "MapDeviceAddressSpaceAligned";
		break;
	case (nn::hac::svc::SVC_MAP_DEVICE_ADDRESS_SPACE):
		str = "MapDeviceAddressSpace";
		break;
	case (nn::hac::svc::SVC_UNMAP_DEVICE_ADDRESS_SPACE):
		str = "UnmapDeviceAddressSpace";
		break;
	case (nn::hac::svc::SVC_INVALIDATE_PROCESS_DATA_CACHE):
		str = "InvalidateProcessDataCache";
		break;
	case (nn::hac::svc::SVC_STORE_PROCESS_DATA_CACHE):
		str = "StoreProcessDataCache";
		break;
	case (nn::hac::svc::SVC_FLUSH_PROCESS_DATA_CACHE):
		str = "FlushProcessDataCache";
		break;
	case (nn::hac::svc::SVC_DEBUG_ACTIVE_PROCESS):
		str = "DebugActiveProcess";
		break;
	case (nn::hac::svc::SVC_BREAK_DEBUG_PROCESS):
		str = "BreakDebugProcess";
		break;
	case (nn::hac::svc::SVC_TERMINATE_DEBUG_PROCESS):
		str = "TerminateDebugProcess";
		break;
	case (nn::hac::svc::SVC_GET_DEBUG_EVENT):
		str = "GetDebugEvent";
		break;
	case (nn::hac::svc::SVC_CONTINUE_DEBUG_EVENT):
		str = "ContinueDebugEvent";
		break;
	case (nn::hac::svc::SVC_GET_PROCESS_LIST):
		str = "GetProcessList";
		break;
	case (nn::hac::svc::SVC_GET_THREAD_LIST):
		str = "GetThreadList";
		break;
	case (nn::hac::svc::SVC_GET_DEBUG_THREAD_CONTEXT):
		str = "GetDebugThreadContext";
		break;
	case (nn::hac::svc::SVC_SET_DEBUG_THREAD_CONTEXT):
		str = "SetDebugThreadContext";
		break;
	case (nn::hac::svc::SVC_QUERY_DEBUG_PROCESS_MEMORY):
		str = "QueryDebugProcessMemory";
		break;
	case (nn::hac::svc::SVC_READ_DEBUG_PROCESS_MEMORY):
		str = "ReadDebugProcessMemory";
		break;
	case (nn::hac::svc::SVC_WRITE_DEBUG_PROCESS_MEMORY):
		str = "WriteDebugProcessMemory";
		break;
	case (nn::hac::svc::SVC_SET_HARDWARE_BREAK_POINT):
		str = "SetHardwareBreakPoint";
		break;
	case (nn::hac::svc::SVC_GET_DEBUG_THREAD_PARAM):
		str = "GetDebugThreadParam";
		break;
	case (nn::hac::svc::SVC_UNK_0x6E):
		str = "svc6E";
		break;
	case (nn::hac::svc::SVC_GET_SYSTEM_INFO):
		str = "GetSystemInfo";
		break;
	case (nn::hac::svc::SVC_CREATE_PORT):
		str = "CreatePort";
		break;
	case (nn::hac::svc::SVC_MANAGE_NAMED_PORT):
		str = "ManageNamedPort";
		break;
	case (nn::hac::svc::SVC_CONNECT_TO_PORT):
		str = "ConnectToPort";
		break;
	case (nn::hac::svc::SVC_SET_PROCESS_MEMORY_PERMISSION):
		str = "SetProcessMemoryPermission";
		break;
	case (nn::hac::svc::SVC_MAP_PROCESS_MEMORY):
		str = "MapProcessMemory";
		break;
	case (nn::hac::svc::SVC_UNMAP_PROCESS_MEMORY):
		str = "UnmapProcessMemory";
		break;
	case (nn::hac::svc::SVC_QUERY_PROCESS_MEMORY):
		str = "QueryProcessMemory";
		break;
	case (nn::hac::svc::SVC_MAP_PROCESS_CODE_MEMORY):
		str = "MapProcessCodeMemory";
		break;
	case (nn::hac::svc::SVC_UNMAP_PROCESS_CODE_MEMORY):
		str = "UnmapProcessCodeMemory";
		break;
	case (nn::hac::svc::SVC_CREATE_PROCESS):
		str = "CreateProcess";
		break;
	case (nn::hac::svc::SVC_START_PROCESS):
		str = "StartProcess";
		break;
	case (nn::hac::svc::SVC_TERMINATE_PROCESS):
		str = "TerminateProcess";
		break;
	case (nn::hac::svc::SVC_GET_PROCESS_INFO):
		str = "GetProcessInfo";
		break;
	case (nn::hac::svc::SVC_CREATE_RESOURCE_LIMIT):
		str = "CreateResourceLimit";
		break;
	case (nn::hac::svc::SVC_SET_RESOURCE_LIMIT_LIMIT_VALUE):
		str = "SetResourceLimitLimitValue";
		break;
	case (nn::hac::svc::SVC_CALL_SECURE_MONITOR):
		str = "CallSecureMonitor";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* MetaProcess::getMemMapPermStr(nn::hac::MemoryMappingHandler::MemoryPerm type) const
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

const char* MetaProcess::getMemMapTypeStr(nn::hac::MemoryMappingHandler::MappingType type) const
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