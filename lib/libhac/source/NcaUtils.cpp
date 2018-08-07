#include <nn/hac/NcaUtils.h>

void nn::hac::NcaUtils::decryptNcaHeader(const byte_t* src, byte_t* dst, const fnd::aes::sAesXts128Key& key)
{
	byte_t tweak[fnd::aes::kAesBlockSize];

	// decrypt main header
	byte_t raw_hdr[nn::hac::nca::kSectorSize];
	fnd::aes::AesXtsMakeTweak(tweak, 1);
	fnd::aes::AesXtsDecryptSector(src + sectorToOffset(1), nn::hac::nca::kSectorSize, key.key[0], key.key[1], tweak, raw_hdr);

	bool useNca2SectorIndex = ((nn::hac::sNcaHeader*)(raw_hdr))->st_magic.get() == nn::hac::nca::kNca2StructMagic;

	// decrypt whole header
	for (size_t i = 0; i < nn::hac::nca::kHeaderSectorNum; i++)
	{
		fnd::aes::AesXtsMakeTweak(tweak, (i > 1 && useNca2SectorIndex)? 0 : i);
		fnd::aes::AesXtsDecryptSector(src + sectorToOffset(i), nn::hac::nca::kSectorSize, key.key[0], key.key[1], tweak, dst + sectorToOffset(i));
	}
}

byte_t nn::hac::NcaUtils::getMasterKeyRevisionFromKeyGeneration(byte_t key_generation)
{
	byte_t masterkey_rev;

	switch (key_generation)
	{
		case(0):
		case(1):
			masterkey_rev = 0;
			break;
		case(2):
			masterkey_rev = 1;
			break;
		case(3):
			masterkey_rev = 2;
			break;
		case(4):
			masterkey_rev = 3;
			break;
		case(5):
			masterkey_rev = 4;
			break;
		default:
			masterkey_rev = key_generation - 1;
	}

	return masterkey_rev;
}

void nn::hac::NcaUtils::getNcaPartitionAesCtr(const nn::hac::sNcaFsHeader* hdr, byte_t* ctr)
{
	for (size_t i = 0; i < 8; i++)
	{
		ctr[7-i] = hdr->aes_ctr_upper[i];
		ctr[15-i] = 0;
	}
}