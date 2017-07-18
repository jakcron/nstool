#include "AcidBinary.h"



nx::AcidBinary::AcidBinary()
{
	clear();
}

nx::AcidBinary::AcidBinary(const AcidBinary & other)
{
	copyFrom(other);
}

nx::AcidBinary::AcidBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

void nx::AcidBinary::clear()
{
	AciBinary::clear();
	mEmbeddedPublicKey = crypto::rsa::sRsa2048Key();
}

const crypto::rsa::sRsa2048Key & nx::AcidBinary::getNcaHeader2RsaKey() const
{
	return mEmbeddedPublicKey;
}

void nx::AcidBinary::setNcaHeader2RsaKey(const crypto::rsa::sRsa2048Key & key)
{
	mEmbeddedPublicKey = key;
}

bool nx::AcidBinary::isEqual(const AcidBinary & other) const
{
	return AciBinary::operator==(other) \
		&& (mEmbeddedPublicKey == other.mEmbeddedPublicKey);
}

void nx::AcidBinary::copyFrom(const AcidBinary & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		AciBinary::operator=(other);
		mEmbeddedPublicKey = other.mEmbeddedPublicKey;
	}
}

bool nx::AcidBinary::operator==(const AcidBinary & other) const
{
	return isEqual(other);
}

bool nx::AcidBinary::operator!=(const AcidBinary & other) const
{
	return !isEqual(other);
}

void nx::AcidBinary::operator=(const AcidBinary & other)
{
	copyFrom(other);
}

const u8 * nx::AcidBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::AcidBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::AcidBinary::exportBinary()
{
	AciBinary::setHeaderOffset(crypto::rsa::kRsa2048Size); // not include signature
	AciBinary::exportBinary();
	mBinaryBlob.alloc(AciBinary::getSize() + crypto::rsa::kRsa2048Size * 2);
	
	memcpy(mBinaryBlob.getBytes() + crypto::rsa::kRsa2048Size, mEmbeddedPublicKey.modulus, crypto::rsa::kRsa2048Size);
	memcpy(mBinaryBlob.getBytes() + crypto::rsa::kRsa2048Size * 2, AciBinary::getBytes(), AciBinary::getSize());
}

void nx::AcidBinary::signBinary(const crypto::rsa::sRsa2048Key & key)
{
	if (mBinaryBlob.getSize() == 0)
	{
		exportBinary();
	}

	u8 hash[crypto::sha::kSha256HashLen];
	crypto::sha::Sha256(mBinaryBlob.getBytes() + crypto::rsa::kRsa2048Size, mBinaryBlob.getSize() - crypto::rsa::kRsa2048Size, hash);

	if (crypto::rsa::pkcs::rsaSign(key, crypto::sha::HASH_SHA256, hash, mBinaryBlob.getBytes()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to sign ACID");
	}
}

void nx::AcidBinary::importBinary(const u8 * bytes, size_t len)
{
	if (len <= crypto::rsa::kRsa2048Size*2)
	{
		throw fnd::Exception(kModuleName, "ACID binary too small");
	}

	// import aci binary past sig + pubkey
	AciBinary::importBinary(bytes + crypto::rsa::kRsa2048Size * 2, len - crypto::rsa::kRsa2048Size * 2);

	// save internal copy
	size_t acid_size = AciBinary::getSize() + crypto::rsa::kRsa2048Size * 2;
	if (acid_size > len)
	{
		throw fnd::Exception(kModuleName, "ACID binary too small");
	}

	mBinaryBlob.alloc(acid_size);
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	memcpy(mEmbeddedPublicKey.modulus, bytes + crypto::rsa::kRsa2048Size, crypto::rsa::kRsa2048Size);
}

void nx::AcidBinary::verifyBinary(const crypto::rsa::sRsa2048Key & key)
{
	if (mBinaryBlob.getSize() == 0)
	{
		throw fnd::Exception(kModuleName, "No ACID binary exists to verify");
	}

	u8 hash[crypto::sha::kSha256HashLen];
	crypto::sha::Sha256(mBinaryBlob.getBytes() + crypto::rsa::kRsa2048Size, mBinaryBlob.getSize() - crypto::rsa::kRsa2048Size, hash);

	if (crypto::rsa::pkcs::rsaVerify(key, crypto::sha::HASH_SHA256, hash, mBinaryBlob.getBytes()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to verify ACID");
	}
}
