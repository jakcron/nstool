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
		MemoryBlob();
		MemoryBlob(const byte_t* bytes, size_t len);

		bool operator==(const MemoryBlob& other) const;
		bool operator!=(const MemoryBlob& other) const;
		void operator=(const MemoryBlob& other);

		void alloc(size_t size);
		void extend(size_t new_size);
		void clear();

		inline byte_t& operator[](size_t index) { return mData[index]; }
		inline const byte_t& operator[](size_t index) const { return mData[index]; }

		inline byte_t* getBytes() { return mData.data(); }
		inline const byte_t* getBytes() const { return mData.data(); }
		inline size_t getSize() const { return mVisableSize; }
	private:
		const std::string kModuleName = "MEMORY_BLOB";
		static const size_t kAllocBlockSize = 0x1000;

		std::vector<byte_t> mData;
		size_t mSize;
		size_t mVisableSize;

		void allocateMemory(size_t size);
		void clearMemory();
	};
}
