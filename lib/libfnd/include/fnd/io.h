#pragma once
#include <string>
#include <fnd/MemoryBlob.h>

namespace fnd
{
	namespace io
	{
		size_t getFileSize(const std::string& path);
		void readFile(const std::string& path, MemoryBlob& blob);
		void readFile(const std::string& path, size_t offset, size_t len, MemoryBlob& blob);
		void writeFile(const std::string& path, const MemoryBlob& blob);
		void writeFile(const std::string& path, const byte_t* data, size_t len);
		void makeDirectory(const std::string& path);
		void getEnvironVar(std::string& var, const std::string& key);
		void makePath(std::string& out, const std::vector<std::string>& elements);
	}
}
