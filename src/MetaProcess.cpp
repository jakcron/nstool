#include "MetaProcess.h"

#include <pietendo/hac/AccessControlInfoUtil.h>
#include <pietendo/hac/FileSystemAccessUtil.h>
#include <pietendo/hac/KernelCapabilityUtil.h>
#include <pietendo/hac/MetaUtil.h>

nstool::MetaProcess::MetaProcess() :
	mModuleName("nstool::MetaProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::MetaProcess::process()
{
	importMeta();

	if (mVerify)
	{
		validateAcidSignature(mMeta.getAccessControlInfoDesc(), mMeta.getAccessControlInfoDescKeyGeneration());
		validateAciFromAcid(mMeta.getAccessControlInfo(), mMeta.getAccessControlInfoDesc());
	}

	if (mCliOutputMode.show_basic_info)
	{
		// npdm binary
		displayMetaHeader(mMeta);

		// aci binary
		displayAciHdr(mMeta.getAccessControlInfo());
		displayFac(mMeta.getAccessControlInfo().getFileSystemAccessControl());
		displaySac(mMeta.getAccessControlInfo().getServiceAccessControl());
		displayKernelCap(mMeta.getAccessControlInfo().getKernelCapabilities());

		// acid binary
		if (mCliOutputMode.show_extended_info)
		{
			displayAciDescHdr(mMeta.getAccessControlInfoDesc());
			displayFac(mMeta.getAccessControlInfoDesc().getFileSystemAccessControl());
			displaySac(mMeta.getAccessControlInfoDesc().getServiceAccessControl());
			displayKernelCap(mMeta.getAccessControlInfoDesc().getKernelCapabilities());
		}
	}
}

void nstool::MetaProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::MetaProcess::setKeyCfg(const KeyBag& keycfg)
{
	mKeyCfg = keycfg;
}

void nstool::MetaProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::MetaProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const pie::hac::Meta& nstool::MetaProcess::getMeta() const
{
	return mMeta;
}

void nstool::MetaProcess::importMeta()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is greater than 20MB, don't import.
	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());
	if (file_size > (0x100000 * 20))
	{
		throw tc::Exception(mModuleName, "File too large.");
	}

	// read meta
	tc::ByteData scratch = tc::ByteData(file_size);
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	mMeta.fromBytes(scratch.data(), scratch.size());
}

void nstool::MetaProcess::validateAcidSignature(const pie::hac::AccessControlInfoDesc& acid, byte_t key_generation)
{
	try {
		if (mKeyCfg.acid_sign_key.find(key_generation) == mKeyCfg.acid_sign_key.end())
		{
			throw tc::Exception("Failed to load rsa public key");
		}

		acid.validateSignature(mKeyCfg.acid_sign_key.at(key_generation));
	}
	catch (tc::Exception& e) {
		fmt::print("[WARNING] ACID Signature: FAIL ({:s})\n", e.error());
	}
	
}

void nstool::MetaProcess::validateAciFromAcid(const pie::hac::AccessControlInfo& aci, const pie::hac::AccessControlInfoDesc& acid)
{
	// check Program ID
	if (acid.getProgramIdRestrict().min > 0 && aci.getProgramId() < acid.getProgramIdRestrict().min)
	{
		fmt::print("[WARNING] ACI ProgramId: FAIL (Outside Legal Range)\n");
	}
	else if (acid.getProgramIdRestrict().max > 0 && aci.getProgramId() > acid.getProgramIdRestrict().max)
	{
		fmt::print("[WARNING] ACI ProgramId: FAIL (Outside Legal Range)\n");
	}

	auto fs_access = aci.getFileSystemAccessControl().getFsAccess();
	auto desc_fs_access = acid.getFileSystemAccessControl().getFsAccess();
	for (size_t i = 0; i < fs_access.size(); i++)
	{
		bool rightFound = false;
		for (size_t j = 0; j < desc_fs_access.size() && rightFound == false; j++)
		{
			if (fs_access[i] == desc_fs_access[j])
				rightFound = true;
		}

		if (rightFound == false)
		{
			fmt::print("[WARNING] ACI/FAC FsaRights: FAIL ({:s} not permitted)\n", pie::hac::FileSystemAccessUtil::getFsAccessFlagAsString(fs_access[i]));
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

			fmt::print("[WARNING] ACI/FAC ContentOwnerId: FAIL (0x{:016x} not permitted)\n", aci.getFileSystemAccessControl().getContentOwnerIdList()[i]);
		}
	}

	// See https://github.com/jakcron/nstool/issues/92
	// Nintendo doesn't populate SaveDataOwnerIdList in ACID, so this field cannot be verified
#if 0
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

			fmt::print("[WARNING] ACI/FAC SaveDataOwnerId: FAIL (0x{:016x} ({:d}) not permitted)\n", aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i].id, (uint32_t)aci.getFileSystemAccessControl().getSaveDataOwnerIdList()[i].access_type);
		}
	}
