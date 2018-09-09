#pragma once
#include <cstdint>
#include <cstring>
#include <fnd/sha.h>

namespace fnd
{
	namespace rsa
	{
		const size_t kRsa1024Size = 0x80;
		const size_t kRsa2048Size = 0x100;
		const size_t kRsa4096Size = 0x200;
		const size_t kRsaPublicExponentSize = 4;

		enum RsaType
		{
			RSA_1024,
			RSA_2048,
			RSA_4096,
		};

#pragma pack (push, 1)
		struct sRsa1024Key
		{
			uint8_t modulus[kRsa1024Size];
			uint8_t priv_exponent[kRsa1024Size];
			uint8_t public_exponent[kRsaPublicExponentSize];

			void operator=(const sRsa1024Key& other)
			{
				memcpy(this->modulus, modulus, kRsa1024Size);
				memcpy(this->priv_exponent, priv_exponent, kRsa1024Size);
				memcpy(this->public_exponent, other.public_exponent, kRsaPublicExponentSize);
			}

			bool operator==(const sRsa1024Key& other) const
			{
				return memcmp(this->modulus, other.modulus, kRsa1024Size) == 0 \
					&& memcmp(this->priv_exponent, other.priv_exponent, kRsa1024Size) == 0 \
					&& memcmp(this->public_exponent, other.public_exponent, kRsaPublicExponentSize) == 0;
			}

			bool operator!=(const sRsa1024Key& other) const
			{
				return !operator==(other);
			}
		};

		struct sRsa2048Key
		{
			uint8_t modulus[kRsa2048Size];
			uint8_t priv_exponent[kRsa2048Size];
			uint8_t public_exponent[kRsaPublicExponentSize];

			void operator=(const sRsa2048Key& other)
			{
				memcpy(this->modulus, other.modulus, kRsa2048Size);
				memcpy(this->priv_exponent, other.priv_exponent, kRsa2048Size);
				memcpy(this->public_exponent, other.public_exponent, kRsaPublicExponentSize);
			}

			bool operator==(const sRsa2048Key& other) const
			{
				return memcmp(this->modulus, other.modulus, kRsa2048Size) == 0 && memcmp(this->priv_exponent, other.priv_exponent, kRsa2048Size) == 0 && memcmp(this->public_exponent, other.public_exponent, kRsaPublicExponentSize) == 0;
			}

			bool operator!=(const sRsa2048Key& other) const
			{
				return !operator==(other);
			}
		};

		struct sRsa4096Key
		{
			uint8_t modulus[kRsa4096Size];
			uint8_t priv_exponent[kRsa4096Size];
			uint8_t public_exponent[kRsaPublicExponentSize];

			void operator=(const sRsa4096Key& other)
			{
				memcpy(this->modulus, other.modulus, kRsa4096Size);
				memcpy(this->priv_exponent, other.priv_exponent, kRsa4096Size);
				memcpy(this->public_exponent, other.public_exponent, kRsaPublicExponentSize);
			}

			bool operator==(const sRsa4096Key& other) const
			{
				return memcmp(this->modulus, other.modulus, kRsa4096Size) == 0 && memcmp(this->priv_exponent, other.priv_exponent, kRsa4096Size) == 0 && memcmp(this->public_exponent, other.public_exponent, kRsaPublicExponentSize) == 0;
			}

			bool operator!=(const sRsa4096Key& other) const
			{
				return !operator==(other);
			}
		};
#pragma pack (pop)

		namespace pkcs
		{
			// rsa1024
			int rsaSign(const sRsa1024Key& key, sha::HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa1024Size]);
			int rsaVerify(const sRsa1024Key& key, sha::HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa1024Size]);
			// rsa2048
			int rsaSign(const sRsa2048Key& key, sha::HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa2048Size]);
			int rsaVerify(const sRsa2048Key& key, sha::HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa2048Size]);
			// rsa4096
			int rsaSign(const sRsa4096Key& key, sha::HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa4096Size]);
			int rsaVerify(const sRsa4096Key& key, sha::HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa4096Size]);
		}

		namespace pss
		{
			// rsa1024
			//int rsaSign(const sRsa1024Key& key, sha::HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa1024Size]);
			//int rsaVerify(const sRsa1024Key& key, sha::HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa1024Size]);
			// rsa2048
			int rsaSign(const sRsa2048Key& key, sha::HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa2048Size]);
			int rsaVerify(const sRsa2048Key& key, sha::HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa2048Size]);
			// rsa4096
			//int rsaSign(const sRsa4096Key& key, sha::HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa4096Size]);
			//int rsaVerify(const sRsa4096Key& key, sha::HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa4096Size]);
		}
	}
}
