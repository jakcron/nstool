#pragma once
#include <fnd/types.h>

namespace fnd
{
	class SimpleTextOutput
	{
	public:
		static void hxdStyleDump(const byte_t* data, size_t len, size_t row_len, size_t byte_grouping_size);
		static void hxdStyleDump(const byte_t* data, size_t len);
		static void hexDump(const byte_t* data, size_t len, size_t row_len, size_t indent_len);
		static void hexDump(const byte_t* data, size_t len);
	private:
		static const size_t kDefaultRowLen = 0x10;
		static const size_t kDefaultByteGroupingSize = 1;
	};

}