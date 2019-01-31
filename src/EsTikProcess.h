#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <fnd/Vec.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/es/TicketBody_V2.h>
#include "KeyConfiguration.h"
#include "common.h"

class EsTikProcess
{
public:
	EsTikProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setKeyCfg(const KeyConfiguration& keycfg);
	void setCertificateChain(const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& certs);
	void setCliOutputMode(CliOutputMode mode);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "EsTikProcess";

	fnd::SharedPtr<fnd::IFile> mFile;
	KeyConfiguration mKeyCfg;
	CliOutputMode mCliOutputMode;
	bool mVerify;
	
	fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>> mCerts;

	nn::pki::SignedData<nn::es::TicketBody_V2> mTik;

	void importTicket();
	void verifyTicket();
	void displayTicket();
	const char* getSignTypeStr(uint32_t type) const;
	const char* getTitleKeyPersonalisationStr(byte_t flag) const;
	const char* getLicenseTypeStr(byte_t flag) const;
	const char* getPropertyFlagStr(byte_t flag) const;
};