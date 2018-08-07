#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/Vec.h>
#include <fnd/rsa.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <string>

class PkiValidator
{
public:
	PkiValidator();

	void setRootKey(const fnd::rsa::sRsa4096Key& root_key);
	void addCertificates(const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& certs);
	void addCertificate(const nn::pki::SignedData<nn::pki::CertificateBody>& cert);
	void clearCertificates();

	void validateSignature(const std::string& issuer, nn::pki::sign::SignatureId signature_id, const fnd::Vec<byte_t>& signature, const fnd::Vec<byte_t>& hash) const;

private:
	const std::string kModuleName = "NNPkiValidator";


	fnd::rsa::sRsa4096Key mRootKey;
	fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>> mCertificateBank;

	void makeCertIdent(const nn::pki::SignedData<nn::pki::CertificateBody>& cert, std::string& ident) const;
	void makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const;
	bool doesCertExist(const std::string& ident) const;
	const nn::pki::SignedData<nn::pki::CertificateBody>& getCert(const std::string& ident) const;
	fnd::sha::HashType getCryptoHashAlgoFromEsSignHashAlgo(nn::pki::sign::HashAlgo hash_algo) const;
};