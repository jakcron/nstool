#include <nn/hac/AesKeygen.h>

void nn::hac::AesKeygen::generateKey(byte_t* dst, const byte_t* src, const byte_t* src_key)
{
	fnd::aes::AesEcbDecrypt(src, sizeof(fnd::aes::sAes128Key), src_key, dst);
}

void nn::hac::AesKeygen::generateKey(byte_t* dst, const byte_t* src1, const byte_t* src2, const byte_t* src1_key)
{
	fnd::aes::sAes128Key src2_key;
	generateKey(src2_key.key, src1, src1_key);
	generateKey(dst, src2, src2_key.key);
}
void nn::hac::AesKeygen::generateKey(byte_t* dst, const byte_t* src1, const byte_t* src2, const byte_t* src3, const byte_t* src1_key)
{
	fnd::aes::sAes128Key src3_key;
	generateKey(src3_key.key, src1, src2, src1_key);
	generateKey(dst, src3, src3_key.key);
}