#include <iostream>
#include <iomanip>
#include "NpdmProcess.h"

NpdmProcess::NpdmProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void NpdmProcess::process()
{
	importNpdm();

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

void NpdmProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void NpdmProcess::setKeyCfg(const KeyConfiguration& keycfg)
{
	mKeyCfg = keycfg;
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

void NpdmProcess::importNpdm()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mNpdm.fromBytes(scratch.data(), scratch.size());
}

void NpdmProcess::validateAcidSignature(const nn::hac::AccessControlInfoDescBinary& acid)
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

void NpdmProcess::validateAciFromAcid(const nn::hac::AccessControlInfoBinary& aci, const nn::hac::AccessControlInfoDescBinary& acid)
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

void NpdmProcess::displayNpdmHeader(const nn::hac::NpdmBinary& hdr)
{
	std::cout << "[NPDM HEADER]" << std::endl;
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

void NpdmProcess::displayAciHdr(const nn::hac::AccessControlInfoBinary& aci)
{
	std::cout << "[Access Control Info]" << std::endl;
	std::cout << "  ProgramID:       0x" << std::hex << std::setw(16) << std::setfill('0') << aci.getProgramId() << std::endl;
}

void NpdmProcess::displayAciDescHdr(const nn::hac::AccessControlInfoDescBinary& acid)
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

void NpdmProcess::displayFac(const nn::hac::FileSystemAccessControlBinary& fac)
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
			std::cout << std::endl;
		}
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

void NpdmProcess::displaySac(const nn::hac::ServiceAccessControlBinary& sac)
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

void NpdmProcess::displayKernelCap(const nn::hac::KernelCapabilityBinary& kern)
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
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)maps[i].addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(maps[i].addr + maps[i].size) << 12) - 1) << " (perm=" << getMemMapPermStr(maps[i].perm) << ") (type=" << getMemMapTypeStr(maps[i].type) << ") not permitted)" << std::endl;
		}
		//std::cout << "  IoMaps:" << std::endl;
		for (size_t i = 0; i < ioMaps.size(); i++)
		{
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)ioMaps[i].addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(ioMaps[i].addr + ioMaps[i].size) << 12) - 1) << " (perm=" << getMemMapPermStr(ioMaps[i].perm) << ") (type=" << getMemMapTypeStr(ioMaps[i].type) << ") not permitted)" << std::endl;
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

const char* NpdmProcess::getInstructionTypeStr(nn::hac::npdm::InstructionType type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::npdm::INSTR_32BIT):
		str = "32Bit";
		break;
	case (nn::hac::npdm::INSTR_64BIT):
		str = "64Bit";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* NpdmProcess::getProcAddressSpaceTypeStr(nn::hac::npdm::ProcAddrSpaceType type) const
{
	const char* str = nullptr;

	switch(type)
	{
	case (nn::hac::npdm::ADDR_SPACE_64BIT):
		str = "64Bit";
		break;
	case (nn::hac::npdm::ADDR_SPACE_32BIT):
		str = "32Bit";
		break;
	case (nn::hac::npdm::ADDR_SPACE_32BIT_NO_RESERVED):
		str = "32Bit no reserved";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* NpdmProcess::getAcidFlagStr(nn::hac::aci::Flag flag) const
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

const char* NpdmProcess::getMiscFlagStr(nn::hac::MiscFlagsHandler::Flags flag) const
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

const char* NpdmProcess::getFsaRightStr(nn::hac::fac::FsAccessFlag flag) const
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

const char* NpdmProcess::getSaveDataOwnerAccessModeStr(nn::hac::fac::SaveDataOwnerIdAccessType type) const
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

const char* NpdmProcess::getSystemCallStr(byte_t syscall_id) const
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

const char* NpdmProcess::getMemMapPermStr(nn::hac::MemoryMappingHandler::MemoryPerm type) const
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

const char* NpdmProcess::getMemMapTypeStr(nn::hac::MemoryMappingHandler::MappingType type) const
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