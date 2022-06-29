#pragma once
#include "types.h"
#include "KeyBag.h"

#include <pietendo/hac/es/SignedData.h>
#include <pietendo/hac/es/CertificateBody.h>

namespace nstool {

class PkiValidator
{
public:
	PkiValidator();

	void setKeyCfg(const KeyBag& keycfg);
	void addCertificates(const std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>>& certs);
	void addCertificate(const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& cert);
	void clearCertificates();

	void validateSignature(const std::string& issuer, pie::hac::es::sign::SignatureId signature_id, const tc::ByteData& signature, const tc::ByteData& hash) const;

private:
	std::string mModuleName;

	KeyBag mKeyCfg;
	std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>> mCertificateBank;

	void makeCertIdent(const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& cert, std::string& ident) const;
	void makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const;
	bool doesCertExist(const std::string& ident) const;
	const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& getCert(const std::string& ident) const;
};

}