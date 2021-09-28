#pragma once
#include "types.h"
#include "KeyBag.h"

#include <nn/hac/Meta.h>

namespace nstool {

class MetaProcess
{
public:
	MetaProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::Meta& getMeta() const;

private:
	const std::string kModuleName = "MetaProcess";

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::Meta mMeta;

	void importMeta();

	void validateAcidSignature(const nn::hac::AccessControlInfoDesc& acid, byte_t key_generation);
	void validateAciFromAcid(const nn::hac::AccessControlInfo& aci, const nn::hac::AccessControlInfoDesc& acid);

	void displayMetaHeader(const nn::hac::Meta& hdr);
	void displayAciHdr(const nn::hac::AccessControlInfo& aci);
	void displayAciDescHdr(const nn::hac::AccessControlInfoDesc& aci);
	void displayFac(const nn::hac::FileSystemAccessControl& fac);
	void displaySac(const nn::hac::ServiceAccessControl& sac);
	void displayKernelCap(const nn::hac::KernelCapabilityControl& kern);
};

}