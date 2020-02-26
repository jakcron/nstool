#include "MetaProcess.h"

#include <iostream>
#include <iomanip>

#include <nn/hac/AccessControlInfoUtil.h>
#include <nn/hac/FileSystemAccessUtil.h>
#include <nn/hac/KernelCapabilityUtil.h>
#include <nn/hac/MetaUtil.h>

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

			std::cout << "[WARNING] ACI/FAC FsaRights: FAIL (" << nn::hac::FileSystemAccessUtil::getFsaRightAsString(aci.getFileSystemAccessControl().getFsaRightsList()[i]) << " not permitted)" << std::endl;
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
			std::cout << "[WARNING] ACI/KC SystemCallList: FAIL (" << nn::hac::KernelCapabilityUtil::getSystemCallAsString(aci.getKernelCapabilities().getSystemCalls().getSystemCalls()[i]) << " not permitted)" << std::endl;
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

			std::cout << "[WARNING] ACI/KC MemoryMap: FAIL (0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)map.addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(map.addr + map.size) << 12) - 1) << " (perm=" << nn::hac::KernelCapabilityUtil::getMemMapPermAsString(map.perm) << ") (type=" << nn::hac::KernelCapabilityUtil::getMemMapTypeAsString(map.type) << ") not permitted)" << std::endl;
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

			std::cout << "[WARNING] ACI/KC IoMemoryMap: FAIL (0x" << std::hex << std::setw(16) << std::setfill('0') << ((uint64_t)map.addr << 12) << " - 0x" << std::hex << std::setw(16) << std::setfill('0') << (((uint64_t)(map.addr + map.size) << 12) - 1) << " (perm=" << nn::hac::KernelCapabilityUtil::getMemMapPermAsString(map.perm) << ") (type=" << nn::hac::KernelCapabilityUtil::getMemMapTypeAsString(map.type) << ") not permitted)" << std::endl;
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
			std::cout << "[WARNING] ACI/KC MiscFlag: FAIL (" << nn::hac::KernelCapabilityUtil::getMiscFlagAsString(aci.getKernelCapabilities().getMiscFlags().getFlagList()[i]) << " not permitted)" << std::endl;
		}
	}
}

void MetaProcess::displayMetaHeader(const nn::hac::Meta& hdr)
{
	std::cout << "[Meta Header]" << std::endl;
	std::cout << "  Process Architecture Params:" << std::endl;
	std::cout << "    Ins. Type:     " << nn::hac::MetaUtil::getInstructionTypeAsString(hdr.getInstructionType()) << std::endl;
	std::cout << "    Addr Space:    " << nn::hac::MetaUtil::getProcAddressSpaceTypeAsString(hdr.getProcAddressSpaceType()) << std::endl;
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
			std::cout << "    " << nn::hac::AccessControlInfoUtil::getAcidFlagAsString(acid.getFlagList()[i]) << " (" << std::dec << (uint32_t)acid.getFlagList()[i] << ")" << std::endl;
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
			std::cout << nn::hac::FileSystemAccessUtil::getFsaRightAsString(fac.getFsaRightsList()[i]);
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
			std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << fac.getSaveDataOwnerIdList()[i].id << " (" << nn::hac::FileSystemAccessUtil::getSaveDataOwnerAccessModeAsString(fac.getSaveDataOwnerIdList()[i].access_type) << ")" << std::endl;
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
			std::string syscall_name = nn::hac::KernelCapabilityUtil::getSystemCallAsString(syscalls[i]);
			std::cout << syscall_name;
			if (syscalls[i] != syscalls.atBack())
				std::cout << ", ";
			lineLen += syscall_name.length();
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