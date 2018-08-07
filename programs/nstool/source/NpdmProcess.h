#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nn/hac/NpdmBinary.h>

#include "nstool.h"

class NpdmProcess
{
public:
	NpdmProcess();
	~NpdmProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	const nn::hac::NpdmBinary& getNpdmBinary() const;

private:
	const std::string kModuleName = "NpdmProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
	const sKeyset* mKeyset;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nn::hac::NpdmBinary mNpdm;

	void validateAcidSignature(const nn::hac::AccessControlInfoDescBinary& acid);
	void validateAciFromAcid(const nn::hac::AccessControlInfoBinary& aci, const nn::hac::AccessControlInfoDescBinary& acid);

	void displayNpdmHeader(const nn::hac::NpdmBinary& hdr);
	void displayAciHdr(const nn::hac::AccessControlInfoBinary& aci);
	void displayAciDescHdr(const nn::hac::AccessControlInfoDescBinary& aci);
	void displayFac(const nn::hac::FileSystemAccessControlBinary& fac);
	void displaySac(const nn::hac::ServiceAccessControlBinary& sac);
	void displayKernelCap(const nn::hac::KernelCapabilityBinary& kern);
};