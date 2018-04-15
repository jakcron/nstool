#pragma once
#include <string>
#include <map>

namespace fnd
{
	class ResourceFileReader
	{
	public:
		ResourceFileReader();

		void processFile(const std::string& path);

		bool doesExist(const std::string& key) const;
		const std::string& operator[](const std::string& key);
		
	private:
		const std::string kModuleName = "ResourceFileReader";

		std::map<std::string,std::string> mResources;
	};
}
