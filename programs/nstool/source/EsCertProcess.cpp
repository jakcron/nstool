#include <iostream>
#include <iomanip>

#include <fnd/SimpleTextOutput.h>
#include <es/SignUtils.h>
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
	std::string cert_ident = cert.getBody().getIssuer() + es::sign::kIdentDelimiter + cert.getBody().getSubject();
	
	es::sign::SignatureAlgo cert_sign_algo = es::sign::getSignatureAlgo(cert.getSignature().getSignType());
	es::sign::HashAlgo cert_hash_algo = es::sign::getHashAlgo(cert.getSignature().getSignType());
	byte_t cert_hash[crypto::sha::kSha256HashLen];
	memset(cert_hash, 0, crypto::sha::kSha256HashLen);


	try 
	{
		// get cert hash
		switch (cert_hash_algo)
		{
		case (es::sign::HASH_ALGO_SHA1):
			crypto::sha::Sha1(cert.getBody().getBytes().data(), cert.getBody().getBytes().size(), cert_hash);
			break;
		case (es::sign::HASH_ALGO_SHA256):
			crypto::sha::Sha256(cert.getBody().getBytes().data(), cert.getBody().getBytes().size(), cert_hash);
			break;
		default:
			throw fnd::Exception(kModuleName, "Unrecognised hash type");
		}

		// validate signature
		int sig_validate_res = -1;

		// special case if signed by Root
		if (cert.getBody().getIssuer() == es::sign::kRootIssuerStr)
		{
			if (cert_sign_algo != es::sign::SIGN_ALGO_RSA4096)
			{
				throw fnd::Exception(kModuleName, "Issued by Root, but does not have a RSA4096 signature");
			}
			sig_validate_res = crypto::rsa::pkcs::rsaVerify(mKeyset->pki.root_sign_key, getCryptoHashAlgoFromEsSignHashAlgo(cert_hash_algo), cert_hash, cert.getSignature().getSignature().data()); 
		}
		else
		{
			// try to find issuer cert		
			const es::CertificateBody& issuer = getIssuerCert(cert.getBody().getIssuer()).getBody();
			es::cert::PublicKeyType issuer_pubk_type = issuer.getPublicKeyType();

			if (issuer_pubk_type == es::cert::RSA4096 && cert_sign_algo == es::sign::SIGN_ALGO_RSA4096)
			{
				sig_validate_res = crypto::rsa::pkcs::rsaVerify(issuer.getRsa4098PublicKey(), getCryptoHashAlgoFromEsSignHashAlgo(cert_hash_algo), cert_hash, cert.getSignature().getSignature().data()); 
			}
			else if (issuer_pubk_type == es::cert::RSA2048 && cert_sign_algo == es::sign::SIGN_ALGO_RSA2048)
			{
				sig_validate_res = crypto::rsa::pkcs::rsaVerify(issuer.getRsa2048PublicKey(), getCryptoHashAlgoFromEsSignHashAlgo(cert_hash_algo), cert_hash, cert.getSignature().getSignature().data()); 
			}
			else if (issuer_pubk_type == es::cert::ECDSA240 && cert_sign_algo == es::sign::SIGN_ALGO_ECDSA240)
			{
				throw fnd::Exception(kModuleName, "ECDSA signatures are not supported");
			}
			else
			{
				throw fnd::Exception(kModuleName, "Mismatch between issuer public key and signature type");
			}
		}

		if (sig_validate_res != 0)
		{
			throw fnd::Exception(kModuleName, "Incorrect signature");
		}
	}
	catch (const fnd::Exception& e) 
	{
		std::cout << "[WARNING] Failed to validate " << cert_ident << " (" << e.error() << ")" << std::endl;
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

	std::cout << "[ES Certificate]" << std::endl;

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
	
	if (cert.getBody().getPublicKeyType() == es::cert::RSA4096)
	{
		std::cout << "  PublicKey:" << std::endl;
		std::cout << "    Modulus:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa4098PublicKey().modulus, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::rsa::kRsa4096Size : 0x10, 0x10, 6);
		std::cout << "    Public Exponent:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa4098PublicKey().public_exponent, crypto::rsa::kRsaPublicExponentSize, 0x10, 6);
	}
	else if (cert.getBody().getPublicKeyType() == es::cert::RSA2048)
	{
		std::cout << "  PublicKey:" << std::endl;
		std::cout << "    Public Exponent:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().modulus, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::rsa::kRsa2048Size : 0x10, 0x10, 6);
		std::cout << "    Modulus:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getRsa2048PublicKey().public_exponent, crypto::rsa::kRsaPublicExponentSize, 0x10, 6);
	}
	else if (cert.getBody().getPublicKeyType() == es::cert::ECDSA240)
	{
		std::cout << "  PublicKey:" << std::endl;
		std::cout << "    R:" << std::endl;
		fnd::SimpleTextOutput::hexDump(cert.getBody().getEcdsa240PublicKey().r, _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED) ? crypto::ecdsa::kEcdsa240Size : 0x10, 0x10, 6);
		std::cout << "    S:" << std::endl;
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

crypto::sha::HashType EsCertProcess::getCryptoHashAlgoFromEsSignHashAlgo(es::sign::HashAlgo es_hash_algo) const
{
	crypto::sha::HashType hash_type = crypto::sha::HASH_SHA1;

	switch (es_hash_algo)
	{
	case (es::sign::HASH_ALGO_SHA1):
		hash_type = crypto::sha::HASH_SHA1;
		break;
	case (es::sign::HASH_ALGO_SHA256):
		hash_type = crypto::sha::HASH_SHA256;
		break;
	};

	return hash_type;
}


const char* EsCertProcess::getSignTypeStr(es::sign::SignatureId type) const
{
	const char* str;
	switch (type)
	{
	case (es::sign::SIGN_ID_RSA4096_SHA1):
		str = "RSA4096-SHA1";
		break;
	case (es::sign::SIGN_ID_RSA2048_SHA1):
		str = "RSA2048-SHA1";
		break;
	case (es::sign::SIGN_ID_ECDSA240_SHA1):
		str = "ECDSA240-SHA1";
		break;
	case (es::sign::SIGN_ID_RSA4096_SHA256):
		str = "RSA4096-SHA256";
		break;
	case (es::sign::SIGN_ID_RSA2048_SHA256):
		str = "RSA2048-SHA256";
		break;
	case (es::sign::SIGN_ID_ECDSA240_SHA256):
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

const char* EsCertProcess::getPublicKeyTypeStr(es::cert::PublicKeyType type) const
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