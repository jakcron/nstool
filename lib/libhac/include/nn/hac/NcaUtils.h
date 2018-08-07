#pragma once
#include <nn/hac/nca.h>

namespace nn
{
namespace hac
{
	class NcaUtils
	{
	public:
		static inline size_t sectorToOffset(size_t sector_index) { return sector_index * nn::hac::nca::kSectorSize; }
		static void decryptNcaHeader(const byte_t* src, byte_t* dst, const fnd::aes::sAesXts128Key& key);
		static byte_t getMasterKeyRevisionFromKeyGeneration(byte_t key_generation);
		static void getNcaPartitionAesCtr(const nn::hac::sNcaFsHeader* hdr, byte_t* ctr);
	};
}
}