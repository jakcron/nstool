#pragma once
#include "types.h"
#include "KeyBag.h"

#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>

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

	std::vector<nn::pki::SignedData<nn::pki::CertificateBody>> mCert;

	void importCerts();
	void validateCerts();
	void displayCerts();
	void displayCert(const nn::pki::SignedData<nn::pki::CertificateBody>& cert);

	std::string getSignTypeStr(nn::pki::sign::SignatureId type) const;
	std::string getEndiannessStr(bool isLittleEndian) const;
	std::string getPublicKeyTypeStr(nn::pki::cert::PublicKeyType type) const;
};

}