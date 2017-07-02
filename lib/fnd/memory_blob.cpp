#include "memory_blob.h"

using namespace fnd;

MemoryBlob::MemoryBlob() :
	data_(),
	size_(0),
	apparent_size_(0)
{

}

MemoryBlob::~MemoryBlob()
{
}

int MemoryBlob::alloc(size_t size)
{
	int ret = ERR_NONE;
	if (size > size_)
	{
		ret = AllocateMemory(size);
	}
	else
	{
		apparent_size_ = size;
		ClearMemory();
	}
	return ret;
}

int MemoryBlob::extend(size_t new_size)
{
	try {
		data_.resize(new_size);
	}
	catch (...) {
		return ERR_FAILMALLOC;
	}

	return ERR_NONE;
	
	return 0;
}

int MemoryBlob::AllocateMemory(size_t size)
{
	size_ = (size_t)align(size, 0x1000);
	apparent_size_ = size;
	data_.resize(size_);
	ClearMemory();
	return ERR_NONE;
}

void MemoryBlob::ClearMemory()
{
	memset(data_.data(), 0, size_);
}
