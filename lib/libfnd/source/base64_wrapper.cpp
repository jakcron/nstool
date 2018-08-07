#include <fnd/base64.h>
#include <polarssl/base64.h>

size_t fnd::base64::B64_GetEncodeLen(const uint8_t* src, size_t slen)
{
	size_t dlen = 0;

	base64_encode(nullptr, &dlen, src, slen);

	return dlen;
}

void fnd::base64::B64_Encode(const uint8_t* src, size_t slen, uint8_t* dst, size_t dlen)
{
	base64_encode(dst, &dlen, src, slen);
}

size_t fnd::base64::B64_GetDecodeLen(const uint8_t* src, size_t slen)
{
	size_t dlen = 0;

	base64_decode(nullptr, &dlen, src, slen);

	return dlen;
}

void fnd::base64::B64_Decode(const uint8_t* src, size_t slen, uint8_t* dst, size_t dlen)
{
	base64_decode(dst, &dlen, src, slen);
}