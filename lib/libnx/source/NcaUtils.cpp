#include <nx/NcaUtils.h>

void nx::NcaUtils::decryptNcaHeader(const byte_t* src, byte_t* dst, const crypto::aes::sAesXts128Key& key)
{
	byte_t tweak[crypto::aes::kAesBlockSize];

	// decrypt main header
	byte_t raw_hdr[nx::nca::kSectorSize];
	crypto::aes::AesXtsMakeTweak(tweak, 1);
	crypto::aes::AesXtsDecryptSector(src + sectorToOffset(1), nx::nca::kSectorSize, key.key[0], key.key[1], tweak, raw_hdr);

	bool useNca2SectorIndex = ((nx::sNcaHeader*)(raw_hdr))->signature.get() == nx::nca::kNca2Sig;

	// decrypt whole header
	for (size_t i = 0; i < nx::nca::kHeaderSectorNum; i++)
	{
		crypto::aes::AesXtsMakeTweak(tweak, (i > 1 && useNca2SectorIndex)? 0 : i);
		crypto::aes::AesXtsDecryptSector(src + sectorToOffset(i), nx::nca::kSectorSize, key.key[0], key.key[1], tweak, dst + sectorToOffset(i));
	}
}

byte_t nx::NcaUtils::getMasterKeyRevisionFromKeyGeneration(byte_t key_generation)
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

void nx::NcaUtils::getNcaPartitionAesCtr(const nx::sNcaFsHeader* hdr, byte_t* ctr)
{
	for (size_t i = 0; i < 8; i++)
	{
		ctr[7-i] = hdr->aes_ctr_upper[i];
		ctr[15-i] = 0;
	}
}