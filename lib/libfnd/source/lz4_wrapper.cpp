#include <fnd/lz4.h>
#include <lz4.h>

void fnd::lz4::compressData(const uint8_t* src, uint32_t src_len, uint8_t* dst, uint32_t dst_capacity, uint32_t& compressed_size)
{
	compressed_size = LZ4_compress_default((const char*)src, (char*)dst, (int)src_len, (int)dst_capacity);
}

void fnd::lz4::decompressData(const uint8_t* src, uint32_t src_len, uint8_t* dst, uint32_t dst_capacity, uint32_t& decompressed_size)
{
	decompressed_size = LZ4_decompress_safe((const char*)src, (char*)dst, (int)src_len, (int)dst_capacity);
}

 