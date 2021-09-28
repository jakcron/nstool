#pragma once
#include "types.h"
#include "KeyBag.h"

#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/es/TicketBody_V2.h>

namespace nstool {

class EsTikProcess
{
public:
	EsTikProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCertificateChain(const std::vector<nn::pki::SignedData<nn::pki::CertificateBody>>& certs);
	void setCliOutputMode(CliOutputMode mode);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "EsTikProcess";

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	std::vector<nn::pki::SignedData<nn::pki::CertificateBody>> mCerts;

	nn::pki::SignedData<nn::es::TicketBody_V2> mTik;

	void importTicket();
	void verifyTicket();
	void displayTicket();
	const char* getSignTypeStr(uint32_t type) const;
	const char* getTitleKeyPersonalisationStr(byte_t flag) const;
	const char* getLicenseTypeStr(byte_t flag) const;
	const char* getPropertyFlagStr(byte_t flag) const;
};

}