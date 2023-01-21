#include "PkiValidator.h"

#include <tc/crypto.h>
#include <pietendo/hac/define/types.h>
#include <pietendo/hac/es/SignUtils.h>

nstool::PkiValidator::PkiValidator() :
	mModuleName("nstool::PkiValidator")
{
	clearCertificates();
}

void nstool::PkiValidator::setKeyCfg(const KeyBag& keycfg)
{
	// save a copy of the certificate bank
	std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>> old_certs = mCertificateBank;
	
	// clear the certificate bank
	mCertificateBank.clear();

	// overwrite the root key
	mKeyCfg = keycfg;

	// if there were certificates before, reimport them (so they are checked against the new root key)
	if (old_certs.size() > 0)
	{
		addCertificates(old_certs);
	}
}

void nstool::PkiValidator::addCertificates(const std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>>& certs)
{
	for (size_t i = 0; i < certs.size(); i++)
	{
		addCertificate(certs[i]);
	}
}

void nstool::PkiValidator::addCertificate(const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& cert)
{
	std::string cert_ident;
	pie::hac::es::sign::HashAlgo cert_hash_algo;
	tc::ByteData cert_hash;

	try 
	{	
		makeCertIdent(cert, cert_ident);

		if (doesCertExist(cert_ident) == true)
		{
			throw tc::Exception(mModuleName, "Certificate already exists");
		}

		cert_hash_algo = pie::hac::es::sign::getHashAlgo(cert.getSignature().getSignType());

		// get cert hash
		switch (cert_hash_algo)
		{
		case (pie::hac::es::sign::HASH_ALGO_SHA1):

			cert_hash = tc::ByteData(tc::crypto::Sha1Generator::kHashSize);
			tc::crypto::GenerateSha1Hash(cert_hash.data(), cert.getBody().getBytes().data(), cert.getBody().getBytes().size());
			break;
		case (pie::hac::es::sign::HASH_ALGO_SHA256):
			cert_hash = tc::ByteData(tc::crypto::Sha2256Generator::kHashSize);
			tc::crypto::GenerateSha2256Hash(cert_hash.data(), cert.getBody().getBytes().data(), cert.getBody().getBytes().size());
			break;
		default:
			throw tc::Exception(mModuleName, "Unrecognised hash type");
		}

		validateSignature(cert.getBody().getIssuer(), cert.getSignature().getSignType(), cert.getSignature().getSignature(), cert_hash);

		mCertificateBank.push_back(cert);
	}
	catch (const tc::Exception& e) 
	{
		throw tc::Exception(mModuleName, fmt::format("Failed to add certificate {:s} ({:s})", cert_ident, e.error()));
	}
}

void nstool::PkiValidator::clearCertificates()
{
	mCertificateBank.clear();
}

void nstool::PkiValidator::validateSignature(const std::string& issuer, pie::hac::es::sign::SignatureId signature_id, const tc::ByteData& signature, const tc::ByteData& hash) const
{	
	pie::hac::es::sign::SignatureAlgo sign_algo = pie::hac::es::sign::getSignatureAlgo(signature_id);	

	// validate signature
	bool sig_valid = false;

	// get public key
	// tc::crypto::EccKey ecc_key;
	tc::crypto::RsaKey rsa_key;

	// special case if signed by Root (legacy nstool only defers to keybag for "Root", it did not store certificates)
	if (issuer == "Root")
	{
		auto itr = mKeyCfg.broadon_signer.find(issuer);

		if (itr == mKeyCfg.broadon_signer.end())
		{
			throw tc::Exception(mModuleName, fmt::format("Public key for issuer \"{:s}\" does not exist.", issuer));
		}

		if (sign_algo != itr->second.key_type)
		{
			throw tc::Exception(mModuleName, fmt::format("Public key for issuer \"{:s}\" cannot verify this signature.", issuer));
		}

		if (sign_algo == pie::hac::es::sign::SIGN_ALGO_ECDSA240)
		{
			throw tc::Exception(mModuleName, "ECDSA signatures are not supported");	
		}

		rsa_key = itr->second.rsa_key;
	}
	else
	{
		// try to find issuer cert		
		const pie::hac::es::CertificateBody& issuer_cert = getCert(issuer).getBody();
		pie::hac::es::cert::PublicKeyType issuer_pubk_type = issuer_cert.getPublicKeyType();

		if (issuer_pubk_type == pie::hac::es::cert::RSA4096 && sign_algo == pie::hac::es::sign::SIGN_ALGO_RSA4096)
		{
			rsa_key = issuer_cert.getRsa4096PublicKey();
		}
		else if (issuer_pubk_type == pie::hac::es::cert::RSA2048 && sign_algo == pie::hac::es::sign::SIGN_ALGO_RSA2048)
		{
			rsa_key = issuer_cert.getRsa2048PublicKey();
		}
		else if (issuer_pubk_type == pie::hac::es::cert::ECDSA240 && sign_algo == pie::hac::es::sign::SIGN_ALGO_ECDSA240)
		{
			// ecc_key = issuer_cert.getEcdsa240PublicKey();
			throw tc::Exception(mModuleName, "ECDSA signatures are not supported");
		}
		else
		{
			throw tc::Exception(mModuleName, "Mismatch between issuer public key and signature type");
		}
	}

	// verify signature
	switch (signature_id) {
		case (pie::hac::es::sign::SIGN_ID_RSA4096_SHA1):
			sig_valid = tc::crypto::VerifyRsa4096Pkcs1Sha1(signature.data(), hash.data(), rsa_key);
			break;
		case (pie::hac::es::sign::SIGN_ID_RSA2048_SHA1):
			sig_valid = tc::crypto::VerifyRsa2048Pkcs1Sha1(signature.data(), hash.data(), rsa_key);
			break;
		case (pie::hac::es::sign::SIGN_ID_ECDSA240_SHA1):
			sig_valid = false;
			break;
		case (pie::hac::es::sign::SIGN_ID_RSA4096_SHA256):
			sig_valid = tc::crypto::VerifyRsa4096Pkcs1Sha2256(signature.data(), hash.data(), rsa_key);
			break;
		case (pie::hac::es::sign::SIGN_ID_RSA2048_SHA256):
			sig_valid = tc::crypto::VerifyRsa2048Pkcs1Sha2256(signature.data(), hash.data(), rsa_key);
			break;
		case (pie::hac::es::sign::SIGN_ID_ECDSA240_SHA256):
			sig_valid = false;
			break;
	}

	if (sig_valid == false)
	{
		throw tc::Exception(mModuleName, "Incorrect signature");
	}

	
}

void nstool::PkiValidator::makeCertIdent(const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& cert, std::string& ident) const
{
	makeCertIdent(cert.getBody().getIssuer(), cert.getBody().getSubject(), ident);
}

void nstool::PkiValidator::makeCertIdent(const std::string& issuer, const std::string& subject, std::string& ident) const
{
	ident = issuer + pie::hac::es::sign::kIdentDelimiter + subject;
	ident = ident.substr(0, std::min<size_t>(ident.length(),64));
}

bool nstool::PkiValidator::doesCertExist(const std::string& ident) const
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

const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& nstool::PkiValidator::getCert(const std::string& ident) const
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

	throw tc::Exception(mModuleName, "Issuer certificate does not exist");
}