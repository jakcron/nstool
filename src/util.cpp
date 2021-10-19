#include "util.h"

#include <tc/io/FileStream.h>
#include <tc/io/SubStream.h>
#include <tc/io/IOUtil.h>

#include <sstream>
#include <algorithm>
#include <iostream>

inline bool isNotPrintable(char chr) { return isprint(chr) == false; }

void nstool::processResFile(const std::shared_ptr<tc::io::IStream>& file, std::map<std::string, std::string>& dict)
{
	if (file == nullptr || !file->canRead() || file->length() == 0)
	{
		return;
	}

	std::stringstream in_stream;

	// populate string stream
	tc::ByteData cache = tc::ByteData(0x1000);
	file->seek(0, tc::io::SeekOrigin::Begin);
	for (int64_t pos = 0; pos < file->length();)
	{
		size_t bytes_read = file->read(cache.data(), cache.size());

		in_stream << std::string((char*)cache.data(), bytes_read);

		pos += tc::io::IOUtil::castSizeToInt64(bytes_read);
	}

	// process stream
	std::string line, key, value;
	while (std::getline(in_stream, line))
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

		dict[key] = value;
	}

}

void nstool::writeSubStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, int64_t offset, int64_t length, const tc::io::Path& out_path, tc::ByteData& cache)
{
	writeStreamToStream(std::make_shared<tc::io::SubStream>(tc::io::SubStream(in_stream, offset, length)), std::make_shared<tc::io::FileStream>(tc::io::FileStream(out_path, tc::io::FileMode::Create, tc::io::FileAccess::Write)), cache);
}

void nstool::writeSubStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, int64_t offset, int64_t length, const tc::io::Path& out_path, size_t cache_size)
{
	writeStreamToStream(std::make_shared<tc::io::SubStream>(tc::io::SubStream(in_stream, offset, length)), std::make_shared<tc::io::FileStream>(tc::io::FileStream(out_path, tc::io::FileMode::Create, tc::io::FileAccess::Write)), cache_size);
}

void nstool::writeStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, const tc::io::Path& out_path, tc::ByteData& cache)
{
	writeStreamToStream(in_stream, std::make_shared<tc::io::FileStream>(tc::io::FileStream(out_path, tc::io::FileMode::Create, tc::io::FileAccess::Write)), cache);
}

void nstool::writeStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, const tc::io::Path& out_path, size_t cache_size)
{
	writeStreamToStream(in_stream, std::make_shared<tc::io::FileStream>(tc::io::FileStream(out_path, tc::io::FileMode::Create, tc::io::FileAccess::Write)), cache_size);
}

void nstool::writeStreamToStream(const std::shared_ptr<tc::io::IStream>& in_stream, const std::shared_ptr<tc::io::IStream>& out_stream, tc::ByteData& cache)
{
	// iterate thru child files
	size_t cache_read_len;
	
	in_stream->seek(0, tc::io::SeekOrigin::Begin);
	out_stream->seek(0, tc::io::SeekOrigin::Begin);
	for (int64_t remaining_data = in_stream->length(); remaining_data > 0;)
	{
		cache_read_len = in_stream->read(cache.data(), cache.size());
		if (cache_read_len == 0)
		{
			throw tc::io::IOException("nstool::writeStreamToStream()", "Failed to read from source streeam.");
		}

		out_stream->write(cache.data(), cache_read_len);

		remaining_data -= int64_t(cache_read_len);
	}
}

void nstool::writeStreamToStream(const std::shared_ptr<tc::io::IStream>& in_stream, const std::shared_ptr<tc::io::IStream>& out_stream, size_t cache_size)
{
	tc::ByteData cache = tc::ByteData(cache_size);
	writeStreamToStream(in_stream, out_stream, cache);
}

std::string nstool::getTruncatedBytesString(const byte_t* data, size_t len)
{
	if (data == nullptr) { return fmt::format(""); }

	std::string str = "";

	if (len <= 8)
	{
		str = tc::cli::FormatUtil::formatBytesAsString(data, len, true, "");
	}
	else
	{
		str = fmt::format("{:02X}{:02X}{:02X}{:02X}...{:02X}{:02X}{:02X}{:02X}", data[0], data[1], data[2], data[3], data[len-4], data[len-3], data[len-2], data[len-1]);
	}

	return str;
}

std::string nstool::getTruncatedBytesString(const byte_t* data, size_t len, bool do_not_truncate)
{
	if (data == nullptr) { return fmt::format(""); }

	std::string str = "";

	if (len <= 8 || do_not_truncate)
	{
		str = tc::cli::FormatUtil::formatBytesAsString(data, len, true, "");
	}
	else
	{
		str = fmt::format("{:02X}{:02X}{:02X}{:02X}...{:02X}{:02X}{:02X}{:02X}", data[0], data[1], data[2], data[3], data[len-4], data[len-3], data[len-2], data[len-1]);
	}

	return str;
}