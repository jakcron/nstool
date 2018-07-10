#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/Vec.h>
#include <es/SignedData.h>
#include <es/TicketBody_V2.h>
#include "nstool.h"

class EsTikProcess
{
public:
	EsTikProcess();
	~EsTikProcess();

	void process();

	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputMode mode);
	void setVerifyMode(bool verify);

private:
	const std::string kModuleName = "EsTikProcess";

	fnd::IFile* mFile;
	bool mOwnIFile;
	const sKeyset* mKeyset;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	es::SignedData<es::TicketBody_V2> mTik;

	void displayTicket();
	const char* getTitleKeyPersonalisationStr(byte_t flag) const;
	const char* getLicenseTypeStr(byte_t flag) const;
	const char* getPropertyFlagStr(byte_t flag) const;
};