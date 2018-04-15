#include <fnd/types.h>
#include <fnd/ResourceFileReader.h>
#include <fstream>
#include <algorithm>
#include <iostream>

fnd::ResourceFileReader::ResourceFileReader()
{
}

inline bool isNotPrintable(char chr) { return isprint(chr) == false; }

void fnd::ResourceFileReader::processFile(const std::string& path)
{
	std::ifstream file;
	file.open(path, std::ios::in);
	if (file.is_open() == false)
	{
		throw fnd::Exception(kModuleName, "Failed to open file");
	}

	std::string line, key, value;
	while (std::getline(file, line))
	{
		// read up to comment line
		if (line.find(";") != std::string::npos)
			line = line.substr(0, line.find(";"));

		// change chars to lower string
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);

		// strip whitespace
		line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

		// strip nonprintable
		line.erase(std::remove_if(line.begin(), line.end(), isNotPrintable), line.end());

		// skip lines that don't have '='
		if (line.find("=") == std::string::npos)
			continue;

		key = line.substr(0,line.find("="));
		value = line.substr(line.find("=")+1);

		// skip if key or value is empty
		if (key.empty() || value.empty())
			continue;

		//std::cout << "[" + key + "]=(" + value + ")" << std::endl;

		mResources[key] = value;
	}
	file.close();
}

bool fnd::ResourceFileReader::doesExist(const std::string& key) const
{
	return (mResources.find(key) != mResources.end());
}

const std::string& fnd::ResourceFileReader::operator[](const std::string& key)
{
	return mResources[key];
}