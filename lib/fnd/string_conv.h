#pragma once
#include <stdexcept>
#include <string>
#include <cstdint>

namespace fnd
{
	class StringConv
	{
	public:
		static std::u16string ConvertChar8ToChar16(const std::string& in);
		static std::string ConvertChar16ToChar8(const std::u16string& in);

	private:
		static const char32_t kUtf16EncodeMax = 0x10FFFF;
		static const char32_t kUtf16NonNativeStart = 0x10000;
		static const char16_t kUtf16SurrogateBits = 10;
		static const char16_t kUtf16SurrogateMask = (1 << kUtf16SurrogateBits) - 1;
		static const char16_t kUtf16HighSurrogateStart = 0xD800;
		static const char16_t kUtf16HighSurrogateEnd = kUtf16HighSurrogateStart | kUtf16SurrogateMask;
		static const char16_t kUtf16LowSurrogateStart = 0xDC00;
		static const char16_t kUtf16LowSurrogateEnd = kUtf16LowSurrogateStart | kUtf16SurrogateMask;

		static const char32_t kUtf8AsciiStart = 0x00;
		static const char32_t kUtf8AsciiEnd = 0x7F;
		static const char32_t kUtf82ByteStart = 0x80;
		static const char32_t kUtf82ByteEnd = 0x7FF;
		static const char32_t kUtf83ByteStart = 0x800;
		static const char32_t kUtf83ByteEnd = 0x7FFF;
		static const char32_t kUtf84ByteStart = 0x8000;
		static const char32_t kUtf84ByteEnd = 0x10FFFF;


		static inline uint8_t make_utf8_prefix(uint8_t prefix_bits) { return ((uint8_t)(-1)) << (8 - prefix_bits); }
		static inline uint8_t make_utf8_mask(uint8_t prefix_bits) { return ((uint8_t)(-1)) >> (prefix_bits + 1); }
		static inline uint8_t make_utf8(uint8_t prefix_bits, uint8_t data) { return make_utf8_prefix(prefix_bits) | (data & make_utf8_mask(prefix_bits)); }
		static inline uint8_t get_utf8_data(uint8_t prefix_bits, uint8_t utf8_chr) { return utf8_chr & make_utf8_mask(prefix_bits); }
		static inline bool utf8_has_prefix(uint8_t prefix_bits, uint8_t utf8_chr) { return ((utf8_chr & make_utf8_prefix(prefix_bits)) == make_utf8_prefix(prefix_bits)) && ((utf8_chr & ~make_utf8_mask(prefix_bits)) == make_utf8_prefix(prefix_bits)); }
		static inline uint8_t get_utf8_prefix(uint8_t utf8_chr)
		{
			uint8_t prefix = 0;
			while ((utf8_chr & (1 << 7)) != 0)
			{
				utf8_chr <<= 1;
				prefix++;
			}
			return prefix;
		}
	};
}
