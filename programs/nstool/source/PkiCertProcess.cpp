#include <iostream>
#include <iomanip>

#include <fnd/SimpleTextOutput.h>
#include <nn/pki/SignUtils.h>
#include "OffsetAdjustedIFile.h"
#include "PkiCertProcess.h"
#include "PkiValidator.h"

PkiCertProcess::PkiCertProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

PkiCertProcess::~PkiCertProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void PkiCertProcess::process()
{
	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	importCerts();
	if (mVerify)
		validateCerts();

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayCerts();
}

void PkiCertProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void PkiCertProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void PkiCertProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void PkiCertProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void PkiCertProcess::importCerts()
{
	fnd::Vec<byte_t> scratch;

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	nn::pki::SignedData<nn::pki::CertificateBody> cert;
	for (size_t f_pos = 0; f_pos < scratch.size(); f_pos += cert.getBytes().size())
	{
		cert.fromBytes(scratch.data() + f_pos, scratch.size() - f_pos);
		mCert.addElement(cert);
	}
}

void PkiCertProcess::validateCerts()
{
	PkiValidator pki;
	
	try
	{
		pki.setRootKey(mKeyset->pki.root_sign_key);
		pki.addCertificates(mCert);
	}
	catch (const fnd::Exception& e)
	{
		std::cout << "[WARNING] " << e.error() << std::endl;
		return;
	}
}

void PkiCertProcess::displayCerts()
{
	for (size_t i = 0; i < mCert.size(); i++)
	{
		displayCert(mCert[i]);
	}
}

void PkiCertProcess::displayCert(const nn::pki::SignedData<nn::pki::CertificateBody>& cert)
{
#define _SPLIT_VER(ver) ( (ver>>26) & 0x3f), ( (ver>>20) & 0x3f), ( (ver>>16) & 0xf), (ver & 0xffff)
#define _HEXDUMP_U(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02X", var[a__a__A]); } while(0)
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	std::cout << "[NNPKI Certificate]" << std::endl;

	std::cout << "  SignType       " << getSignTypeStr(cert.getSignature().getSignType());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << " (0x" << std::hex << cert.getSignature().getSignType() << ") (" << getEndiannessStr(cert.getSignature().isLittleEndian());
	std::cout << std::endl;

	std::cout << "  Issuer:        " << cert.getBody().getIssuer() << std::endl;
	std::cout << "  Subject:       " << cert.getBody().getSubject() << std::endl;
	std::cout << "  PublicKeyType: " << getPublicKeyTypeStr(cert.getBody().getPublicKeyType());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
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
		std::cout << "    Public Exponent:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().modulus, getHexDumpLen(fnd::rsa::kRsa2048Size), 0x10, 6);
		std::cout << "    Modulus:" << std::endl;
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
	


#undef _HEXDUMP_L
#undef _HEXDUMP_U
#undef _SPLIT_VER
}

size_t PkiCertProcess::getHexDumpLen(size_t max_size) const
{
	return _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? max_size : kSmallHexDumpLen;
}

const char* PkiCertProcess::getSignTypeStr(nn::pki::sign::SignatureId type) const
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

const char* PkiCertProcess::getEndiannessStr(bool isLittleEndian) const
{
	return isLittleEndian ? "LittleEndian" : "BigEndian";
}

const char* PkiCertProcess::getPublicKeyTypeStr(nn::pki::cert::PublicKeyType type) const
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