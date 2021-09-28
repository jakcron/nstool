#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include <nn/pki/SignUtils.h>
#include "PkiCertProcess.h"
#include "PkiValidator.h"

nstool::PkiCertProcess::PkiCertProcess() :
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::PkiCertProcess::process()
{
	importCerts();

	if (mVerify)
		validateCerts();

	if (mCliOutputMode.show_basic_info)
		displayCerts();
}

void nstool::PkiCertProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::PkiCertProcess::setKeyCfg(const KeyBag& keycfg)
{
	mKeyCfg = keycfg;
}

void nstool::PkiCertProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void nstool::PkiCertProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::PkiCertProcess::importCerts()
{
	tc::ByteData scratch;

	if (*mFile == nullptr)
	{
		throw tc::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());

	nn::pki::SignedData<nn::pki::CertificateBody> cert;
	for (size_t f_pos = 0; f_pos < scratch.size(); f_pos += cert.getBytes().size())
	{
		cert.fromBytes(scratch.data() + f_pos, scratch.size() - f_pos);
		mCert.push_back(cert);
	}
}

void nstool::PkiCertProcess::validateCerts()
{
	PkiValidator pki;
	
	try
	{
		pki.setKeyCfg(mKeyCfg);
		pki.addCertificates(mCert);
	}
	catch (const tc::Exception& e)
	{
		std::cout << "[WARNING] " << e.error() << std::endl;
		return;
	}
}

void nstool::PkiCertProcess::displayCerts()
{
	for (size_t i = 0; i < mCert.size(); i++)
	{
		displayCert(mCert[i]);
	}
}

void nstool::PkiCertProcess::displayCert(const nn::pki::SignedData<nn::pki::CertificateBody>& cert)
{
	std::cout << "[NNPKI Certificate]" << std::endl;

	std::cout << "  SignType       " << getSignTypeStr(cert.getSignature().getSignType());
	if (mCliOutputMode.show_extended_info)
		std::cout << " (0x" << std::hex << cert.getSignature().getSignType() << ") (" << getEndiannessStr(cert.getSignature().isLittleEndian()) << ")";
	std::cout << std::endl;

	std::cout << "  Issuer:        " << cert.getBody().getIssuer() << std::endl;
	std::cout << "  Subject:       " << cert.getBody().getSubject() << std::endl;
	std::cout << "  PublicKeyType: " << getPublicKeyTypeStr(cert.getBody().getPublicKeyType());
	if (mCliOutputMode.show_extended_info)
		std::cout << " (" << std::dec << cert.getBody().getPublicKeyType() << ")";
	std::cout << std::endl;
	std::cout << "  CertID:        0x" << std::hex << cert.getBody().getCertId() << std::endl;
	
	if (cert.getBody().getPublicKeyType() == nn::pki::cert::RSA4096)
	{
		std::cout << "  PublicKey:" << std::endl;
		std::cout << "    Modulus:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa4098PublicKey().modulus, getHexDumpLen(fnd::rsa::kRsa4096Size), 0x10, 6);
		std::cout << "    Public Exponent:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa4098PublicKey().public_exponent, fnd::rsa::kRsaPublicExponentSize, 0x10, 6);
	}
	else if (cert.getBody().getPublicKeyType() == nn::pki::cert::RSA2048)
	{
		std::cout << "  PublicKey:" << std::endl;
		std::cout << "    Modulus:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().modulus, getHexDumpLen(fnd::rsa::kRsa2048Size), 0x10, 6);
		std::cout << "    Public Exponent:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().public_exponent, fnd::rsa::kRsaPublicExponentSize, 0x10, 6);
	}
	else if (cert.getBody().getPublicKeyType() == nn::pki::cert::ECDSA240)
	{
		std::cout << "  PublicKey:" << std::endl;
		std::cout << "    R:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getEcdsa240PublicKey().r, getHexDumpLen(fnd::ecdsa::kEcdsa240Size), 0x10, 6);
		std::cout << "    S:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getEcdsa240PublicKey().s, getHexDumpLen(fnd::ecdsa::kEcdsa240Size), 0x10, 6);
	}
}

size_t nstool::PkiCertProcess::getHexDumpLen(size_t max_size) const
{
	return mCliOutputMode.show_extended_info ? max_size : kSmallHexDumpLen;
}

const char* nstool::PkiCertProcess::getSignTypeStr(nn::pki::sign::SignatureId type) const
{
	const char* str;
	switch (type)
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

const char* nstool::PkiCertProcess::getEndiannessStr(bool isLittleEndian) const
{
	return isLittleEndian ? "LittleEndian" : "BigEndian";
}

const char* nstool::PkiCertProcess::getPublicKeyTypeStr(nn::pki::cert::PublicKeyType type) const
{
	const char* str;
	switch (type)
	{
	case (nn::pki::cert::RSA4096):
		str = "RSA4096";
		break;
	case (nn::pki::cert::RSA2048):
		str = "RSA2048";
		break;
	case (nn::pki::cert::ECDSA240):
		str = "ECDSA240";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}