#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/rsa.h>
#include <crypto/ecdsa.h>

namespace es
{
	namespace sign
	{
		enum SignType
		{
			SIGN_RSA4096_SHA1 = 0x10000,
			SIGN_RSA2048_SHA1,
			SIGN_ECDSA240_SHA1,
			SIGN_RSA4096_SHA256,
			SIGN_RSA2048_SHA256,
			SIGN_ECDSA240_SHA256,
		};

		static const size_t kEcdsaSigSize = 0x3C;
	}
#pragma pack(push,1)
	struct sRsa4096SignBlock
	{
		be_uint32_t sign_type;
		byte_t signature[crypto::rsa::kRsa4096Size];
		byte_t padding[0x3C];
	};

	struct sRsa2048SignBlock
	{
		be_uint32_t sign_type;
		byte_t signature[crypto::rsa::kRsa2048Size];
		byte_t padding[0x3C];
	};

	struct sEcdsa240SignBlock
	{
		be_uint32_t sign_type;
		byte_t signature[sign::kEcdsaSigSize];
		byte_t padding[0x40];
	};
#pragma pack(pop)
}