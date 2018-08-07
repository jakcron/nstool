#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/aes.h>
#include <fnd/rsa.h>
#include <fnd/ecdsa.h>

namespace nn
{
namespace pki
{
	namespace sign
	{
		enum SignatureId
		{
			SIGN_ID_RSA4096_SHA1 = 0x10000,
			SIGN_ID_RSA2048_SHA1,
			SIGN_ID_ECDSA240_SHA1,
			SIGN_ID_RSA4096_SHA256,
			SIGN_ID_RSA2048_SHA256,
			SIGN_ID_ECDSA240_SHA256,
		};

		enum SignatureAlgo
		{
			SIGN_ALGO_RSA4096,
			SIGN_ALGO_RSA2048,
			SIGN_ALGO_ECDSA240
		};

		enum HashAlgo
		{
			HASH_ALGO_SHA1,
			HASH_ALGO_SHA256
		};

		static const size_t kEcdsaSigSize = 0x3C;

		static const std::string kRootIssuerStr = "Root";
		static const std::string kIdentDelimiter = "-";
	}
#pragma pack(push,1)
	struct sRsa4096SignBlock
	{
		be_uint32_t sign_type;
		byte_t signature[fnd::rsa::kRsa4096Size];
		byte_t padding[0x3C];
	};

	struct sRsa2048SignBlock
	{
		be_uint32_t sign_type;
		byte_t signature[fnd::rsa::kRsa2048Size];
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
}