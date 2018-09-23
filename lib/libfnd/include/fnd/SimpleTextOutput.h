#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/Vec.h>

namespace fnd
{
	class SimpleTextOutput
	{
	public:
		static void hxdStyleDump(const byte_t* data, size_t len, size_t row_len, size_t byte_grouping_size);
		static void hxdStyleDump(const byte_t* data, size_t len);
		static void hexDump(const byte_t* data, size_t len, size_t row_len, size_t indent_len);
		static void hexDump(const byte_t* data, size_t len);
		static std::string arrayToString(const byte_t* data, size_t len, bool upper_case, const std::string& separator);
		static void stringToArray(const std::string& str, fnd::Vec<byte_t>& array);

	private:
		static const size_t kDefaultRowLen = 0x10;
		static const size_t kDefaultByteGroupingSize = 1;
	};

}