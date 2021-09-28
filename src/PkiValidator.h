#pragma once
#include "types.h"
#include "KeyBag.h"

#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>

namespace nstool {

class PkiValidator
{
public:
	PkiValidator();

	void setKeyCfg(const KeyBag& keycfg);
	void addCertificates(const std::vector<nn::pki::SignedData<nn::pki::CertificateBody>>& certs);
	void addCertificate(const nn::pki::SignedData<nn::pki::CertificateBody>& cert);
	void clearCertificates();

	void validateSignature(const std::string& issuer, nn::pki::sign::SignatureId signature_id, const tc::ByteData& signature, const tc::ByteData& hash) const;

private:
	const std::string kModuleName = "NNPkiValidator";

	KeyBag mKeyCfg;
	std::vector<nn::pki::SignedData<nn::pki::CertificateBody>> mCertificateBank;

	void makeCertIdent(const nn::pki::SignedData<nn::pki::CertificateBody>& cert, std::string& ident) const;
	void makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const;
	bool doesCertExist(const std::string& ident) const;
	const nn::pki::SignedData<nn::pki::CertificateBody>& getCert(const std::string& ident) const;
	fnd::sha::HashType getCryptoHashAlgoFromEsSignHashAlgo(nn::pki::sign::HashAlgo hash_algo) const;
};

}