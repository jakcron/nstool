#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/aes.h>

namespace nn
{
namespace hac
{
	class AesKeygen
	{
	public:

		// 1 stage key generation dst = src_key.decrypt(src)
		static void generateKey(byte_t* dst, const byte_t* src, const byte_t* src_key);
		// 2 stage key generation dst = (src1_key.decrypt(src1)).decrypt(src2)
		static void generateKey(byte_t* dst, const byte_t* src1, const byte_t* src2, const byte_t* src1_key);
		// 3 stage key generation dst = ((src1_key.decrypt(src1)).decrypt(src2)).decrypt(src3)
		static void generateKey(byte_t* dst, const byte_t* src1, const byte_t* src2, const byte_t* src3, const byte_t* src1_key);
	};
}
}