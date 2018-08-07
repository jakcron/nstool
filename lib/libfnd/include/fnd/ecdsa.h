#pragma once
#include <cstdint>
#include <cstring>

namespace fnd
{
	namespace ecdsa
	{
		const size_t kEcdsa240Size = 0x1E;

		enum EcdsaType
		{
			ECDSA_240,
		};

#pragma pack (push, 1)
		struct sEcdsa240Point
		{
			uint8_t r[kEcdsa240Size];
			uint8_t s[kEcdsa240Size];

			void operator=(const sEcdsa240Point& other)
			{
				memcpy(this->r, r, kEcdsa240Size);
				memcpy(this->s, s, kEcdsa240Size);
			}

			bool operator==(const sEcdsa240Point& other) const
			{
				return memcmp(this->r, other.r, kEcdsa240Size) == 0 \
					&& memcmp(this->s, other.s, kEcdsa240Size) == 0;
			}

			bool operator!=(const sEcdsa240Point& other) const
			{
				return !operator==(other);
			}
		};

		struct sEcdsa240PrivateKey
		{
			uint8_t k[kEcdsa240Size];

			void operator=(const sEcdsa240PrivateKey& other)
			{
				memcpy(this->k, k, kEcdsa240Size);
			}

			bool operator==(const sEcdsa240PrivateKey& other) const
			{
				return memcmp(this->k, other.k, kEcdsa240Size) == 0;
			}

			bool operator!=(const sEcdsa240PrivateKey& other) const
			{
				return !operator==(other);
			}
		};
#pragma pack (pop)
	}
}
