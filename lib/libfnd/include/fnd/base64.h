#pragma once
#include <cstdint>
#include <cstring>

namespace fnd
{
namespace base64
{
	size_t B64_GetEncodeLen(const uint8_t* src, size_t slen);
	void B64_Encode(const uint8_t* src, size_t slen, uint8_t* dst, size_t dlen);
	size_t B64_GetDecodeLen(const uint8_t* src, size_t slen);
	void B64_Decode(const uint8_t* src, size_t slen, uint8_t* dst, size_t dlen);
}
}