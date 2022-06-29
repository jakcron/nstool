#pragma once
#include "types.h"
#include "KeyBag.h"

#include <pietendo/hac/es/SignedData.h>
#include <pietendo/hac/es/CertificateBody.h>
#include <pietendo/hac/es/TicketBody_V2.h>

namespace nstool {

class EsTikProcess
{
public:
	EsTikProcess();

	void process();

	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setKeyCfg(const KeyBag& keycfg);
	void setCertificateChain(const std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>>& certs);
	void setCliOutputMode(CliOutputMode mode);
	void setVerifyMode(bool verify);
private:
	std::string mModuleName;

	std::shared_ptr<tc::io::IStream> mFile;
	KeyBag mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>> mCerts;

	pie::hac::es::SignedData<pie::hac::es::TicketBody_V2> mTik;

	void importTicket();
	void verifyTicket();
	void displayTicket();
	std::string getSignTypeStr(uint32_t type) const;
	std::string getTitleKeyPersonalisationStr(byte_t flag) const;
	std::string getLicenseTypeStr(byte_t flag) const;
	std::string getPropertyFlagStr(byte_t flag) const;
	std::string getTitleVersionStr(uint16_t version) const;
};

}