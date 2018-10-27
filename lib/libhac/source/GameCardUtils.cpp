#include <nn/hac/GameCardUtils.h>

void nn::hac::GameCardUtils::getXciHeaderAesIv(const nn::hac::sGcHeader* hdr, byte_t* iv)
{
	for (size_t i = 0; i < 16; i++)
	{
		iv[15-i] = hdr->aescbc_iv.iv[i];
	}
}

void nn::hac::GameCardUtils::decryptXciHeader(const byte_t* src, byte_t* dst, const byte_t* key)
{
	byte_t iv[fnd::aes::kAesBlockSize];

	getXciHeaderAesIv((const nn::hac::sGcHeader*)src, iv);

	// copy plain
	memcpy(dst, src, nn::hac::gc::kHeaderEncOffset);

	// decrypt encrypted data
	fnd::aes::AesCbcDecrypt(src + nn::hac::gc::kHeaderEncOffset, nn::hac::gc::kHeaderEncSize, key, iv, dst + nn::hac::gc::kHeaderEncOffset);
}