#pragma once
#include "types.h"

namespace nstool
{

void processResFile(const std::shared_ptr<tc::io::IStream>& file, std::map<std::string, std::string>& dict);

void writeSubStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, int64_t offset, int64_t length, const tc::io::Path& out_path, tc::ByteData& cache);
void writeSubStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, int64_t offset, int64_t length, const tc::io::Path& out_path, size_t cache_size = 0x10000);
void writeStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, const tc::io::Path& out_path, tc::ByteData& cache);
void writeStreamToFile(const std::shared_ptr<tc::io::IStream>& in_stream, const tc::io::Path& out_path, size_t cache_size = 0x10000);
void writeStreamToStream(const std::shared_ptr<tc::io::IStream>& in_stream, const std::shared_ptr<tc::io::IStream>& out_stream, tc::ByteData& cache);
void writeStreamToStream(const std::shared_ptr<tc::io::IStream>& in_stream, const std::shared_ptr<tc::io::IStream>& out_stream, size_t cache_size = 0x10000);


std::string getTruncatedBytesString(const byte_t* data, size_t len);
std::string getTruncatedBytesString(const byte_t* data, size_t len, bool do_not_truncate);

}