#endif

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
			fmt::print("[WARNING] ACI/SAC ServiceList: FAIL ({:s}{:s} not permitted)\n", aci.getServiceAccessControl().getServiceList()[i].getName(), (aci.getServiceAccessControl().getServiceList()[i].isServer()? " (Server)" : ""));
		}
	}

	// check KC
	// check thread info
	if (aci.getKernelCapabilities().getThreadInfo().getMaxCpuId() != acid.getKernelCapabilities().getThreadInfo().getMaxCpuId())
	{
		fmt::print("[WARNING] ACI/KC ThreadInfo/MaxCpuId: FAIL ({:d} not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMaxCpuId());
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMinCpuId() != acid.getKernelCapabilities().getThreadInfo().getMinCpuId())
	{
		fmt::print("[WARNING] ACI/KC ThreadInfo/MinCpuId: FAIL ({:d} not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMinCpuId());
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMaxPriority() != acid.getKernelCapabilities().getThreadInfo().getMaxPriority())
	{
		fmt::print("[WARNING] ACI/KC ThreadInfo/MaxPriority: FAIL ({:d} not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMaxPriority());
	}
	if (aci.getKernelCapabilities().getThreadInfo().getMinPriority() != acid.getKernelCapabilities().getThreadInfo().getMinPriority())
	{
		fmt::print("[WARNING] ACI/KC ThreadInfo/MinPriority: FAIL ({:d} not permitted)\n", aci.getKernelCapabilities().getThreadInfo().getMinPriority());
	}
	// check system calls
	auto syscall_ids = aci.getKernelCapabilities().getSystemCalls().getSystemCallIds();
	auto desc_syscall_ids = acid.getKernelCapabilities().getSystemCalls().getSystemCallIds();
	for (size_t i = 0; i < syscall_ids.size(); i++)
	{
		if (syscall_ids.test(i) && desc_syscall_ids.test(i) == false)
		{
			fmt::print("[WARNING] ACI/KC SystemCallList: FAIL ({:s} not permitted)\n", pie::hac::KernelCapabilityUtil::getSystemCallIdAsString(pie::hac::kc::SystemCallId(i)));
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
			auto map = aci.getKernelCapabilities().getMemoryMaps().getMemoryMaps()[i];

			fmt::print("[WARNING] ACI/KC MemoryMap: FAIL ({:s} not permitted)\n", formatMappingAsString(map));
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
			auto map = aci.getKernelCapabilities().getMemoryMaps().getIoMemoryMaps()[i];

			fmt::print("[WARNING] ACI/KC IoMemoryMap: FAIL ({:s} not permitted)\n", formatMappingAsString(map));
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
			fmt::print("[WARNING] ACI/KC InteruptsList: FAIL (0x{:x} not permitted)\n", aci.getKernelCapabilities().getInterupts().getInteruptList()[i]);
		}
	}
	// check misc params
	if (aci.getKernelCapabilities().getMiscParams().getProgramType() != acid.getKernelCapabilities().getMiscParams().getProgramType())
	{
		fmt::print("[WARNING] ACI/KC ProgramType: FAIL ({:d} not permitted)\n", (uint32_t)aci.getKernelCapabilities().getMiscParams().getProgramType());
	}
	// check kernel version
	uint32_t aciKernelVersion = (uint32_t)aci.getKernelCapabilities().getKernelVersion().getVerMajor() << 16 |  (uint32_t)aci.getKernelCapabilities().getKernelVersion().getVerMinor();
	uint32_t acidKernelVersion =  (uint32_t)acid.getKernelCapabilities().getKernelVersion().getVerMajor() << 16 |  (uint32_t)acid.getKernelCapabilities().getKernelVersion().getVerMinor();
	if (aciKernelVersion < acidKernelVersion)
	{
		fmt::print("[WARNING] ACI/KC RequiredKernelVersion: FAIL ({:d}.{:d} not permitted)\n", aci.getKernelCapabilities().getKernelVersion().getVerMajor(), aci.getKernelCapabilities().getKernelVersion().getVerMinor());
	}
	// check handle table size
	if (aci.getKernelCapabilities().getHandleTableSize().getHandleTableSize() > acid.getKernelCapabilities().getHandleTableSize().getHandleTableSize())
	{
		fmt::print("[WARNING] ACI/KC HandleTableSize: FAIL (0x{:x} too large)\n", aci.getKernelCapabilities().getHandleTableSize().getHandleTableSize());
	}
	// check misc flags
	auto misc_flags = aci.getKernelCapabilities().getMiscFlags().getMiscFlags();
	auto desc_misc_flags = acid.getKernelCapabilities().getMiscFlags().getMiscFlags();
	for (size_t i = 0; i < misc_flags.size(); i++)
	{
		if (misc_flags.test(i) && desc_misc_flags.test(i) == false)
		{
			fmt::print("[WARNING] ACI/KC MiscFlag: FAIL ({:s} not permitted)\n", pie::hac::KernelCapabilityUtil::getMiscFlagsBitAsString(pie::hac::kc::MiscFlagsBit(i)));
		}		
	}
}

void nstool::MetaProcess::displayMetaHeader(const pie::hac::Meta& hdr)
{
	fmt::print("[Meta Header]\n");
	fmt::print("  ACID KeyGeneration: {:d}\n", hdr.getAccessControlInfoDescKeyGeneration());
	fmt::print("  Flags:\n");
	fmt::print("    Is64BitInstruction:       {}\n", hdr.getIs64BitInstructionFlag());
	fmt::print("    ProcessAddressSpace:      {:s}\n", pie::hac::MetaUtil::getProcessAddressSpaceAsString(hdr.getProcessAddressSpace()));
	fmt::print("    OptimizeMemoryAllocation: {}\n", hdr.getOptimizeMemoryAllocationFlag());
	fmt::print("  SystemResourceSize: 0x{:x}\n", hdr.getSystemResourceSize());
	fmt::print("  Main Thread Params:\n");
	fmt::print("    Priority:      {:d}\n", hdr.getMainThreadPriority());
	fmt::print("    CpuId:         {:d}\n", hdr.getMainThreadCpuId());
	fmt::print("    StackSize:     0x{:x}\n", hdr.getMainThreadStackSize());
	fmt::print("  TitleInfo:\n");
	fmt::print("    Version:       v{:d}\n", hdr.getVersion());
	fmt::print("    Name:          {:s}\n", hdr.getName());
	if (hdr.getProductCode().length())
	{
		fmt::print("    ProductCode:   {:s}\n", hdr.getProductCode());
	}
}

void nstool::MetaProcess::displayAciHdr(const pie::hac::AccessControlInfo& aci)
{
	fmt::print("[Access Control Info]\n");
	fmt::print("  ProgramID:       0x{:016x}\n", aci.getProgramId());
}

void nstool::MetaProcess::displayAciDescHdr(const pie::hac::AccessControlInfoDesc& acid)
{
	fmt::print("[Access Control Info Desc]\n");
	fmt::print("  Flags:           \n");
	fmt::print("    Production:            {}\n", acid.getProductionFlag());
	fmt::print("    Unqualified Approval:  {}\n", acid.getUnqualifiedApprovalFlag());
	fmt::print("    Memory Region:         {:s} ({:d})\n", pie::hac::AccessControlInfoUtil::getMemoryRegionAsString(acid.getMemoryRegion()), (uint32_t)acid.getMemoryRegion());
	fmt::print("  ProgramID Restriction\n");
	fmt::print("    Min:           0x{:016x}\n", acid.getProgramIdRestrict().min);
	fmt::print("    Max:           0x{:016x}\n", acid.getProgramIdRestrict().max);
}

void nstool::MetaProcess::displayFac(const pie::hac::FileSystemAccessControl& fac)
{
	fmt::print("[FS Access Control]\n");
	fmt::print("  Format Version:  {:d}\n", fac.getFormatVersion());

	if (fac.getFsAccess().size())
	{
		std::vector<std::string> fs_access_str_list;
		for (auto itr = fac.getFsAccess().begin(); itr != fac.getFsAccess().end(); itr++)
		{
			std::string flag_string = pie::hac::FileSystemAccessUtil::getFsAccessFlagAsString(pie::hac::fac::FsAccessFlag(*itr));
			if (mCliOutputMode.show_extended_info)
			{
				fs_access_str_list.push_back(fmt::format("{:s} (bit {:d})", flag_string, (uint32_t)*itr));
			}
			else
			{
				fs_access_str_list.push_back(flag_string);
			}
			
		}

		fmt::print("  FsAccess:\n");
		fmt::print("{:s}", tc::cli::FormatUtil::formatListWithLineLimit(fs_access_str_list, 60, 4));
	}
	
	if (fac.getContentOwnerIdList().size())
	{
		fmt::print("  Content Owner IDs:\n");
		for (size_t i = 0; i < fac.getContentOwnerIdList().size(); i++)
		{
			fmt::print("    0x{:016x}\n", fac.getContentOwnerIdList()[i]);
		}
	}

	if (fac.getSaveDataOwnerIdList().size())
	{
		fmt::print("  Save Data Owner IDs:\n");
		for (size_t i = 0; i < fac.getSaveDataOwnerIdList().size(); i++)
		{
			fmt::print("    0x{:016x} ({:s})\n", fac.getSaveDataOwnerIdList()[i].id, pie::hac::FileSystemAccessUtil::getSaveDataOwnerAccessModeAsString(fac.getSaveDataOwnerIdList()[i].access_type));
		}
	}
}

void nstool::MetaProcess::displaySac(const pie::hac::ServiceAccessControl& sac)
{
	fmt::print("[Service Access Control]\n");
	fmt::print("  Service List:\n");
	std::vector<std::string> service_name_list;
	for (size_t i = 0; i < sac.getServiceList().size(); i++)
	{
		service_name_list.push_back(sac.getServiceList()[i].getName() + (sac.getServiceList()[i].isServer() ? "(isSrv)" : ""));
	}
	fmt::print("{:s}", tc::cli::FormatUtil::formatListWithLineLimit(service_name_list, 60, 4));
}

void nstool::MetaProcess::displayKernelCap(const pie::hac::KernelCapabilityControl& kern)
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

std::string nstool::MetaProcess::formatMappingAsString(const pie::hac::MemoryMappingHandler::sMemoryMapping& map) const
{
	return fmt::format("0x{:016x} - 0x{:016x} (perm={:s}) (type={:s})", ((uint64_t)map.addr << 12), (((uint64_t)(map.addr + map.size) << 12) - 1), pie::hac::KernelCapabilityUtil::getMemoryPermissionAsString(map.perm), pie::hac::KernelCapabilityUtil::getMappingTypeAsString(map.type));
}