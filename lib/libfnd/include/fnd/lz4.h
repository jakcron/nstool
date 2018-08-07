#pragma once
#include <string>

namespace fnd
{
	namespace lz4
	{
		void compressData(const uint8_t* src, uint32_t src_len, uint8_t* dst, uint32_t dst_capacity, uint32_t& compressed_size);
		void decompressData(const uint8_t* src, uint32_t src_len, uint8_t* dst, uint32_t dst_capacity, uint32_t& decompressed_size);
	}
}