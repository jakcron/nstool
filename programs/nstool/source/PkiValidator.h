#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/Vec.h>
#include <crypto/rsa.h>
#include <pki/SignedData.h>
#include <pki/CertificateBody.h>
#include <string>

class PkiValidator
{
public:
	PkiValidator();

	void setRootKey(const crypto::rsa::sRsa4096Key& root_key);
	void addCertificates(const fnd::List<pki::SignedData<pki::CertificateBody>>& certs);
	void addCertificate(const pki::SignedData<pki::CertificateBody>& cert);
	void clearCertificates();

	void validateSignature(const std::string& issuer, pki::sign::SignatureId signature_id, const fnd::Vec<byte_t>& signature, const fnd::Vec<byte_t>& hash) const;

private:
	const std::string kModuleName = "NNPkiValidator";


	crypto::rsa::sRsa4096Key mRootKey;
	fnd::List<pki::SignedData<pki::CertificateBody>> mCertificateBank;

	void makeCertIdent(const pki::SignedData<pki::CertificateBody>& cert, std::string& ident) const;
	void makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const;
	bool doesCertExist(const std::string& ident) const;
	const pki::SignedData<pki::CertificateBody>& getCert(const std::string& ident) const;
	crypto::sha::HashType getCryptoHashAlgoFromEsSignHashAlgo(pki::sign::HashAlgo hash_algo) const;
};