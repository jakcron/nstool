#include <nx/XciUtils.h>

void nx::XciUtils::getXciHeaderAesIv(const nx::sXciHeader* hdr, byte_t* iv)
{
	for (size_t i = 0; i < 16; i++)
	{
		iv[15-i] = hdr->aescbc_iv.iv[i];
	}
}

void nx::XciUtils::decryptXciHeader(const byte_t* src, byte_t* dst, const byte_t* key)
{
	byte_t iv[crypto::aes::kAesBlockSize];

	getXciHeaderAesIv((const nx::sXciHeader*)src, iv);

	// copy plain
	memcpy(dst, src, nx::xci::kHeaderEncOffset);

	// decrypt encrypted data
	crypto::aes::AesCbcDecrypt(src + nx::xci::kHeaderEncOffset, nx::xci::kHeaderEncSize, key, iv, dst + nx::xci::kHeaderEncOffset);
}