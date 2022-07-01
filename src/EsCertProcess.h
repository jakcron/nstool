#pragma once
#include "types.h"
#include "KeyBag.h"

#include <pietendo/hac/es/SignedData.h>
#include <pietendo/hac/es/CertificateBody.h>

namespace nstool {

class EsCertProcess
{
public:
	EsCertProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>> mCert;

	void importCerts();
	void validateCerts();
	void displayCerts();
	void displayCert(const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& cert);

	std::string getSignTypeStr(pie::hac::es::sign::SignatureId type) const;
	std::string getEndiannessStr(bool isLittleEndian) const;
	std::string getPublicKeyTypeStr(pie::hac::es::cert::PublicKeyType type) const;
};

}