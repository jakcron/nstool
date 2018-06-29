#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <nx/NpdmBinary.h>

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

	const nx::NpdmBinary& getNpdmBinary() const;

private:
	const std::string kModuleName = "NpdmProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
	const sKeyset* mKeyset;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	nx::NpdmBinary mNpdm;

	void validateAcidSignature(const nx::AccessControlInfoDescBinary& acid);
	void validateAciFromAcid(const nx::AccessControlInfoBinary& aci, const nx::AccessControlInfoDescBinary& acid);

	void displayNpdmHeader(const nx::NpdmBinary& hdr);
	void displayAciHdr(const nx::AccessControlInfoBinary& aci);
	void displayAciDescHdr(const nx::AccessControlInfoDescBinary& aci);
	void displayFac(const nx::FileSystemAccessControlBinary& fac);
	void displaySac(const nx::ServiceAccessControlBinary& sac);
	void displayKernelCap(const nx::KernelCapabilityBinary& kern);
};