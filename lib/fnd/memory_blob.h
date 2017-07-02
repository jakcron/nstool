#pragma once
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <fnd/types.h>

namespace fnd
{
	class MemoryBlob
	{
	public:
		enum ErrorCode
		{
			ERR_NONE,
			ERR_FAILOPEN,
			ERR_FAILMALLOC,
			ERR_FAILREAD,
		};

		MemoryBlob();

		~MemoryBlob();

		int alloc(size_t size);
		int extend(size_t new_size);;

		inline byte_t* data() { return data_.data(); }
		inline const byte_t* data() const { return data_.data(); }
		inline size_t size() const { return apparent_size_; }
	private:
		std::vector<byte_t> data_;
		size_t size_;
		size_t apparent_size_;

		int AllocateMemory(size_t size);

		void ClearMemory();
	};
}
