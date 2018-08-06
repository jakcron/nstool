#include "PkiValidator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <pki/SignUtils.h>

PkiValidator::PkiValidator()
{
	clearCertificates();
}

void PkiValidator::setRootKey(const crypto::rsa::sRsa4096Key& root_key)
{
	// save a copy of the certificate bank
	fnd::List<pki::SignedData<pki::CertificateBody>> old_certs = mCertificateBank;
	
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

void PkiValidator::addCertificates(const fnd::List<pki::SignedData<pki::CertificateBody>>& certs)
{
	for (size_t i = 0; i < certs.size(); i++)
	{
		addCertificate(certs[i]);
	}
}

void PkiValidator::addCertificate(const pki::SignedData<pki::CertificateBody>& cert)
{
	std::string cert_ident;
	pki::sign::SignatureAlgo cert_sign_algo;
	pki::sign::HashAlgo cert_hash_algo;
	fnd::Vec<byte_t> cert_hash;

	try 
	{	
		makeCertIdent(cert, cert_ident);

		if (doesCertExist(cert_ident) == true)
		{
			throw fnd::Exception(kModuleName, "Certificate already exists");
		}

		cert_sign_algo = pki::sign::getSignatureAlgo(cert.getSignature().getSignType());
		cert_hash_algo = pki::sign::getHashAlgo(cert.getSignature().getSignType());

		// get cert hash
		switch (cert_hash_algo)
		{
		case (pki::sign::HASH_ALGO_SHA1):
			cert_hash.alloc(crypto::sha::kSha1HashLen);
			crypto::sha::Sha1(cert.getBody().getBytes().data(), cert.getBody().getBytes().size(), cert_hash.data());
			break;
		case (pki::sign::HASH_ALGO_SHA256):
			cert_hash.alloc(crypto::sha::kSha256HashLen);
			crypto::sha::Sha256(cert.getBody().getBytes().data(), cert.getBody().getBytes().size(), cert_hash.data());
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

void PkiValidator::validateSignature(const std::string& issuer, pki::sign::SignatureId signature_id, const fnd::Vec<byte_t>& signature, const fnd::Vec<byte_t>& hash) const
{	
	pki::sign::SignatureAlgo sign_algo = pki::sign::getSignatureAlgo(signature_id);
	pki::sign::HashAlgo hash_algo = pki::sign::getHashAlgo(signature_id);
	

	// validate signature
	int sig_validate_res = -1;

	// special case if signed by Root
	if (issuer == pki::sign::kRootIssuerStr)
	{
		if (sign_algo != pki::sign::SIGN_ALGO_RSA4096)
		{
			throw fnd::Exception(kModuleName, "Issued by Root, but does not have a RSA4096 signature");
		}
		sig_validate_res = crypto::rsa::pkcs::rsaVerify(mRootKey, getCryptoHashAlgoFromEsSignHashAlgo(hash_algo), hash.data(), signature.data()); 
	}
	else
	{
		// try to find issuer cert		
		const pki::CertificateBody& issuer_cert = getCert(issuer).getBody();
		pki::cert::PublicKeyType issuer_pubk_type = issuer_cert.getPublicKeyType();

		if (issuer_pubk_type == pki::cert::RSA4096 && sign_algo == pki::sign::SIGN_ALGO_RSA4096)
		{
			sig_validate_res = crypto::rsa::pkcs::rsaVerify(issuer_cert.getRsa4098PublicKey(), getCryptoHashAlgoFromEsSignHashAlgo(hash_algo), hash.data(), signature.data()); 
		}
		else if (issuer_pubk_type == pki::cert::RSA2048 && sign_algo == pki::sign::SIGN_ALGO_RSA2048)
		{
			sig_validate_res = crypto::rsa::pkcs::rsaVerify(issuer_cert.getRsa2048PublicKey(), getCryptoHashAlgoFromEsSignHashAlgo(hash_algo), hash.data(), signature.data()); 
		}
		else if (issuer_pubk_type == pki::cert::ECDSA240 && sign_algo == pki::sign::SIGN_ALGO_ECDSA240)
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

void PkiValidator::makeCertIdent(const pki::SignedData<pki::CertificateBody>& cert, std::string& ident) const
{
	makeCertIdent(cert.getBody().getIssuer(), cert.getBody().getSubject(), ident);
}

void PkiValidator::makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const
{
	ident = issuer + pki::sign::kIdentDelimiter + subject;
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

const pki::SignedData<pki::CertificateBody>& PkiValidator::getCert(const std::string& ident) const
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

crypto::sha::HashType PkiValidator::getCryptoHashAlgoFromEsSignHashAlgo(pki::sign::HashAlgo hash_algo) const
{
	crypto::sha::HashType hash_type = crypto::sha::HASH_SHA1;

	switch (hash_algo)
	{
	case (pki::sign::HASH_ALGO_SHA1):
		hash_type = crypto::sha::HASH_SHA1;
		break;
	case (pki::sign::HASH_ALGO_SHA256):
		hash_type = crypto::sha::HASH_SHA256;
		break;
	};

	return hash_type;
}