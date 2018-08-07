#include <fnd/aes.h>
#include <polarssl/aes.h>

using namespace fnd::aes;

inline void XorBlock(const uint8_t a[kAesBlockSize], const uint8_t b[kAesBlockSize], uint8_t out[kAesBlockSize])
{
	for (uint8_t i = 0; i < 16; i++)
	{
		out[i] = a[i] ^ b[i];
	}
}

inline uint32_t getbe32(const uint8_t* data) { return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]; }
inline void putbe32(uint8_t* data, uint32_t val) { data[0] = val >> 24; data[1] = val >> 16; data[2] = val >> 8; data[3] = val; }



void fnd::aes::AesEcbDecrypt(const uint8_t * in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t * out)
{
	aes_context ctx;
	aes_setkey_dec(&ctx, key, 128);
	
	for (size_t i = 0; i < size / kAesBlockSize; i++)
	{
		aes_crypt_ecb(&ctx, AES_DECRYPT, in + kAesBlockSize * i, out + kAesBlockSize * i);
	}
	
}

void fnd::aes::AesEcbEncrypt(const uint8_t * in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t * out)
{
	aes_context ctx;
	aes_setkey_enc(&ctx, key, 128);
	for (size_t i = 0; i < size / kAesBlockSize; i++)
	{
		aes_crypt_ecb(&ctx, AES_ENCRYPT, in + kAesBlockSize * i, out + kAesBlockSize * i);
	}
}

void fnd::aes::AesCtr(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t ctr[kAesBlockSize], uint8_t* out)
{
	aes_context ctx;
	uint8_t block[kAesBlockSize] = { 0 };
	size_t counterOffset = 0;

	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_ctr(&ctx, size, &counterOffset, ctr, block, in, out);
}

void fnd::aes::AesIncrementCounter(const uint8_t in[kAesBlockSize], size_t block_num, uint8_t out[kAesBlockSize])
{
	memcpy(out, in, kAesBlockSize);

	uint32_t ctr[4];
	ctr[3] = getbe32(&in[0]);
	ctr[2] = getbe32(&in[4]);
	ctr[1] = getbe32(&in[8]);
	ctr[0] = getbe32(&in[12]);

	for (uint32_t i = 0; i < 4; i++) {
		uint64_t total = ctr[i] + block_num;
		// if there wasn't a wrap around, add the two together and exit
		if (total <= 0xffffffff) {
			ctr[i] += (uint32_t)block_num;
			break;
		}

		// add the difference
		ctr[i] = (uint32_t)(total - 0x100000000);
		// carry to next word
		block_num = (uint32_t)(total >> 32);
	}

	putbe32(&out[0], ctr[3]);
	putbe32(&out[4], ctr[2]);
	putbe32(&out[8], ctr[1]);
	putbe32(&out[12], ctr[0]);
}

void fnd::aes::AesCbcDecrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out)
{
	aes_context ctx;
	aes_setkey_dec(&ctx, key, 128);
	aes_crypt_cbc(&ctx, AES_DECRYPT, size, iv, in, out);
}

void fnd::aes::AesCbcEncrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out)
{
	aes_context ctx;
	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_cbc(&ctx, AES_ENCRYPT, size, iv, in, out);
}

void fnd::aes::AesXtsDecryptSector(const uint8_t * in, uint64_t sector_size, const uint8_t key1[kAes128KeySize], const uint8_t key2[kAes128KeySize], uint8_t tweak[kAesBlockSize], uint8_t * out)
{
	aes_context data_ctx;
	aes_setkey_dec(&data_ctx, key1, 128);

	uint8_t enc_tweak[kAesBlockSize];
	AesEcbEncrypt(tweak, kAesBlockSize, key2, enc_tweak);

	size_t block_num = sector_size / kAesBlockSize;
	uint8_t block[kAesBlockSize];
	for (size_t i = 0; i < block_num; i++)
	{
		XorBlock(in + (i * kAesBlockSize), enc_tweak, block);
		aes_crypt_ecb(&data_ctx, AES_DECRYPT, block, block);
		XorBlock(block, enc_tweak, out + i * kAesBlockSize);
		GaloisFunc(enc_tweak);
	}

	if (sector_size % kAesBlockSize)
	{
		// TODO: implement ciphertext stealing
	}
}

void fnd::aes::AesXtsEncryptSector(const uint8_t * in, uint64_t sector_size, const uint8_t key1[kAes128KeySize], const uint8_t key2[kAes128KeySize], uint8_t tweak[kAesBlockSize], uint8_t * out)
{
	aes_context data_ctx;
	aes_setkey_enc(&data_ctx, key1, 128);

	uint8_t enc_tweak[kAesBlockSize];
	AesEcbEncrypt(tweak, kAesBlockSize, key2, enc_tweak);

	size_t block_num = sector_size / kAesBlockSize;
	uint8_t block[kAesBlockSize];
	for (size_t i = 0; i < block_num; i++)
	{
		XorBlock(in + (i * kAesBlockSize), enc_tweak, block);
		aes_crypt_ecb(&data_ctx, AES_ENCRYPT, block, block);
		XorBlock(block, enc_tweak, out + (i * kAesBlockSize));
		GaloisFunc(enc_tweak);
	}

	if (sector_size % kAesBlockSize)
	{
		// TODO: implement ciphertext stealing
	}
}

void fnd::aes::AesXtsMakeTweak(uint8_t tweak[kAesBlockSize], size_t block_index)
{
	memset(tweak, 0, kAesBlockSize);
	AesIncrementCounter(tweak, block_index, tweak);
}

void fnd::aes::GaloisFunc(uint8_t x[kAesBlockSize])
{
	uint8_t t = x[15];

	for (uint8_t i = 15; i > 0; i--)
	{
		x[i] = (x[i] << 1) | (x[i - 1] & 0x80 ? 1 : 0);
	}

	x[0] = (x[0] << 1) ^ (t & 0x80 ? 0x87 : 0x00);
}