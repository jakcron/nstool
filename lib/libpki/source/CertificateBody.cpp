#include <nn/pki/CertificateBody.h>

nn::pki::CertificateBody::CertificateBody()
{
	clear();
}

nn::pki::CertificateBody::CertificateBody(const CertificateBody& other)
{
	*this = other;
}

void nn::pki::CertificateBody::operator=(const CertificateBody& other)
{
	mRawBinary = other.mRawBinary;
	mIssuer = other.mIssuer;
	mSubject = other.mSubject;
	mCertId = other.mCertId;
	mPublicKeyType = other.mPublicKeyType;
	mRsa4096PublicKey = other.mRsa4096PublicKey;
	mRsa2048PublicKey = other.mRsa2048PublicKey;
	mEcdsa240PublicKey = other.mEcdsa240PublicKey;
}

bool nn::pki::CertificateBody::operator==(const CertificateBody& other) const
{
	return (mIssuer == other.mIssuer) \
		&& (mSubject == other.mSubject) \
		&& (mCertId == other.mCertId) \
		&& (mPublicKeyType == other.mPublicKeyType) \
		&& (mRsa4096PublicKey == other.mRsa4096PublicKey) \
		&& (mRsa2048PublicKey == other.mRsa2048PublicKey) \
		&& (mEcdsa240PublicKey == other.mEcdsa240PublicKey);
}

bool nn::pki::CertificateBody::operator!=(const CertificateBody& other) const
{
	return !(*this == other);
}

