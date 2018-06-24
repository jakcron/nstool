#include <nx/AcidBinary.h>

nx::AcidBinary::AcidBinary()
{
	clear();
}

nx::AcidBinary::AcidBinary(const AcidBinary & other)
{
	*this = other;
}

bool nx::AcidBinary::operator==(const AcidBinary & other) const
{
	return AciBinary::operator==(other) \
		&& (mEmbeddedPublicKey == other.mEmbeddedPublicKey);
}

bool nx::AcidBinary::operator!=(const AcidBinary & other) const
{
	return !(*this == other);
}

void nx::AcidBinary::operator=(const AcidBinary & other)
{
	if (other.getBytes().size())
	{
		importBinary(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		AciBinary::operator=(other);
		mEmbeddedPublicKey = other.mEmbeddedPublicKey;
	}
}

void nx::AcidBinary::toBytes()
{
	AciBinary::setHeaderOffset(crypto::rsa::kRsa2048Size); // not include signature
	AciBinary::exportBinary();
	mRawBinary.alloc(AciBinary::getBytes().size() + crypto::rsa::kRsa2048Size * 2);
	
	memcpy(mRawBinary.data() + crypto::rsa::kRsa2048Size, mEmbeddedPublicKey.modulus, crypto::rsa::kRsa2048Size);
	memcpy(mRawBinary.data() + crypto::rsa::kRsa2048Size * 2, AciBinary::getBytes().data(), AciBinary::getBytes().size());
}

void nx::AcidBinary::signBinary(const crypto::rsa::sRsa2048Key & key)
{
	if (mRawBinary.size() == 0)
	{
		exportBinary();
	}

	byte_t hash[crypto::sha::kSha256HashLen];
	crypto::sha::Sha256(mRawBinary.data() + crypto::rsa::kRsa2048Size, mRawBinary.size() - crypto::rsa::kRsa2048Size, hash);

	if (crypto::rsa::pkcs::rsaSign(key, crypto::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to sign ACID");
	}
}

void nx::AcidBinary::fromBytes(const byte_t * bytes, size_t len)
{
	if (len <= crypto::rsa::kRsa2048Size*2)
	{
		throw fnd::Exception(kModuleName, "ACID binary too small");
	}

	// import aci binary past sig + pubkey
	AciBinary::importBinary(bytes + crypto::rsa::kRsa2048Size * 2, len - crypto::rsa::kRsa2048Size * 2);

	// save internal copy
	size_t acid_size = AciBinary::getBytes().size() + crypto::rsa::kRsa2048Size * 2;
	if (acid_size > len)
	{
		throw fnd::Exception(kModuleName, "ACID binary too small");
	}

	mRawBinary.alloc(acid_size);
	memcpy(mRawBinary.data(), bytes, mRawBinary.size());
	memcpy(mEmbeddedPublicKey.modulus, bytes + crypto::rsa::kRsa2048Size, crypto::rsa::kRsa2048Size);
}

void nx::AcidBinary::verifyBinary(const crypto::rsa::sRsa2048Key & key) const
{
	if (mRawBinary.size() == 0)
	{
		throw fnd::Exception(kModuleName, "No ACID binary exists to verify");
	}

	byte_t hash[crypto::sha::kSha256HashLen];
	crypto::sha::Sha256(mRawBinary.data() + crypto::rsa::kRsa2048Size, mRawBinary.size() - crypto::rsa::kRsa2048Size, hash);

	if (crypto::rsa::pss::rsaVerify(key, crypto::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to verify ACID");
	}
}

const fnd::Vec<byte_t>& nx::AcidBinary::getBytes() const
{
	return mRawBinary;
}

void nx::AcidBinary::clear()
{
	AciBinary::clear();
	mRawBinary.clear();
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
