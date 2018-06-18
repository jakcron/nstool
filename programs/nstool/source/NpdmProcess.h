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

	void validateAcidSignature(const nx::AcidBinary& acid);
	void validateAciFromAcid(const nx::AciBinary& aci, const nx::AcidBinary& acid);

	void displayNpdmHeader(const nx::NpdmHeader& hdr);
	void displayAciHdr(const nx::AciHeader& aci);
	void displayFac(const nx::FacBinary& fac);
	void displaySac(const nx::SacBinary& sac);
	void displayKernelCap(const nx::KcBinary& kern);
};