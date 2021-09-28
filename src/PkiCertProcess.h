#pragma once
#include "types.h"
#include "KeyBag.h"

#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>

namespace nstool {

class PkiCertProcess
{
public:
	PkiCertProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "PkiCertProcess";
	static const size_t kSmallHexDumpLen = 0x10;

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	std::vector<nn::pki::SignedData<nn::pki::CertificateBody>> mCert;

	void importCerts();
	void validateCerts();
	void displayCerts();
	void displayCert(const nn::pki::SignedData<nn::pki::CertificateBody>& cert);

	size_t getHexDumpLen(size_t max_size) const;
	const char* getSignTypeStr(nn::pki::sign::SignatureId type) const;
	const char* getEndiannessStr(bool isLittleEndian) const;
	const char* getPublicKeyTypeStr(nn::pki::cert::PublicKeyType type) const;
};

}