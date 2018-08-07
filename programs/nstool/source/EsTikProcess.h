#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/Vec.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/es/TicketBody_V2.h>
#include "nstool.h"

class EsTikProcess
{
public:
	EsTikProcess();
	~EsTikProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setKeyset(const sKeyset* keyset);
	void setCertificateChain(const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& certs);
	void setCliOutputMode(CliOutputMode mode);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "EsTikProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
	const sKeyset* mKeyset;
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