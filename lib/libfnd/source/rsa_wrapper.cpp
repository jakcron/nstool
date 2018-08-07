#include <fnd/rsa.h>
#include <polarssl/rsa.h>
#include <polarssl/md.h>

using namespace fnd::rsa;
using namespace fnd::sha;

int getWrappedHashType(HashType type)
{
	switch (type)
	{
	case HASH_SHA1:
		return SIG_RSA_SHA1;
		break;
	case HASH_SHA256:
		return SIG_RSA_SHA256;
		break;
	default:
		return SIG_RSA_RAW;
		break;
	}
	return 0;
}

int getMdWrappedHashType(HashType type)
{
	switch (type)
	{
	case HASH_SHA1:
		return POLARSSL_MD_SHA1;
		break;
	case HASH_SHA256:
		return POLARSSL_MD_SHA256;
		break;
	default:
		return POLARSSL_MD_NONE;
		break;
	}
	return 0;
}

uint32_t getWrappedHashSize(HashType type)
{
	uint32_t size = 0;

	switch (type)
	{
	case HASH_SHA1:
		size = kSha1HashLen;
		break;
	case HASH_SHA256:
		size = kSha256HashLen;
		break;
	default:
		break;
	}
	return size;
}

int fnd::rsa::pkcs::rsaSign(const sRsa1024Key & key, HashType hash_type, const uint8_t * hash, uint8_t signature[kRsa1024Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa1024Size;
	mpi_read_binary(&ctx.D, key.priv_exponent, ctx.len);
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int fnd::rsa::pkcs::rsaVerify(const sRsa1024Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa1024Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa1024Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int fnd::rsa::pkcs::rsaSign(const sRsa2048Key & key, HashType hash_type, const uint8_t * hash, uint8_t signature[kRsa2048Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.D, key.priv_exponent, ctx.len);
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int fnd::rsa::pkcs::rsaVerify(const sRsa2048Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa2048Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int fnd::rsa::pkcs::rsaSign(const sRsa4096Key & key, HashType hash_type, const uint8_t * hash, uint8_t signature[kRsa4096Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa4096Size;
	mpi_read_binary(&ctx.D, key.priv_exponent, ctx.len);
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int fnd::rsa::pkcs::rsaVerify(const sRsa4096Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa4096Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa4096Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}

int fnd::rsa::pss::rsaVerify(const sRsa2048Key & key, HashType hash_type, const uint8_t * hash, const uint8_t signature[kRsa2048Size])
{
	static const uint8_t public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V21, getMdWrappedHashType(hash_type));

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, key.modulus, ctx.len);

	ret = rsa_rsassa_pss_verify(&ctx, RSA_PUBLIC, getWrappedHashType(hash_type), getWrappedHashSize(hash_type), hash, signature);

	rsa_free(&ctx);

	return ret;
}