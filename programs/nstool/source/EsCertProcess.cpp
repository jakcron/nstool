#include <fnd/SimpleTextOutput.h>
#include "OffsetAdjustedIFile.h"
#include "EsCertProcess.h"

EsCertProcess::EsCertProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

EsCertProcess::~EsCertProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void EsCertProcess::process()
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

void EsCertProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void EsCertProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void EsCertProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void EsCertProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void EsCertProcess::importCerts()
{
	fnd::Vec<byte_t> scratch;

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	es::SignedData<es::CertificateBody> cert;
	for (size_t f_pos = 0; f_pos < scratch.size(); f_pos += cert.getBytes().size())
	{
		cert.fromBytes(scratch.data() + f_pos, scratch.size() - f_pos);
		mCert.addElement(cert);
	}
}

void EsCertProcess::validateCerts()
{
	for (size_t i = 0; i < mCert.size(); i++)
	{
		EsCertProcess::validateCert(mCert[i]);
	}
}

void EsCertProcess::validateCert(const es::SignedData<es::CertificateBody>& cert)
{
	std::string cert_ident = "";
	cert_ident += cert.getBody().getSubject();
	cert_ident += cert.getBody().getSubject();
	cert_ident += cert.getBody().getSubject();

	try 
	{
		if (cert.getBody().getIssuer() == es::sign::kRootIssuerStr)
		{
			throw fnd::Exception(kModuleName, "Signed by Root");
		}

		const es::CertificateBody& issuer = getIssuerCert(cert.getBody().getIssuer()).getBody();

		if (issuer.getPublicKeyType() == es::cert::RSA4096 && (cert.getSignature().getSignType() == es::sign::SIGN_RSA4096_SHA1 || cert.getSignature().getSignType() == es::sign::SIGN_RSA4096_SHA256))
		{
			throw fnd::Exception(kModuleName, "RSA4096 signatures are not supported");
		}
		else if (issuer.getPublicKeyType() == es::cert::RSA2048 && (cert.getSignature().getSignType() == es::sign::SIGN_RSA2048_SHA1 || cert.getSignature().getSignType() == es::sign::SIGN_RSA2048_SHA256))
		{
			throw fnd::Exception(kModuleName, "RSA2048 signatures are not supported");
		}
		else if (issuer.getPublicKeyType() == es::cert::ECDSA240 && (cert.getSignature().getSignType() == es::sign::SIGN_ECDSA240_SHA1 || cert.getSignature().getSignType() == es::sign::SIGN_ECDSA240_SHA256))
		{
			throw fnd::Exception(kModuleName, "ECDSA signatures are not supported");
		}
		else
		{
			throw fnd::Exception(kModuleName, "Mismatch between issuer public key and signature type");
		}
	}
	catch (const fnd::Exception& e) 
	{
		printf("[WARNING] Failed to validate %s (%s)\n", cert_ident.c_str(), e.error());
		return;
	}
	
}

void EsCertProcess::displayCerts()
{
	for (size_t i = 0; i < mCert.size(); i++)
	{
		displayCert(mCert[i]);
	}
}

void EsCertProcess::displayCert(const es::SignedData<es::CertificateBody>& cert)
{
#define _SPLIT_VER(ver) ( (ver>>26) & 0x3f), ( (ver>>20) & 0x3f), ( (ver>>16) & 0xf), (ver & 0xffff)
#define _HEXDUMP_U(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02X", var[a__a__A]); } while(0)
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	printf("[ES Certificate]\n");
	printf("  SignType:       %s", getSignTypeStr(cert.getSignature().getSignType()));
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		printf(" (0x%" PRIx32 ") (%s)", cert.getSignature().getSignType(), getEndiannessStr(cert.getSignature().isLittleEndian()));
	printf("\n");
	printf("  Issuer:         %s\n", cert.getBody().getIssuer().c_str());
	printf("  Subject:        %s\n", cert.getBody().getSubject().c_str());
	printf("  PublicKeyType:  %s", getPublicKeyType(cert.getBody().getPublicKeyType()));
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		printf(" (%d)", cert.getBody().getPublicKeyType());
	printf("\n");
	printf("  CertID:         0x%" PRIx32 " \n", cert.getBody().getCertId());
	
	if (cert.getBody().getPublicKeyType() == es::cert::RSA4096)
	{
		printf("  PublicKey:\n");
		printf("    Modulus:\n");
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa4098PublicKey().modulus, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::rsa::kRsa4096Size : 0x10, 0x10, 6);
		printf("    Public Exponent:\n");
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa4098PublicKey().public_exponent, crypto::rsa::kRsaPublicExponentSize, 0x10, 6);
	}
	else if (cert.getBody().getPublicKeyType() == es::cert::RSA2048)
	{
		printf("  PublicKey:\n");
		printf("    Public Exponent:\n");
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().modulus, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::rsa::kRsa2048Size : 0x10, 0x10, 6);
		printf("    Modulus:\n");
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().public_exponent, crypto::rsa::kRsaPublicExponentSize, 0x10, 6);
	}
	else if (cert.getBody().getPublicKeyType() == es::cert::ECDSA240)
	{
		printf("  PublicKey:\n");
		printf("    R:\n");
		fnd::SimpleTextOutput::hexDump(cert.getBody().getEcdsa240PublicKey().r, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::ecdsa::kEcdsa240Size : 0x10, 0x10, 6);
		printf("    S:\n");
		fnd::SimpleTextOutput::hexDump(cert.getBody().getEcdsa240PublicKey().s, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::ecdsa::kEcdsa240Size : 0x10, 0x10, 6);
	}
	


#undef _HEXDUMP_L
#undef _HEXDUMP_U
#undef _SPLIT_VER
}

const es::SignedData<es::CertificateBody>& EsCertProcess::getIssuerCert(const std::string& issuer_name) const
{
	std::string full_cert_name;
	for (size_t i = 0; i < mCert.size(); i++)
	{
		full_cert_name = mCert[i].getBody().getIssuer() + es::sign::kIdentDelimiter + mCert[i].getBody().getSubject();
		if (full_cert_name == issuer_name)
		{
			return mCert[i];
		}
	}

	throw fnd::Exception(kModuleName, "Issuer certificate does not exist");
}

const char* EsCertProcess::getSignTypeStr(es::sign::SignType type) const
{
	const char* str;
	switch (type)
	{
	case (es::sign::SIGN_RSA4096_SHA1):
		str = "RSA4096-SHA1";
		break;
	case (es::sign::SIGN_RSA2048_SHA1):
		str = "RSA2048-SHA1";
		break;
	case (es::sign::SIGN_ECDSA240_SHA1):
		str = "ECDSA240-SHA1";
		break;
	case (es::sign::SIGN_RSA4096_SHA256):
		str = "RSA4096-SHA256";
		break;
	case (es::sign::SIGN_RSA2048_SHA256):
		str = "RSA2048-SHA256";
		break;
	case (es::sign::SIGN_ECDSA240_SHA256):
		str = "ECDSA240-SHA256";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsCertProcess::getEndiannessStr(bool isLittleEndian) const
{
	return isLittleEndian ? "LittleEndian" : "BigEndian";
}

const char* EsCertProcess::getPublicKeyType(es::cert::PublicKeyType type) const
{
	const char* str;
	switch (type)
	{
	case (es::cert::RSA4096):
		str = "RSA4096";
		break;
	case (es::cert::RSA2048):
		str = "RSA2048";
		break;
	case (es::cert::ECDSA240):
		str = "ECDSA240";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}