void nn::pki::CertificateBody::toBytes()
{
	// get public key size
	size_t pubkeySize = 0;
	switch (mPublicKeyType)
	{
		case (cert::RSA4096):
			pubkeySize = sizeof(sRsa4096PublicKeyBlock);
			break;
		case (cert::RSA2048):
			pubkeySize = sizeof(sRsa2048PublicKeyBlock);
			break;
		case (cert::ECDSA240):
			pubkeySize = sizeof(sEcdsa240PublicKeyBlock);
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown public key type");
	}

	mRawBinary.alloc(sizeof(sCertificateHeader) + pubkeySize);
	sCertificateHeader* hdr = (sCertificateHeader*)mRawBinary.data();

	// copy header vars
	strncpy(hdr->issuer, mIssuer.c_str(), cert::kIssuerSize);
	hdr->key_type = mPublicKeyType;
	strncpy(hdr->subject, mSubject.c_str(), cert::kSubjectSize);
	hdr->cert_id = mCertId;

	// copy public key
	if (mPublicKeyType == cert::RSA4096)
	{
		sRsa4096PublicKeyBlock* pubkey = (sRsa4096PublicKeyBlock*)(mRawBinary.data() + sizeof(sCertificateHeader));
		memcpy(pubkey->modulus, mRsa4096PublicKey.modulus, sizeof(mRsa4096PublicKey.modulus));
		memcpy(pubkey->public_exponent, mRsa4096PublicKey.public_exponent, sizeof(mRsa4096PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::RSA2048)
	{
		sRsa2048PublicKeyBlock* pubkey = (sRsa2048PublicKeyBlock*)(mRawBinary.data() + sizeof(sCertificateHeader));
		memcpy(pubkey->modulus, mRsa2048PublicKey.modulus, sizeof(mRsa2048PublicKey.modulus));
		memcpy(pubkey->public_exponent, mRsa2048PublicKey.public_exponent, sizeof(mRsa2048PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::ECDSA240)
	{
		sEcdsa240PublicKeyBlock* pubkey = (sEcdsa240PublicKeyBlock*)(mRawBinary.data() + sizeof(sCertificateHeader));
		pubkey->public_key = mEcdsa240PublicKey;
	}
}

void nn::pki::CertificateBody::fromBytes(const byte_t* src, size_t size)
{
	clear();

	// check minimum size
	if (size < sizeof(sCertificateHeader))
	{ 
		throw fnd::Exception(kModuleName, "Certificate too small");
	}

	const sCertificateHeader* hdr = (const sCertificateHeader*)src;

	// get public key size
	size_t pubkeySize = 0;
	switch (hdr->key_type.get())
	{
		case (cert::RSA4096):
			pubkeySize = sizeof(sRsa4096PublicKeyBlock);
			break;
		case (cert::RSA2048):
			pubkeySize = sizeof(sRsa2048PublicKeyBlock);
			break;
		case (cert::ECDSA240):
			pubkeySize = sizeof(sEcdsa240PublicKeyBlock);
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown public key type");
	}

	// check total size
	if (size < (sizeof(sCertificateHeader) + pubkeySize))
	{ 
		throw fnd::Exception(kModuleName, "Certificate too small");
	}

	// save raw binary
	mRawBinary.alloc((sizeof(sCertificateHeader) + pubkeySize));
	memcpy(mRawBinary.data(), src, mRawBinary.size());

	// save hdr variables
	hdr = (const sCertificateHeader*)mRawBinary.data();

	if (hdr->issuer[0] != 0)
		mIssuer = std::string(hdr->issuer, _MIN(strlen(hdr->issuer), cert::kIssuerSize));
	mPublicKeyType = (cert::PublicKeyType)hdr->key_type.get();
	if (hdr->subject[0] != 0)
		mSubject = std::string(hdr->subject, _MIN(strlen(hdr->subject), cert::kSubjectSize));
	mCertId = hdr->cert_id.get();

	// save public key
	if (mPublicKeyType == cert::RSA4096)
	{
		const sRsa4096PublicKeyBlock* pubkey = (const sRsa4096PublicKeyBlock*)(mRawBinary.data() + sizeof(sCertificateHeader));
		memcpy(mRsa4096PublicKey.modulus, pubkey->modulus, sizeof(mRsa4096PublicKey.modulus));
		memcpy(mRsa4096PublicKey.public_exponent, pubkey->public_exponent, sizeof(mRsa4096PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::RSA2048)
	{
		const sRsa2048PublicKeyBlock* pubkey = (const sRsa2048PublicKeyBlock*)(mRawBinary.data() + sizeof(sCertificateHeader));
		memcpy(mRsa2048PublicKey.modulus, pubkey->modulus, sizeof(mRsa2048PublicKey.modulus));
		memcpy(mRsa2048PublicKey.public_exponent, pubkey->public_exponent, sizeof(mRsa2048PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::ECDSA240)
	{
		const sEcdsa240PublicKeyBlock* pubkey = (const sEcdsa240PublicKeyBlock*)(mRawBinary.data() + sizeof(sCertificateHeader));
		mEcdsa240PublicKey = pubkey->public_key;
	}
}

const fnd::Vec<byte_t>& nn::pki::CertificateBody::getBytes() const
{
	return mRawBinary;
}


void nn::pki::CertificateBody::clear()
{
	mIssuer.clear();
	mSubject.clear();
	mCertId = 0;
	mPublicKeyType = cert::RSA2048;

	memset(&mRsa4096PublicKey, 0, sizeof(fnd::rsa::sRsa4096Key));
	memset(&mRsa2048PublicKey, 0, sizeof(fnd::rsa::sRsa2048Key));
	memset(&mEcdsa240PublicKey, 0, sizeof(fnd::ecdsa::sEcdsa240Point));
}

const std::string& nn::pki::CertificateBody::getIssuer() const
{
	return mIssuer;
}

void nn::pki::CertificateBody::setIssuer(const std::string& issuer)
{
	if (issuer.size() > cert::kIssuerSize)
	{
		throw fnd::Exception(kModuleName, "Issuer name too long");
	}

	mIssuer = issuer;
}

nn::pki::cert::PublicKeyType nn::pki::CertificateBody::getPublicKeyType() const
{
	return mPublicKeyType;
}

void nn::pki::CertificateBody::setPublicKeyType(cert::PublicKeyType type)
{
	mPublicKeyType = type;
}

const std::string& nn::pki::CertificateBody::getSubject() const
{
	return mSubject;
}

void nn::pki::CertificateBody::setSubject(const std::string& subject)
{
	if (subject.size() > cert::kSubjectSize)
	{
		throw fnd::Exception(kModuleName, "Subject name too long");
	}

	mSubject = subject;
}

uint32_t nn::pki::CertificateBody::getCertId() const
{
	return mCertId;
}

void nn::pki::CertificateBody::setCertId(uint32_t id)
{
	mCertId = id;
}

const fnd::rsa::sRsa4096Key& nn::pki::CertificateBody::getRsa4098PublicKey() const
{
	return mRsa4096PublicKey;
}

void nn::pki::CertificateBody::setRsa4098PublicKey(const fnd::rsa::sRsa4096Key& key)
{
	mRsa4096PublicKey = key;
}

const fnd::rsa::sRsa2048Key& nn::pki::CertificateBody::getRsa2048PublicKey() const
{
	return mRsa2048PublicKey;
}

void nn::pki::CertificateBody::setRsa2048PublicKey(const fnd::rsa::sRsa2048Key& key)
{
	mRsa2048PublicKey = key;
}

const fnd::ecdsa::sEcdsa240Point& nn::pki::CertificateBody::getEcdsa240PublicKey() const
{
	return mEcdsa240PublicKey;
}

void nn::pki::CertificateBody::setEcdsa240PublicKey(const fnd::ecdsa::sEcdsa240Point& key)
{
	mEcdsa240PublicKey = key;
}