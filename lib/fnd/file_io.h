#pragma once
#include <string>
#include <fnd/memory_blob.h>

namespace fnd
{
	class FileIO
	{
	public:
		static void ReadFile(const std::string& path, MemoryBlob& blob);
		//static void ReadFile(const char* path, MemoryBlob& blob, size_t offset, size_t size);
		static void WriteFile(const std::string& path, const MemoryBlob& blob);
		static void WriteFile(const std::string& path, const u8* data, size_t len);
		//static void WriteFile(const char* path, const MemoryBlob& blob, size_t offset, size_t size);
	private:

	};
}


