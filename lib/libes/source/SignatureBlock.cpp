#include <es/SignatureBlock.h>

es::SignatureBlock::SignatureBlock()
{
	clear();
}

es::SignatureBlock::SignatureBlock(const SignatureBlock& other)
{
	copyFrom(other);
}

void es::SignatureBlock::operator=(const SignatureBlock& other)
{
	copyFrom(other);
}

bool es::SignatureBlock::operator==(const SignatureBlock& other) const
{
	return isEqual(other);
}

bool es::SignatureBlock::operator!=(const SignatureBlock& other) const
{
	return !(*this == other);
}

void es::SignatureBlock::importBinary(byte_t* src, size_t size)
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

	mBinaryBlob.alloc(totalSize);
	memcpy(mBinaryBlob.getBytes(), src, totalSize);

	mSignType = (sign::SignType)signType;
	mSignature.alloc(sigSize);
	memcpy(mSignature.getBytes(), mBinaryBlob.getBytes() + 4, sigSize);
}

void es::SignatureBlock::exportBinary()
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

	if (mSignature.getSize() != sigSize)
		throw fnd::Exception(kModuleName, "Signature size is incorrect");
	
	// commit to binary
	mBinaryBlob.alloc(totalSize);
	if (mIsLittleEndian)
		*(le_uint32_t*)(mBinaryBlob.getBytes()) = mSignType;
	else
		*(be_uint32_t*)(mBinaryBlob.getBytes()) = mSignType;
	memcpy(mBinaryBlob.getBytes() + 4, mSignature.getBytes(), sigSize);
}

const byte_t* es::SignatureBlock::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t es::SignatureBlock::getSize() const
{
	return mBinaryBlob.getSize();
}

void es::SignatureBlock::clear()
{
	mBinaryBlob.clear();
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

const fnd::MemoryBlob& es::SignatureBlock::getSignature() const
{
	return mSignature;
}

void es::SignatureBlock::setSignature(const fnd::MemoryBlob& signature)
{
	mSignature = signature;
}

bool es::SignatureBlock::isEqual(const SignatureBlock& other) const
{
	return (mSignType == other.mSignType) \
		&& (mIsLittleEndian == other.mIsLittleEndian) \
		&& (mSignature == other.mSignature);
}

void es::SignatureBlock::copyFrom(const SignatureBlock& other)
{
	mBinaryBlob = other.mBinaryBlob;
	mSignType = other.mSignType;
	mIsLittleEndian = other.mIsLittleEndian;
	mSignature = other.mSignature;
}
