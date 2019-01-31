#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <nn/pki/SignUtils.h>
#include "EsTikProcess.h"
#include "PkiValidator.h"



EsTikProcess::EsTikProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void EsTikProcess::process()
{
	importTicket();

	if (mVerify)
		verifyTicket();

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayTicket();
}

void EsTikProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void EsTikProcess::setKeyCfg(const KeyConfiguration& keycfg)
{
	mKeyCfg = keycfg;
}

void EsTikProcess::setCertificateChain(const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& certs)
{
	mCerts = certs;
}

void EsTikProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void EsTikProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void EsTikProcess::importTicket()
{
	fnd::Vec<byte_t> scratch;


	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());
	mTik.fromBytes(scratch.data(), scratch.size());
}

void EsTikProcess::verifyTicket()
{
	PkiValidator pki_validator;
	fnd::Vec<byte_t> tik_hash;

	switch (nn::pki::sign::getHashAlgo(mTik.getSignature().getSignType()))
	{
	case (nn::pki::sign::HASH_ALGO_SHA1):
		tik_hash.alloc(fnd::sha::kSha1HashLen);
		fnd::sha::Sha1(mTik.getBody().getBytes().data(), mTik.getBody().getBytes().size(), tik_hash.data());
		break;
	case (nn::pki::sign::HASH_ALGO_SHA256):
		tik_hash.alloc(fnd::sha::kSha256HashLen);
		fnd::sha::Sha256(mTik.getBody().getBytes().data(), mTik.getBody().getBytes().size(), tik_hash.data());
		break;
	}

	try 
	{
		pki_validator.setKeyCfg(mKeyCfg);
		pki_validator.addCertificates(mCerts);
		pki_validator.validateSignature(mTik.getBody().getIssuer(), mTik.getSignature().getSignType(), mTik.getSignature().getSignature(), tik_hash);
	}
	catch (const fnd::Exception& e)
	{
		std::cout << "[WARNING] Ticket signature could not be validated (" << e.error() << ")" << std::endl;
	}
}

void EsTikProcess::displayTicket()
{
#define _SPLIT_VER(ver) (uint32_t)((ver>>10) & 0x3f) << "." << (uint32_t)((ver>>4) & 0x3f) << "." << (uint32_t)((ver>>0) & 0xf)

	const nn::es::TicketBody_V2& body = mTik.getBody();	

	std::cout << "[ES Ticket]" << std::endl;

	std::cout << "  SignType:         " << getSignTypeStr(mTik.getSignature().getSignType());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << " (0x" << std::hex << mTik.getSignature().getSignType() << ")";
	std::cout << std::endl;

	std::cout << "  Issuer:           " << body.getIssuer() << std::endl;
	std::cout << "  Title Key:" << std::endl;
	std::cout << "    EncMode:        " << getTitleKeyPersonalisationStr(body.getTitleKeyEncType()) << std::endl;
	std::cout << "    KeyGeneration:  " << std::dec << (uint32_t)body.getCommonKeyId() << std::endl;
	if (body.getTitleKeyEncType() == nn::es::ticket::RSA2048)
	{
		std::cout << "    Data:" << std::endl;
		for (size_t i = 0; i < 0x10; i++)
			std::cout << "      " << fnd::SimpleTextOutput::arrayToString(body.getEncTitleKey() + 0x10*i, 0x10, true, ":") << std::endl;
	}
	else if (body.getTitleKeyEncType() == nn::es::ticket::AES128_CBC)
	{
		std::cout << "    Data:" << std::endl;
		std::cout << "      " << fnd::SimpleTextOutput::arrayToString(body.getEncTitleKey(), 0x10, true, ":") << std::endl;
	}
	else
	{
		std::cout << "    Data:           <cannot display>" << std::endl;
	}

	std::cout << "  Version:          v" << _SPLIT_VER(body.getTicketVersion());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << " (" << (uint32_t)body.getTicketVersion() << ")";
	std::cout << std::endl;
	
	std::cout << "  License Type:     " << getLicenseTypeStr(body.getLicenseType()) << std::endl; 
	
	if (body.getPropertyFlags().size() > 0)
	{
		std::cout << "  Flags:" << std::endl;
		for (size_t i = 0; i < body.getPropertyFlags().size(); i++)
		{
			std::cout << "    " << getPropertyFlagStr(body.getPropertyFlags()[i]) << std::endl;
		}
	}
	
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Reserved Region:" << std::endl;
		fnd::SimpleTextOutput::hexDump(body.getReservedRegion(), 8, 0x10, 4);
	}
	
	if (body.getTicketId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << "  TicketId:         0x" << std::hex << std::setw(16) << std::setfill('0') << body.getTicketId() << std::endl;
	
	if (body.getDeviceId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << "  DeviceId:         0x" << std::hex << std::setw(16) << std::setfill('0') << body.getDeviceId() << std::endl;
	
	std::cout << "  RightsId:         " <<  std::endl << "    ";
	fnd::SimpleTextOutput::hexDump(body.getRightsId(), 16);

	std::cout << "  SectionTotalSize:       0x" << std::hex << body.getSectionTotalSize() << std::endl;
	std::cout << "  SectionHeaderOffset:    0x" << std::hex << body.getSectionHeaderOffset() << std::endl;
	std::cout << "  SectionNum:             0x" << std::hex << body.getSectionNum() << std::endl;
	std::cout << "  SectionEntrySize:       0x" << std::hex << body.getSectionEntrySize() << std::endl;

#undef _SPLIT_VER
}

const char* EsTikProcess::getSignTypeStr(uint32_t type) const
{
	const char* str = nullptr;
	switch(type)
	{
	case (nn::pki::sign::SIGN_ID_RSA4096_SHA1):
		str = "RSA4096-SHA1";
		break;
	case (nn::pki::sign::SIGN_ID_RSA2048_SHA1):
		str = "RSA2048-SHA1";
		break;
	case (nn::pki::sign::SIGN_ID_ECDSA240_SHA1):
		str = "ECDSA240-SHA1";
		break;
	case (nn::pki::sign::SIGN_ID_RSA4096_SHA256):
		str = "RSA4096-SHA256";
		break;
	case (nn::pki::sign::SIGN_ID_RSA2048_SHA256):
		str = "RSA2048-SHA256";
		break;
	case (nn::pki::sign::SIGN_ID_ECDSA240_SHA256):
		str = "ECDSA240-SHA256";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getTitleKeyPersonalisationStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (nn::es::ticket::AES128_CBC):
		str = "Generic (AESCBC)";
		break;
	case (nn::es::ticket::RSA2048):
		str = "Personalised (RSA2048)";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getLicenseTypeStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (nn::es::ticket::LICENSE_PERMANENT):
		str = "Permanent";
		break;
	case (nn::es::ticket::LICENSE_DEMO):
		str = "Demo";
		break;
	case (nn::es::ticket::LICENSE_TRIAL):
		str = "Trial";
		break;
	case (nn::es::ticket::LICENSE_RENTAL):
		str = "Rental";
		break;
	case (nn::es::ticket::LICENSE_SUBSCRIPTION):
		str = "Subscription";
		break;
	case (nn::es::ticket::LICENSE_SERVICE):
		str = "Service";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getPropertyFlagStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (nn::es::ticket::FLAG_PRE_INSTALL):
		str = "PreInstall";
		break;
	case (nn::es::ticket::FLAG_SHARED_TITLE):
		str = "SharedTitle";
		break;
	case (nn::es::ticket::FLAG_ALLOW_ALL_CONTENT):
		str = "AllContent";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}