#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/MetaBinary.h>
#include "KeyConfiguration.h"

#include "common.h"

class MetaProcess
{
public:
	MetaProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setKeyCfg(const KeyConfiguration& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::MetaBinary& getMetaBinary() const;

private:
	const std::string kModuleName = "MetaProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	KeyConfiguration mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::MetaBinary mMeta;

	void importMeta();

	void validateAcidSignature(const nn::hac::AccessControlInfoDesc& acid);
	void validateAciFromAcid(const nn::hac::AccessControlInfo& aci, const nn::hac::AccessControlInfoDesc& acid);

	void displayMetaHeader(const nn::hac::MetaBinary& hdr);
	void displayAciHdr(const nn::hac::AccessControlInfo& aci);
	void displayAciDescHdr(const nn::hac::AccessControlInfoDesc& aci);
	void displayFac(const nn::hac::FileSystemAccessControlBinary& fac);
	void displaySac(const nn::hac::ServiceAccessControlBinary& sac);
	void displayKernelCap(const nn::hac::KernelCapabilityBinary& kern);

	const char* getInstructionTypeStr(nn::hac::meta::InstructionType type) const;
	const char* getProcAddressSpaceTypeStr(nn::hac::meta::ProcAddrSpaceType type) const;
	const char* getAcidFlagStr(nn::hac::aci::Flag flag) const;
	const char* getMiscFlagStr(nn::hac::MiscFlagsHandler::Flags flag) const;
	const char* getFsaRightStr(nn::hac::fac::FsAccessFlag flag) const;
	const char* getSaveDataOwnerAccessModeStr(nn::hac::fac::SaveDataOwnerIdAccessType type) const;
	const char* getSystemCallStr(byte_t syscall_id) const;
	const char* getMemMapPermStr(nn::hac::MemoryMappingHandler::MemoryPerm type) const;
	const char* getMemMapTypeStr(nn::hac::MemoryMappingHandler::MappingType type) const;
};