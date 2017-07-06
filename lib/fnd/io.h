#pragma once
#include <string>
#include <fnd/memory_blob.h>

namespace fnd
{
	namespace io
	{
		void readFile(const std::string& path, MemoryBlob& blob);
		void writeFile(const std::string& path, const MemoryBlob& blob);
		void writeFile(const std::string& path, const u8* data, size_t len);
	}
}
