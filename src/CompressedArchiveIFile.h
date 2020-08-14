#pragma once
#include <sstream>
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <memory>
#include <vector>
#include <nn/hac/define/compression.h>

class CompressedArchiveIFile : public fnd::IFile
{
public:
	CompressedArchiveIFile(const fnd::SharedPtr<fnd::IFile>& file, size_t compression_meta_offset);

	size_t size();
	void seek(size_t offset);
	void read(byte_t* out, size_t len);
	void read(byte_t* out, size_t offset, size_t len);
	void write(const byte_t* out, size_t len);
	void write(const byte_t* out, size_t offset, size_t len);
private:
	const std::string kModuleName = "CompressedArchiveIFile";
	std::stringstream mErrorSs;

	struct CompressionEntry
	{
		nn::hac::compression::CompressionType compression_type;
		uint64_t virtual_offset;
		uint32_t virtual_size;
		uint64_t physical_offset;
		uint32_t physical_size;		
	};

	// raw data
	fnd::SharedPtr<fnd::IFile> mFile;

	// compression metadata
	std::vector<CompressionEntry> mCompEntries;
	size_t mLogicalFileSize;
	size_t mLogicalOffset;

	// cached decompressed entry
	size_t mCacheCapacity; // capacity 
	size_t mCurrentEntryIndex; // index of entry currently associated with the cache
	uint32_t mCurrentCacheDataSize; // size of data currently in cache
	std::shared_ptr<byte_t> mCache; // where decompressed data resides
	std::shared_ptr<byte_t> mScratch; // same size as cache, but is used for storing data pre-compression

	// this will import entry to cache
	void importEntryDataToCache(size_t entry_index);
	size_t getEntryIndexForLogicalOffset(size_t logical_offset);
};