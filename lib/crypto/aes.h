#pragma once
#include <cstdint>
#include <cstring>

namespace crypto
{
namespace aes
{
	const size_t kAes128KeySize = 0x10;
	const size_t kAesBlockSize = 0x10;
	const size_t KAesCcmNonceSize = 0xc;

#pragma pack (push, 1)
	struct sAes128Key
	{
		uint8_t key[kAes128KeySize];

		void set(const uint8_t key[kAes128KeySize])
		{
			memcpy(this->key, key, kAes128KeySize);
		}

		bool compare(const sAes128Key& other) const
		{
			return memcmp(this->key, other.key, kAes128KeySize) == 0;
		}

		void operator=(const sAes128Key& other)
		{
			set(other.key);
		}
		bool operator==(const sAes128Key& other) const
		{
			return compare(other);
		}
		bool operator!=(const sAes128Key& other) const
		{
			return !compare(other);
		}
		
	};

	struct sAesXts128Key
	{
		sAes128Key key1;
		sAes128Key key2;
	};

	struct sAesIvCtr
	{
		uint8_t iv[kAesBlockSize];

		void set(const uint8_t iv[kAes128KeySize])
		{
			memcpy(this->iv, iv, kAes128KeySize);
		}

		void operator=(const sAesIvCtr& other)
		{
			set(other.iv);
		}
	};
#pragma pack (pop)

	// aes-128
	void AesEcbDecrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t* out);
	void AesEcbEncrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t* out);

	void AesCtr(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t ctr[kAesBlockSize], uint8_t* out);
	void AesIncrementCounter(const uint8_t in[kAesBlockSize], size_t block_num, uint8_t out[kAesBlockSize]);

	void AesCbcDecrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out);
	void AesCbcEncrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out);

	void AesXtsDecryptSector(const uint8_t* in, uint64_t sector_size, const uint8_t key1[kAes128KeySize], const uint8_t key2[kAes128KeySize], uint8_t tweak[kAesBlockSize], uint8_t* out);
	void AesXtsEncryptSector(const uint8_t* in, uint64_t sector_size, const uint8_t key1[kAes128KeySize], const uint8_t key2[kAes128KeySize], uint8_t tweak[kAesBlockSize], uint8_t* out);
	void AesXtsMakeTweak(uint8_t tweak[kAesBlockSize], uint64_t block_index);

	void GaloisFunc(uint8_t x[kAesBlockSize]);
}
}