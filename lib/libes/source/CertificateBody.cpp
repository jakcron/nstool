#include <es/CertificateBody.h>

es::CertificateBody::CertificateBody()
{
	clear();
}

es::CertificateBody::CertificateBody(const CertificateBody& other)
{
	copyFrom(other);
}

void es::CertificateBody::operator=(const CertificateBody& other)
{
	copyFrom(other);
}

bool es::CertificateBody::operator==(const CertificateBody& other) const
{
	return isEqual(other);
}

bool es::CertificateBody::operator!=(const CertificateBody& other) const
{
	return !(*this == other);
}

void es::CertificateBody::importBinary(const byte_t* src, size_t size)
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
	mBinaryBlob.alloc((sizeof(sCertificateHeader) + pubkeySize));
	memcpy(mBinaryBlob.getBytes(), src, mBinaryBlob.getSize());

	// save hdr variables
	hdr = (const sCertificateHeader*)mBinaryBlob.getBytes();

	if (hdr->issuer[0] != 0)
		mIssuer = std::string(hdr->issuer, cert::kIssuerSize);
	mPublicKeyType = (cert::PublicKeyType)hdr->key_type.get();
	if (hdr->subject[0] != 0)
		mSubject = std::string(hdr->subject, cert::kSubjectSize);
	mCertId = hdr->cert_id.get();

	// save public key
	if (mPublicKeyType == cert::RSA4096)
	{
		const sRsa4096PublicKeyBlock* pubkey = (const sRsa4096PublicKeyBlock*)(mBinaryBlob.getBytes() + sizeof(sCertificateHeader));
		memcpy(mRsa4096PublicKey.modulus, pubkey->modulus, sizeof(mRsa4096PublicKey.modulus));
		memcpy(mRsa4096PublicKey.public_exponent, pubkey->public_exponent, sizeof(mRsa4096PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::RSA2048)
	{
		const sRsa2048PublicKeyBlock* pubkey = (const sRsa2048PublicKeyBlock*)(mBinaryBlob.getBytes() + sizeof(sCertificateHeader));
		memcpy(mRsa2048PublicKey.modulus, pubkey->modulus, sizeof(mRsa2048PublicKey.modulus));
		memcpy(mRsa2048PublicKey.public_exponent, pubkey->public_exponent, sizeof(mRsa2048PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::ECDSA240)
	{
		const sEcdsa240PublicKeyBlock* pubkey = (const sEcdsa240PublicKeyBlock*)(mBinaryBlob.getBytes() + sizeof(sCertificateHeader));
		mEcdsa240PublicKey = pubkey->public_key;
	}
}

void es::CertificateBody::exportBinary()
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

	mBinaryBlob.alloc(sizeof(sCertificateHeader) + pubkeySize);
	sCertificateHeader* hdr = (sCertificateHeader*)mBinaryBlob.getBytes();

	// copy header vars
	strncpy(hdr->issuer, mIssuer.c_str(), cert::kIssuerSize);
	hdr->key_type = mPublicKeyType;
	strncpy(hdr->subject, mSubject.c_str(), cert::kSubjectSize);
	hdr->cert_id = mCertId;

	// copy public key
	if (mPublicKeyType == cert::RSA4096)
	{
		sRsa4096PublicKeyBlock* pubkey = (sRsa4096PublicKeyBlock*)(mBinaryBlob.getBytes() + sizeof(sCertificateHeader));
		memcpy(pubkey->modulus, mRsa4096PublicKey.modulus, sizeof(mRsa4096PublicKey.modulus));
		memcpy(pubkey->public_exponent, mRsa4096PublicKey.public_exponent, sizeof(mRsa4096PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::RSA2048)
	{
		sRsa2048PublicKeyBlock* pubkey = (sRsa2048PublicKeyBlock*)(mBinaryBlob.getBytes() + sizeof(sCertificateHeader));
		memcpy(pubkey->modulus, mRsa2048PublicKey.modulus, sizeof(mRsa2048PublicKey.modulus));
		memcpy(pubkey->public_exponent, mRsa2048PublicKey.public_exponent, sizeof(mRsa2048PublicKey.public_exponent));
	}
	else if (mPublicKeyType == cert::ECDSA240)
	{
		sEcdsa240PublicKeyBlock* pubkey = (sEcdsa240PublicKeyBlock*)(mBinaryBlob.getBytes() + sizeof(sCertificateHeader));
		pubkey->public_key = mEcdsa240PublicKey;
	}
}

const byte_t* es::CertificateBody::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t es::CertificateBody::getSize() const
{
	return mBinaryBlob.getSize();
}

void es::CertificateBody::clear()
{
	mIssuer.clear();
	mSubject.clear();
	mCertId = 0;
	mPublicKeyType = cert::RSA2048;

	memset(&mRsa4096PublicKey, 0, sizeof(crypto::rsa::sRsa4096Key));
	memset(&mRsa2048PublicKey, 0, sizeof(crypto::rsa::sRsa2048Key));
	memset(&mEcdsa240PublicKey, 0, sizeof(crypto::ecdsa::sEcdsa240Point));
}

const std::string& es::CertificateBody::getIssuer() const
{
	return mIssuer;
}

void es::CertificateBody::setIssuer(const std::string& issuer)
{
	if (issuer.size() > cert::kIssuerSize)
	{
		throw fnd::Exception(kModuleName, "Issuer name too long");
	}

	mIssuer = issuer;
}

es::cert::PublicKeyType es::CertificateBody::getPublicKeyType() const
{
	return mPublicKeyType;
}

void es::CertificateBody::setPublicKeyType(cert::PublicKeyType type)
{
	mPublicKeyType = type;
}

const std::string& es::CertificateBody::getSubject() const
{
	return mSubject;
}

void es::CertificateBody::setSubject(const std::string& subject)
{
	if (subject.size() > cert::kSubjectSize)
	{
		throw fnd::Exception(kModuleName, "Subject name too long");
	}

	mSubject = subject;
}

uint32_t es::CertificateBody::getCertId() const
{
	return mCertId;
}

void es::CertificateBody::setCertId(uint32_t id)
{
	mCertId = id;
}

const crypto::rsa::sRsa4096Key& es::CertificateBody::getRsa4098PublicKey() const
{
	return mRsa4096PublicKey;
}

void es::CertificateBody::setRsa4098PublicKey(const crypto::rsa::sRsa4096Key& key)
{
	mRsa4096PublicKey = key;
}

const crypto::rsa::sRsa2048Key& es::CertificateBody::getRsa2048PublicKey() const
{
	return mRsa2048PublicKey;
}

void es::CertificateBody::setRsa2048PublicKey(const crypto::rsa::sRsa2048Key& key)
{
	mRsa2048PublicKey = key;
}

const crypto::ecdsa::sEcdsa240Point& es::CertificateBody::getEcdsa240PublicKey() const
{
	return mEcdsa240PublicKey;
}

void es::CertificateBody::setEcdsa240PublicKey(const crypto::ecdsa::sEcdsa240Point& key)
{
	mEcdsa240PublicKey = key;
}

bool es::CertificateBody::isEqual(const CertificateBody& other) const
{
	return (mIssuer == other.mIssuer) \
		&& (mSubject == other.mSubject) \
		&& (mCertId == other.mCertId) \
		&& (mPublicKeyType == other.mPublicKeyType) \
		&& (mRsa4096PublicKey == other.mRsa4096PublicKey) \
		&& (mRsa2048PublicKey == other.mRsa2048PublicKey) \
		&& (mEcdsa240PublicKey == other.mEcdsa240PublicKey);

}

void es::CertificateBody::copyFrom(const CertificateBody& other)
{
	mBinaryBlob = other.mBinaryBlob;
	mIssuer = other.mIssuer;
	mSubject = other.mSubject;
	mCertId = other.mCertId;
	mPublicKeyType = other.mPublicKeyType;
	mRsa4096PublicKey = other.mRsa4096PublicKey;
	mRsa2048PublicKey = other.mRsa2048PublicKey;
	mEcdsa240PublicKey = other.mEcdsa240PublicKey;
}
