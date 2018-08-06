#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/List.h>
#include <fnd/Vec.h>
#include <pki/SignedData.h>
#include <pki/CertificateBody.h>
#include "nstool.h"

class EsCertProcess
{
public:
	EsCertProcess();
	~EsCertProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "EsCertProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
	const sKeyset* mKeyset;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	fnd::List<pki::SignedData<pki::CertificateBody>> mCert;

	void importCerts();
	void validateCerts();
	void displayCerts();
	void displayCert(const pki::SignedData<pki::CertificateBody>& cert);


	const char* getSignTypeStr(pki::sign::SignatureId type) const;
	const char* getEndiannessStr(bool isLittleEndian) const;
	const char* getPublicKeyTypeStr(pki::cert::PublicKeyType type) const;
};