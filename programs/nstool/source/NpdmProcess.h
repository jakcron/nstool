#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/SimpleFile.h>
#include <nx/NpdmBinary.h>

#include "nstool.h"

class NpdmProcess
{
public:
	NpdmProcess();

	void process();

	void setInputFile(fnd::IFile& reader);
	void setInputFileOffset(size_t offset);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "NpdmProcess";

	fnd::IFile* mReader;
	size_t mOffset;
	const sKeyset* mKeyset;
	CliOutputType mCliOutputType;
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