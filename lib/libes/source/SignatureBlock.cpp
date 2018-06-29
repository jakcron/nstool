#include <es/SignatureBlock.h>

es::SignatureBlock::SignatureBlock()
{
	clear();
}

es::SignatureBlock::SignatureBlock(const SignatureBlock& other)
{
	*this = other;
}

void es::SignatureBlock::operator=(const SignatureBlock& other)
{
	mRawBinary = other.mRawBinary;
	mSignType = other.mSignType;
	mIsLittleEndian = other.mIsLittleEndian;
	mSignature = other.mSignature;
}

bool es::SignatureBlock::operator==(const SignatureBlock& other) const
{
	return (mSignType == other.mSignType) \
		&& (mIsLittleEndian == other.mIsLittleEndian) \
		&& (mSignature == other.mSignature);
}

bool es::SignatureBlock::operator!=(const SignatureBlock& other) const
{
	return !(*this == other);
}

void es::SignatureBlock::toBytes()
{
	size_t totalSize = 0;
	size_t sigSize = 0;

	switch (mSignType)
	{
		case (sign::SIGN_RSA4096_SHA1):
		case (sign::SIGN_RSA4096_SHA256):
			totalSize = sizeof(sRsa4096SignBlock);
			sigSize = crypto::rsa::kRsa4096Size;
			break;
		case (sign::SIGN_RSA2048_SHA1):
		case (sign::SIGN_RSA2048_SHA256):
			totalSize = sizeof(sRsa2048SignBlock);
			sigSize = crypto::rsa::kRsa2048Size;
			break;
		case (sign::SIGN_ECDSA240_SHA1):
		case (sign::SIGN_ECDSA240_SHA256):
			totalSize = sizeof(sEcdsa240SignBlock);
			sigSize = sign::kEcdsaSigSize;
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown signature type");
	}

	if (mSignature.size() != sigSize)
		throw fnd::Exception(kModuleName, "Signature size is incorrect");

	// commit to binary
	mRawBinary.alloc(totalSize);
	if (mIsLittleEndian)
		*(le_uint32_t*)(mRawBinary.data()) = mSignType;
	else
		*(be_uint32_t*)(mRawBinary.data()) = mSignType;
	memcpy(mRawBinary.data() + 4, mSignature.data(), sigSize);
}

void es::SignatureBlock::fromBytes(const byte_t* src, size_t size)
{
	clear();

	size_t totalSize = 0;
	size_t sigSize = 0;
	uint32_t signType = 0;

	// try Big Endian sign type
	signType = ((be_uint32_t*)src)->get();
	switch (signType)
	{
	case (sign::SIGN_RSA4096_SHA1): 
	case (sign::SIGN_RSA4096_SHA256):
		totalSize = sizeof(sRsa4096SignBlock);
		sigSize = crypto::rsa::kRsa4096Size;
		break;
	case (sign::SIGN_RSA2048_SHA1): 
	case (sign::SIGN_RSA2048_SHA256):
		totalSize = sizeof(sRsa2048SignBlock);
		sigSize = crypto::rsa::kRsa2048Size;
		break;
	case (sign::SIGN_ECDSA240_SHA1): 
	case (sign::SIGN_ECDSA240_SHA256):
		totalSize = sizeof(sEcdsa240SignBlock);
		sigSize = sign::kEcdsaSigSize;
		break;
	}

	// try Big Endian sign type
	if (totalSize == 0)
	{
		signType = ((le_uint32_t*)src)->get();
		switch (signType)
		{
		case (sign::SIGN_RSA4096_SHA1): 
		case (sign::SIGN_RSA4096_SHA256):
			totalSize = sizeof(sRsa4096SignBlock);
			sigSize = crypto::rsa::kRsa4096Size;
			break;
		case (sign::SIGN_RSA2048_SHA1): 
		case (sign::SIGN_RSA2048_SHA256):
			totalSize = sizeof(sRsa2048SignBlock);
			sigSize = crypto::rsa::kRsa2048Size;
			break;
		case (sign::SIGN_ECDSA240_SHA1): 
		case (sign::SIGN_ECDSA240_SHA256):
			totalSize = sizeof(sEcdsa240SignBlock);
			sigSize = sign::kEcdsaSigSize;
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown signature type");
		}

		mIsLittleEndian = true;
	}

	if (totalSize > size)
	{
		throw fnd::Exception(kModuleName, "Certificate too small");
	}

	mRawBinary.alloc(totalSize);
	memcpy(mRawBinary.data(), src, totalSize);

	mSignType = (sign::SignType)signType;
	mSignature.alloc(sigSize);
	memcpy(mSignature.data(), mRawBinary.data() + 4, sigSize);
}

const fnd::Vec<byte_t>& es::SignatureBlock::getBytes() const
{
	return mRawBinary;
}

void es::SignatureBlock::clear()
{
	mRawBinary.clear();
	mSignType = sign::SIGN_RSA4096_SHA1;
	mIsLittleEndian = false;
	mSignature.clear();
}

es::sign::SignType es::SignatureBlock::getSignType() const
{
	return mSignType;
}

void es::SignatureBlock::setSignType(es::sign::SignType type)
{
	mSignType = type;
}

bool es::SignatureBlock::isLittleEndian() const
{
	return mIsLittleEndian;
}

void es::SignatureBlock::setLittleEndian(bool isLE)
{
	mIsLittleEndian = isLE;
}

const fnd::Vec<byte_t>& es::SignatureBlock::getSignature() const
{
	return mSignature;
}

void es::SignatureBlock::setSignature(const fnd::Vec<byte_t>& signature)
{
	mSignature = signature;
}
