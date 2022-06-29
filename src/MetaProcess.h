#pragma once
#include "types.h"
#include "KeyBag.h"

#include <pietendo/hac/Meta.h>

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

	const pie::hac::Meta& getMeta() const;

private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	pie::hac::Meta mMeta;

	void importMeta();

	void validateAcidSignature(const pie::hac::AccessControlInfoDesc& acid, byte_t key_generation);
	void validateAciFromAcid(const pie::hac::AccessControlInfo& aci, const pie::hac::AccessControlInfoDesc& acid);

	void displayMetaHeader(const pie::hac::Meta& hdr);
	void displayAciHdr(const pie::hac::AccessControlInfo& aci);
	void displayAciDescHdr(const pie::hac::AccessControlInfoDesc& aci);
	void displayFac(const pie::hac::FileSystemAccessControl& fac);
	void displaySac(const pie::hac::ServiceAccessControl& sac);
	void displayKernelCap(const pie::hac::KernelCapabilityControl& kern);

	std::string formatMappingAsString(const pie::hac::MemoryMappingHandler::sMemoryMapping& map) const;
};

}