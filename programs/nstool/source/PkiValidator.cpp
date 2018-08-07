#include "PkiValidator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <nn/pki/SignUtils.h>

PkiValidator::PkiValidator()
{
	clearCertificates();
}

void PkiValidator::setRootKey(const fnd::rsa::sRsa4096Key& root_key)
{
	// save a copy of the certificate bank
	fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>> old_certs = mCertificateBank;
	
	// clear the certificate bank
	mCertificateBank.clear();

	// overwrite the root key
	mRootKey = root_key;

	// if there were certificates before, reimport them (so they are checked against the new root key)
	if (old_certs.size() > 0)
	{
		addCertificates(old_certs);
	}
}

void PkiValidator::addCertificates(const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& certs)
{
	for (size_t i = 0; i < certs.size(); i++)
	{
		addCertificate(certs[i]);
	}
}

void PkiValidator::addCertificate(const nn::pki::SignedData<nn::pki::CertificateBody>& cert)
{
	std::string cert_ident;
	nn::pki::sign::SignatureAlgo cert_sign_algo;
	nn::pki::sign::HashAlgo cert_hash_algo;
	fnd::Vec<byte_t> cert_hash;

	try 
	{	
		makeCertIdent(cert, cert_ident);

		if (doesCertExist(cert_ident) == true)
		{
			throw fnd::Exception(kModuleName, "Certificate already exists");
		}

		cert_sign_algo = nn::pki::sign::getSignatureAlgo(cert.getSignature().getSignType());
		cert_hash_algo = nn::pki::sign::getHashAlgo(cert.getSignature().getSignType());

		// get cert hash
		switch (cert_hash_algo)
		{
		case (nn::pki::sign::HASH_ALGO_SHA1):
			cert_hash.alloc(fnd::sha::kSha1HashLen);
			fnd::sha::Sha1(cert.getBody().getBytes().data(), cert.getBody().getBytes().size(), cert_hash.data());
			break;
		case (nn::pki::sign::HASH_ALGO_SHA256):
			cert_hash.alloc(fnd::sha::kSha256HashLen);
			fnd::sha::Sha256(cert.getBody().getBytes().data(), cert.getBody().getBytes().size(), cert_hash.data());
			break;
		default:
			throw fnd::Exception(kModuleName, "Unrecognised hash type");
		}

		validateSignature(cert.getBody().getIssuer(), cert.getSignature().getSignType(), cert.getSignature().getSignature(), cert_hash);

		mCertificateBank.addElement(cert);
	}
	catch (const fnd::Exception& e) 
	{
		std::stringstream ss;
		ss << "Failed to add certificate " << cert_ident << " (" << e.error() << ")";
		throw fnd::Exception(kModuleName, ss.str());
	}
}

void PkiValidator::clearCertificates()
{
	mCertificateBank.clear();
}

void PkiValidator::validateSignature(const std::string& issuer, nn::pki::sign::SignatureId signature_id, const fnd::Vec<byte_t>& signature, const fnd::Vec<byte_t>& hash) const
{	
	nn::pki::sign::SignatureAlgo sign_algo = nn::pki::sign::getSignatureAlgo(signature_id);
	nn::pki::sign::HashAlgo hash_algo = nn::pki::sign::getHashAlgo(signature_id);
	

	// validate signature
	int sig_validate_res = -1;

	// special case if signed by Root
	if (issuer == nn::pki::sign::kRootIssuerStr)
	{
		if (sign_algo != nn::pki::sign::SIGN_ALGO_RSA4096)
		{
			throw fnd::Exception(kModuleName, "Issued by Root, but does not have a RSA4096 signature");
		}
		sig_validate_res = fnd::rsa::pkcs::rsaVerify(mRootKey, getCryptoHashAlgoFromEsSignHashAlgo(hash_algo), hash.data(), signature.data()); 
	}
	else
	{
		// try to find issuer cert		
		const nn::pki::CertificateBody& issuer_cert = getCert(issuer).getBody();
		nn::pki::cert::PublicKeyType issuer_pubk_type = issuer_cert.getPublicKeyType();

		if (issuer_pubk_type == nn::pki::cert::RSA4096 && sign_algo == nn::pki::sign::SIGN_ALGO_RSA4096)
		{
			sig_validate_res = fnd::rsa::pkcs::rsaVerify(issuer_cert.getRsa4098PublicKey(), getCryptoHashAlgoFromEsSignHashAlgo(hash_algo), hash.data(), signature.data()); 
		}
		else if (issuer_pubk_type == nn::pki::cert::RSA2048 && sign_algo == nn::pki::sign::SIGN_ALGO_RSA2048)
		{
			sig_validate_res = fnd::rsa::pkcs::rsaVerify(issuer_cert.getRsa2048PublicKey(), getCryptoHashAlgoFromEsSignHashAlgo(hash_algo), hash.data(), signature.data()); 
		}
		else if (issuer_pubk_type == nn::pki::cert::ECDSA240 && sign_algo == nn::pki::sign::SIGN_ALGO_ECDSA240)
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

void PkiValidator::makeCertIdent(const nn::pki::SignedData<nn::pki::CertificateBody>& cert, std::string& ident) const
{
	makeCertIdent(cert.getBody().getIssuer(), cert.getBody().getSubject(), ident);
}

void PkiValidator::makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const
{
	ident = issuer + nn::pki::sign::kIdentDelimiter + subject;
	ident = ident.substr(0, _MIN(ident.length(),64));
}

bool PkiValidator::doesCertExist(const std::string& ident) const
{
	bool exists = false;
	std::string full_cert_name;
	for (size_t i = 0; i < mCertificateBank.size(); i++)
	{
		makeCertIdent(mCertificateBank[i], full_cert_name);
		if (full_cert_name == ident)
		{
			exists = true;
			break;
		}
	}

	return exists;
}

const nn::pki::SignedData<nn::pki::CertificateBody>& PkiValidator::getCert(const std::string& ident) const
{
	std::string full_cert_name;
	for (size_t i = 0; i < mCertificateBank.size(); i++)
	{
		makeCertIdent(mCertificateBank[i], full_cert_name);
		if (full_cert_name == ident)
		{
			return mCertificateBank[i];
		}
	}

	throw fnd::Exception(kModuleName, "Issuer certificate does not exist");
}

fnd::sha::HashType PkiValidator::getCryptoHashAlgoFromEsSignHashAlgo(nn::pki::sign::HashAlgo hash_algo) const
{
	fnd::sha::HashType hash_type = fnd::sha::HASH_SHA1;

	switch (hash_algo)
	{
	case (nn::pki::sign::HASH_ALGO_SHA1):
		hash_type = fnd::sha::HASH_SHA1;
		break;
	case (nn::pki::sign::HASH_ALGO_SHA256):
		hash_type = fnd::sha::HASH_SHA256;
		break;
	};

	return hash_type;
}