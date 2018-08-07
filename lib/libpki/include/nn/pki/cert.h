#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/rsa.h>
#include <crypto/ecdsa.h>

namespace nn
{
namespace pki
{
	namespace cert
	{
		enum PublicKeyType
		{
			RSA4096,
			RSA2048,
			ECDSA240
		};

		static const size_t kIssuerSize = 0x40;
		static const size_t kSubjectSize = 0x40;
	}
#pragma pack(push,1)
	struct sCertificateHeader
	{
		char issuer[cert::kIssuerSize];
		be_uint32_t key_type;
		char subject[cert::kSubjectSize];
		be_uint32_t cert_id;
	};

	struct sRsa4096PublicKeyBlock
	{
		byte_t modulus[crypto::rsa::kRsa4096Size];
		byte_t public_exponent[crypto::rsa::kRsaPublicExponentSize];
		byte_t padding[0x34];
	};

	struct sRsa2048PublicKeyBlock
	{
		byte_t modulus[crypto::rsa::kRsa2048Size];
		byte_t public_exponent[crypto::rsa::kRsaPublicExponentSize];
		byte_t padding[0x34];
	};

	struct sEcdsa240PublicKeyBlock
	{
		crypto::ecdsa::sEcdsa240Point public_key;
		byte_t padding[0x3C];
	};
#pragma pack(pop)
}
}