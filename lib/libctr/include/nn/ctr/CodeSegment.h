/*
CodeSegment.h
(c) 2018 Jakcron
*/
#pragma once
#include <fnd/types.h>

namespace nn
{
	namespace ctr
	{
		class CodeSegment
		{
		public:
			CodeSegment();
			CodeSegment(const CodeSegment& other);
			CodeSegment(uint32_t address, uint32_t size, uint32_t page_num);

			void operator=(const CodeSegment& other);
			bool operator==(const CodeSegment& other) const;
			bool operator!=(const CodeSegment& other) const;

			void setAddress(uint32_t address);
			uint32_t getAddress() const;

			void setSize(uint32_t size);
			uint32_t getSize() const;

			void setPageNum(uint32_t num);
			uint32_t getPageNum() const;
		private:
			uint32_t mAddress;
			uint32_t mSize;
			uint32_t mPageNum;
		};
	}
}