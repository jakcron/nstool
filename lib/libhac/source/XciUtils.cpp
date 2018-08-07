#include <nn/hac/XciUtils.h>

void nn::hac::XciUtils::getXciHeaderAesIv(const nn::hac::sXciHeader* hdr, byte_t* iv)
{
	for (size_t i = 0; i < 16; i++)
	{
		iv[15-i] = hdr->aescbc_iv.iv[i];
	}
}

void nn::hac::XciUtils::decryptXciHeader(const byte_t* src, byte_t* dst, const byte_t* key)
{
	byte_t iv[fnd::aes::kAesBlockSize];

	getXciHeaderAesIv((const nn::hac::sXciHeader*)src, iv);

	// copy plain
	memcpy(dst, src, nn::hac::xci::kHeaderEncOffset);

	// decrypt encrypted data
	fnd::aes::AesCbcDecrypt(src + nn::hac::xci::kHeaderEncOffset, nn::hac::xci::kHeaderEncSize, key, iv, dst + nn::hac::xci::kHeaderEncOffset);
}