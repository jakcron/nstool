#pragma once
#include <string>

namespace fnd
{
	namespace io
	{
#ifdef _WIN32
		const std::string kPathDivider = "\\";
#else
		const std::string kPathDivider = "/";
#endif

		size_t getFileSize(const std::string& path);
		void makeDirectory(const std::string& path);
		void getEnvironVar(std::string& var, const std::string& key);
		void appendToPath(std::string& base, const std::string& add);
	}
}
