#include <fnd/sha.h>
#include <polarssl/sha1.h>
#include <polarssl/sha2.h>

using namespace fnd::sha;

void fnd::sha::Sha1(const uint8_t* in, uint64_t size, uint8_t hash[kSha1HashLen])
{
	sha1(in, size, hash);
}

void fnd::sha::Sha256(const uint8_t* in, uint64_t size, uint8_t hash[kSha256HashLen])
{
	sha2(in, size, hash, false);
}