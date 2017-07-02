#include "crypto.h"
#include "polarssl/aes.h"
#include "polarssl/sha1.h"
#include "polarssl/sha2.h"
#include "polarssl/rsa.h"

using namespace crypto;

int GetWrappedHashType(HashType type)
{
	switch (type)
	{
	case crypto::HASH_SHA1:
		return SIG_RSA_SHA1;
		break;
	case crypto::HASH_SHA256:
		return SIG_RSA_SHA256;
		break;
	default:
		return SIG_RSA_RAW;
		break;
	}
	return 0;
}

uint32_t GetWrappedHashSize(HashType type)
{
	uint32_t size = 0;

	switch (type)
	{
	case crypto::HASH_SHA1:
		size = kSha1HashLen;
		break;
	case crypto::HASH_SHA256:
		size = kSha256HashLen;
		break;
	default:
		break;
	}
	return size;
}

inline uint32_t getbe32(const uint8_t* data) { return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]; }
inline void putbe32(uint8_t* data, uint32_t val) { data[0] = val >> 24; data[1] = val >> 16; data[2] = val >> 8; data[3] = val; }

void crypto::Sha1(const uint8_t* in, uint64_t size, uint8_t hash[kSha1HashLen])
{
	sha1(in, size, hash);
}

void crypto::Sha256(const uint8_t* in, uint64_t size, uint8_t hash[kSha256HashLen])
{
	sha2(in, size, hash, false);
}

void crypto::AesCtr(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t ctr[kAesBlockSize], uint8_t* out)
{
	aes_context ctx;
	uint8_t block[kAesBlockSize] = { 0 };
	size_t counterOffset = 0;

	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_ctr(&ctx, size, &counterOffset, ctr, block, in, out);
}

void crypto::AesIncrementCounter(const uint8_t in[kAesBlockSize], size_t block_num, uint8_t out[kAesBlockSize])
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
			ctr[i] += block_num;
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

void crypto::AesCbcDecrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out)
{
	aes_context ctx;
	aes_setkey_dec(&ctx, key, 128);
	aes_crypt_cbc(&ctx, AES_DECRYPT, size, iv, in, out);
}

void crypto::AesCbcEncrypt(const uint8_t* in, uint64_t size, const uint8_t key[kAes128KeySize], uint8_t iv[kAesBlockSize], uint8_t* out)
{
	aes_context ctx;
	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_cbc(&ctx, AES_ENCRYPT, size, iv, in, out);
}

int crypto::RsaSign(const sRsa1024Key & key, HashType hash_type, const uint8_t * hash, uint8_t signature[kRsa1024Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa1024Size;
	mpi_read_binary(&ctx.D, key.priv_exponent, ctx.len);
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, GetWrappedHashType(hash_type), GetWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int crypto::RsaVerify(const sRsa1024Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa1024Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa1024Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, GetWrappedHashType(hash_type), GetWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int crypto::RsaSign(const sRsa2048Key & key, HashType hash_type, const uint8_t * hash, uint8_t signature[kRsa2048Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.D, key.priv_exponent, ctx.len);
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, GetWrappedHashType(hash_type), GetWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int crypto::RsaVerify(const sRsa2048Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa2048Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, GetWrappedHashType(hash_type), GetWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int crypto::RsaSign(const sRsa4096Key & key, HashType hash_type, const uint8_t * hash, uint8_t signature[kRsa4096Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa4096Size;
	mpi_read_binary(&ctx.D, key.priv_exponent, ctx.len);
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, GetWrappedHashType(hash_type), GetWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int crypto::RsaVerify(const sRsa4096Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa4096Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa4096Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, GetWrappedHashType(hash_type), GetWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int crypto::EcdsaSign(const sEcc240PrivateKey & key, HashType hash_type, const uint8_t * hash, sEcc240Point & signature)
{
	return 1;
}

int crypto::EcdsaVerify(const sEcc240Point& key, HashType hash_type, const uint8_t* hash, const sEcc240Point& signature)
{
	return 1;
}


