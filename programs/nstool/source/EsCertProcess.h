#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/List.h>
#include <fnd/Vec.h>
#include <es/SignedData.h>
#include <es/CertificateBody.h>
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

	fnd::List<es::SignedData<es::CertificateBody>> mCert;

	void importCerts();
	void validateCerts();
	void validateCert(const es::SignedData<es::CertificateBody>& cert);
	void displayCerts();
	void displayCert(const es::SignedData<es::CertificateBody>& cert);

	const es::SignedData<es::CertificateBody>& getIssuerCert(const std::string& issuer_name) const;

	crypto::sha::HashType getCryptoHashAlgoFromEsSignHashAlgo(es::sign::HashAlgo hash_algo) const;

	const char* getSignTypeStr(es::sign::SignatureId type) const;
	const char* getEndiannessStr(bool isLittleEndian) const;
	const char* getPublicKeyTypeStr(es::cert::PublicKeyType type) const;
};