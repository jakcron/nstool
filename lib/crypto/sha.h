#pragma once
#include <cstdint>
#include <cstring>

namespace crypto
{
	namespace sha
	{
		const size_t kSha1HashLen = 20;
		const size_t kSha256HashLen = 32;

		enum HashType
		{
			HASH_SHA1,
			HASH_SHA256
		};

#pragma pack (push, 1)
		struct sSha1Hash
		{
			uint8_t bytes[kSha1HashLen];

			void set(const uint8_t hash[kSha1HashLen])
			{
				memcpy(this->bytes, hash, kSha1HashLen);
			}

			void operator=(const sSha1Hash& other)
			{
				set(other.bytes);
			}

			bool compare(const uint8_t hash[kSha1HashLen])
			{
				return memcmp(this->bytes, hash, kSha1HashLen) == 0;
			}

			bool compare(const sSha1Hash& other)
			{
				return memcmp(this->bytes, other.bytes, kSha1HashLen) == 0;
			}

			bool operator==(const sSha1Hash& other)
			{
				return compare(other);
			}
		};

		struct sSha256Hash
		{
			uint8_t bytes[kSha256HashLen];

			void set(const uint8_t hash[kSha256HashLen])
			{
				memcpy(this->bytes, hash, kSha256HashLen);
			}

			void operator=(const sSha256Hash& other)
			{
				set(other.bytes);
			}

			bool compare(const uint8_t hash[kSha256HashLen])
			{
				return memcmp(this->bytes, hash, kSha256HashLen) == 0;
			}

			bool compare(const sSha256Hash& other)
			{
				return memcmp(this->bytes, other.bytes, kSha256HashLen) == 0;
			}

			bool operator==(const sSha256Hash& other)
			{
				return compare(other);
			}
		};
#pragma pack (pop)

		void Sha1(const uint8_t* in, uint64_t size, uint8_t hash[kSha1HashLen]);
		void Sha256(const uint8_t* in, uint64_t size, uint8_t hash[kSha256HashLen]);
	}
}