#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/Vec.h>
#include <crypto/rsa.h>
#include <es/SignedData.h>
#include <es/CertificateBody.h>
#include <string>

class PkiValidator
{
public:
	PkiValidator();

	void setRootKey(const crypto::rsa::sRsa4096Key& root_key);
	void addCertificates(const fnd::List<es::SignedData<es::CertificateBody>>& certs);
	void clearCertificates();

	void validateSignature(const std::string& issuer, es::sign::SignatureId signature_id, const fnd::Vec<byte_t>& signature, const fnd::Vec<byte_t>& hash) const;

private:
	const std::string kModuleName = "NNPkiValidator";


	crypto::rsa::sRsa4096Key mRootKey;
	fnd::List<es::SignedData<es::CertificateBody>> mCertificateBank;

	void makeCertIdent(const es::SignedData<es::CertificateBody>& cert, std::string& ident) const;
	void makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const;
	bool doesCertExist(const std::string& ident) const;
	const es::SignedData<es::CertificateBody>& getCert(const std::string& ident) const;
	crypto::sha::HashType getCryptoHashAlgoFromEsSignHashAlgo(es::sign::HashAlgo hash_algo) const;
};