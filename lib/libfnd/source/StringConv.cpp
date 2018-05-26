#include <fnd/StringConv.h>
#include <vector>
#include <string>

using namespace fnd;

std::u16string StringConv::ConvertChar8ToChar16(const std::string & in)
{
	std::u32string unicode;
	size_t done = 0;
	for (size_t i = 0; i < in.length(); i += done)
	{
		// get number of leading high bits in first byte
		uint8_t prefix = get_utf8_prefix(in[i]);
		if (prefix == 1 || prefix > 4) // 1 is reserved for trailer bytes
		{
			throw std::logic_error("not a UTF-8 string");
		}

		// if there are no prefix bits, this is ASCII
		if (prefix == 0)
		{
			unicode.push_back(in[i]);
			done = 1;
		}
		// otherwise this is a multibyte character
		else
		{
			// there must be enough characters
			if ((i + prefix) > in.length())
			{
				throw std::logic_error("not a UTF-8 string");
			}

			char32_t uni = get_utf8_data(prefix, in[i]);

			for (uint8_t j = 1; j < prefix; j++)
			{
				if (utf8_has_prefix(1, in[i + j]) == false)
				{
					throw std::logic_error("not a UTF-8 string");
				}

				uni <<= 6;
				uni |= get_utf8_data(1, in[i + j]);
			}

			if (uni >= kUtf16HighSurrogateStart && uni <= kUtf16LowSurrogateEnd)
			{
				throw std::logic_error("not a UTF-8 string");
			}
				
			if (uni > kUtf16EncodeMax)
			{
				throw std::logic_error("not a UTF-8 string");
			}
				
			unicode.push_back(uni);
			done = prefix;
		}

	}

	std::u16string utf16;
	for (size_t i = 0; i < unicode.size(); i++)
	{
		char32_t uni = unicode[i];
		if (uni < kUtf16NonNativeStart)
		{
			utf16.push_back((char16_t)uni);
		}
		else
		{
			uni -= kUtf16NonNativeStart;
			utf16.push_back(((uni >> kUtf16SurrogateBits) & kUtf16SurrogateMask) + kUtf16HighSurrogateStart);
			utf16.push_back((uni & kUtf16SurrogateMask) + kUtf16LowSurrogateStart);
		}
	}
	return utf16;
}

std::string StringConv::ConvertChar16ToChar8(const std::u16string & in)
{
	std::u32string unicode;
	size_t done = 0;
	for (size_t i = 0; i < in.length(); i+=done)
	{
		// this isn't a utf16 reserved character, so just add to unicode string
		if (in[i] < kUtf16HighSurrogateStart || in[i] > kUtf16LowSurrogateEnd)
		{
			unicode.push_back(in[i]);
			done = 1;
		}
		// otherwise we need to decode it
		else
		{
			// check that the high surrogate char exists first 
			if (in[i] < kUtf16HighSurrogateStart || in[i] > kUtf16HighSurrogateEnd)
			{
				throw std::logic_error("not a UTF-16 string");
			}
			// check that the low surrogate char exists next
			if (i >= in.length() - 1 || in[i + 1] < kUtf16LowSurrogateStart || in[i + 1] > kUtf16LowSurrogateEnd)
			{
				throw std::logic_error("not a UTF-16 string");
			}

			char32_t uni = ((in[i] & kUtf16SurrogateMask) << kUtf16SurrogateBits) | (in[i + 1] & kUtf16SurrogateMask) | 0x10000;

			unicode.push_back(uni);
			done = 2;
		}
	}

	std::string utf8;
	for (size_t i = 0; i < unicode.length(); i++)
	{
		if (unicode[i] <= kUtf8AsciiEnd)
		{
			utf8.push_back((char)unicode[i]);
		}
		else if (unicode[i] <= kUtf82ByteEnd)
		{
			utf8.push_back(make_utf8(2, (uint8_t)(unicode[i] >> 6)));
			utf8.push_back(make_utf8(1, (uint8_t)(unicode[i] >> 0)));
		}
		else if (unicode[i] <= kUtf83ByteEnd)
		{
			utf8.push_back(make_utf8(3, (uint8_t)(unicode[i] >> 12)));
			utf8.push_back(make_utf8(1, (uint8_t)(unicode[i] >> 6)));
			utf8.push_back(make_utf8(1, (uint8_t)(unicode[i] >> 0)));
		}
		else if (unicode[i] <= kUtf84ByteEnd)
		{
			utf8.push_back(make_utf8(4, (uint8_t)(unicode[i] >> 18)));
			utf8.push_back(make_utf8(1, (uint8_t)(unicode[i] >> 12)));
			utf8.push_back(make_utf8(1, (uint8_t)(unicode[i] >> 6)));
			utf8.push_back(make_utf8(1, (uint8_t)(unicode[i] >> 0)));
		}
		else
		{
			throw std::logic_error("not a UTF-16 string");
		}
	}

	return utf8;
}
