#pragma once
#include <nx/nca.h>

namespace nx
{
	class NcaUtils
	{
	public:
		static inline size_t sectorToOffset(size_t sector_index) { return sector_index * nx::nca::kSectorSize; }
		static void decryptNcaHeader(const byte_t* src, byte_t* dst, const crypto::aes::sAesXts128Key& key);
		static byte_t getMasterKeyRevisionFromKeyGeneration(byte_t key_generation);
		static void getNcaPartitionAesCtr(const nx::sNcaFsHeader* hdr, byte_t* ctr);
	};
}