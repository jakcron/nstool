#pragma once
#include <cstdint>
#include <cstring>

namespace crypto
{
	const size_t kSha1HashLen = 20;
	const size_t kSha256HashLen = 32;
	const size_t kAes128KeySize = 0x10;
	const size_t kAesBlockSize = 0x10;
	const size_t KAesCcmNonceSize = 0xc;
	const size_t kRsa1024Size = 0x80;
	const size_t kRsa2048Size = 0x100;
	const size_t kRsa4096Size = 0x200;
	const size_t kRsaPublicExponentSize = 4;
	const size_t kEcdsaSize = 0x3C;
	const size_t kEcParam240Bit = 0x1E;

	enum HashType
	{
		HASH_SHA1,
		HASH_SHA256
	};

	enum SignType
	{
		SIGN_RSA_1024,
		SIGN_RSA_2048,
		SIGN_RSA_4096,
		SIGN_ECDSA_240,
	};

#pragma pack (push, 1)
	struct sAes128Key
	{
		uint8_t key[kAes128KeySize];

		void set(const uint8_t key[kAes128KeySize])
		{
			memcpy(this->key, key, kAes128KeySize);
		}
	};

	struct sAesIvCtr
	{
		uint8_t iv[kAesBlockSize];
	};

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

		bool operator==(const sRsa1024Key& other)
		{
			return memcmp(this->modulus, other.modulus, kRsa1024Size) == 0 && memcmp(this->priv_exponent, other.priv_exponent, kRsa1024Size) == 0 && memcpy(this->public_exponent, other.public_exponent, kRsaPublicExponentSize) == 0;
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

		bool operator==(const sRsa2048Key& other)
		{
			return memcmp(this->modulus, other.modulus, kRsa2048Size) == 0 && memcmp(this->priv_exponent, other.priv_exponent, kRsa2048Size) == 0 && memcpy(this->public_exponent, other.public_exponent, kRsaPublicExponentSize) == 0;
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

		bool operator==(const sRsa4096Key& other)
		{
			return memcmp(this->modulus, other.modulus, kRsa4096Size) == 0 && memcmp(this->priv_exponent, other.priv_exponent, kRsa4096Size) == 0 && memcpy(this->public_exponent, other.public_exponent, kRsaPublicExponentSize) == 0;
		}
	};

	struct sEcc240Point
	{
		uint8_t r[kEcParam240Bit];
		uint8_t s[kEcParam240Bit];

		void operator=(const sEcc240Point& other)
		{
			memcpy(this->r, other.r, kEcParam240Bit);
			memcpy(this->s, other.s, kEcParam240Bit);
		}

		bool operator==(const sEcc240Point& other)
		{
			return memcmp(this->r, other.r, kEcParam240Bit) == 0 && memcmp(this->s, other.s, kEcParam240Bit) == 0;
		}
	};

	struct sEcc240PrivateKey
	{
		uint8_t k[kEcParam240Bit]; // stub
	};
#pragma pack (pop)

	void Sha1(const uint8_t* in, uint64_t size, uint8_t hash[kSha1HashLen]);
	void Sha256(const uint8_t* in, uint64_t size, uint8_t hash[kSha256HashLen]);

	// aes-128
	void AesCtr(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t ctr[kAesBlockSize], uint8_t* out);
	void AesIncrementCounter(const uint8_t in[kAesBlockSize], size_t block_num, uint8_t out[kAesBlockSize]);

	void AesCbcDecrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out);
	void AesCbcEncrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out);


	// rsa1024
	int RsaSign(const sRsa1024Key& key, HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa1024Size]);
	int RsaVerify(const sRsa1024Key& key, HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa1024Size]);
	// rsa2048
	int RsaSign(const sRsa2048Key& key, HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa2048Size]);
	int RsaVerify(const sRsa2048Key& key, HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa2048Size]);
	// rsa4096
	int RsaSign(const sRsa4096Key& key, HashType hash_type, const uint8_t* hash, uint8_t signature[kRsa4096Size]);
	int RsaVerify(const sRsa4096Key& key, HashType hash_type, const uint8_t* hash, const uint8_t signature[kRsa4096Size]);
	// ecdsa
	int EcdsaSign(const sEcc240PrivateKey& key, HashType hash_type, const uint8_t* hash, sEcc240Point& signature);
	int EcdsaVerify(const sEcc240Point& key, HashType hash_type, const uint8_t* hash, const sEcc240Point& signature);
}