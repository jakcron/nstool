#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <nn/hac/Meta.h>
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

	const nn::hac::Meta& getMeta() const;

private:
	const std::string kModuleName = "MetaProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	KeyConfiguration mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::Meta mMeta;

	void importMeta();

	void validateAcidSignature(const nn::hac::AccessControlInfoDesc& acid);
	void validateAciFromAcid(const nn::hac::AccessControlInfo& aci, const nn::hac::AccessControlInfoDesc& acid);

	void displayMetaHeader(const nn::hac::Meta& hdr);
	void displayAciHdr(const nn::hac::AccessControlInfo& aci);
	void displayAciDescHdr(const nn::hac::AccessControlInfoDesc& aci);
	void displayFac(const nn::hac::FileSystemAccessControl& fac);
	void displaySac(const nn::hac::ServiceAccessControl& sac);
	void displayKernelCap(const nn::hac::KernelCapabilityControl& kern);
};