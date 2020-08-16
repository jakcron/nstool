#include "CompressedArchiveIFile.h"
#include <fnd/lz4.h>

#include <iostream>

CompressedArchiveIFile::CompressedArchiveIFile(const fnd::SharedPtr<fnd::IFile>& base_file, size_t compression_meta_offset) :
	mFile(base_file),
	mCompEntries(),
	mLogicalFileSize(0),
	mCacheCapacity(nn::hac::compression::kRomfsBlockSize),
	mCurrentCacheDataSize(0),
	mCache(std::shared_ptr<byte_t>(new byte_t[mCacheCapacity])),
	mScratch(std::shared_ptr<byte_t>(new byte_t[mCacheCapacity]))
{
	// determine and check the compression metadata size
	size_t compression_meta_size = (*mFile)->size() - compression_meta_offset;
	if (compression_meta_size % sizeof(nn::hac::sCompressionEntry))
	{
		fnd::Exception(kModuleName, "Invalid compression meta size");
	}
	
	// import raw metadata
	std::shared_ptr<byte_t> entries_raw = std::shared_ptr<byte_t>(new byte_t[compression_meta_size]);
	(*mFile)->read(entries_raw.get(), compression_meta_offset, compression_meta_size);

	// process metadata entries
	nn::hac::sCompressionEntry* entries = (nn::hac::sCompressionEntry*)entries_raw.get();
	for (size_t idx = 0, num = compression_meta_size / sizeof(nn::hac::sCompressionEntry); idx < num; idx++)
	{
		if (idx == 0)
		{
			if (entries[idx].physical_offset.get() != 0x0)
				throw fnd::Exception(kModuleName, "Entry 0 had a non-zero physical offset");
			if (entries[idx].virtual_offset.get() != 0x0)
				throw fnd::Exception(kModuleName, "Entry 0 had a non-zero virtual offset");
		}
		else 
		{
			if (entries[idx].physical_offset.get() != align(entries[idx - 1].physical_offset.get() + entries[idx - 1].physical_size.get(), nn::hac::compression::kRomfsBlockAlign))
				throw fnd::Exception(kModuleName, "Entry was not physically aligned with previous entry");
			if (entries[idx].virtual_offset.get() <= entries[idx - 1].virtual_offset.get())
				throw fnd::Exception(kModuleName, "Entry was not virtually aligned with previous entry");

			// set previous entry virtual_size = this->virtual_offset - prev->virtual_offset;
			mCompEntries[mCompEntries.size() - 1].virtual_size = uint32_t(entries[idx].virtual_offset.get() - mCompEntries[mCompEntries.size() - 1].virtual_offset);
		}

		if (entries[idx].physical_size.get() > nn::hac::compression::kRomfsBlockSize)
			throw fnd::Exception(kModuleName, "Entry physical size was too large");
		
		switch ((nn::hac::compression::CompressionType)entries[idx].compression_type)
		{
			case (nn::hac::compression::CompressionType::None):
			case (nn::hac::compression::CompressionType::Lz4):
				break;
			default:
				throw fnd::Exception(kModuleName, "Unsupported CompressionType");
		}

		mCompEntries.push_back({(nn::hac::compression::CompressionType)entries[idx].compression_type, entries[idx].virtual_offset.get(), 0, entries[idx].physical_offset.get(), entries[idx].physical_size.get()});
	}

	// determine logical file size and final entry size
	importEntryDataToCache(mCompEntries.size() - 1);
	mCompEntries[mCurrentEntryIndex].virtual_size = mCurrentCacheDataSize;
	mLogicalFileSize = mCompEntries[mCurrentEntryIndex].virtual_offset + mCompEntries[mCurrentEntryIndex].virtual_size;

	/*
	for (auto itr = mCompEntries.begin(); itr != mCompEntries.end(); itr++)
	{
		std::cout << "entry " << std::endl;
		std::cout << "  type:       " << (uint32_t)itr->compression_type << std::endl;
		std::cout << "  phys_addr:  0x" << std::hex << itr->physical_offset << std::endl;
		std::cout << "  phys_size:  0x" << std::hex << itr->physical_size << std::endl;
		std::cout << "  virt_addr:  0x" << std::hex << itr->virtual_offset << std::endl;
		std::cout << "  virt_size:  0x" << std::hex << itr->virtual_size << std::endl;
	}

	std::cout << "logical size: 0x" << std::hex << mLogicalFileSize << std::endl;
	*/
}

size_t CompressedArchiveIFile::size()
{
	return mLogicalFileSize;
}

void CompressedArchiveIFile::seek(size_t offset)
{
	mLogicalOffset = std::min<size_t>(offset, mLogicalFileSize);
}

void CompressedArchiveIFile::read(byte_t* out, size_t len)
{
	// limit len to the end of the logical file
	len = std::min<size_t>(len, mLogicalFileSize - mLogicalOffset);

	for (size_t pos = 0, entry_index = getEntryIndexForLogicalOffset(mLogicalOffset); pos < len; entry_index++)
	{
		// importing entry into cache (this does nothing if the entry is already imported)
		importEntryDataToCache(entry_index);

		// write padding if required
		if (mCompEntries[entry_index].virtual_size > mCurrentCacheDataSize)
		{
			memset(mCache.get() + mCurrentCacheDataSize, 0, mCompEntries[entry_index].virtual_size - mCurrentCacheDataSize);
		}
		
		// determine subset of cache to copy out
		size_t read_offset = mLogicalOffset - (size_t)mCompEntries[entry_index].virtual_offset;
		size_t read_size = std::min<size_t>(len, (size_t)mCompEntries[entry_index].virtual_size - read_offset);

		memcpy(out + pos, mCache.get() + read_offset, read_size);

		// update position/logical offset
		pos += read_size;
		mLogicalOffset += read_size;
	}
}

void CompressedArchiveIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void CompressedArchiveIFile::write(const byte_t* out, size_t len)
{
	throw fnd::Exception(kModuleName, "write() not supported");
}

void CompressedArchiveIFile::write(const byte_t* out, size_t offset, size_t len)
{
	throw fnd::Exception(kModuleName, "write() not supported");
}

void CompressedArchiveIFile::importEntryDataToCache(size_t entry_index)
{
	// return if entry already imported
	if (mCurrentEntryIndex == entry_index && mCurrentCacheDataSize != 0)
		return;

	// save index
	mCurrentEntryIndex = entry_index;
	
	// reference entry
	CompressionEntry& entry = mCompEntries[mCurrentEntryIndex]; 

	if (entry.compression_type == nn::hac::compression::CompressionType::None)
	{
		(*mFile)->read(mCache.get(), entry.physical_offset, entry.physical_size);
		mCurrentCacheDataSize = entry.physical_size;
	}
	else if (entry.compression_type == nn::hac::compression::CompressionType::Lz4)
	{
		(*mFile)->read(mScratch.get(), entry.physical_offset, entry.physical_size);

		mCurrentCacheDataSize = 0;
		fnd::lz4::decompressData(mScratch.get(), entry.physical_size, mCache.get(), uint32_t(mCacheCapacity), mCurrentCacheDataSize);

		if (mCurrentCacheDataSize == 0)
		{
			throw fnd::Exception(kModuleName, "Decompression of final block failed");
		}
	}
}

size_t CompressedArchiveIFile::getEntryIndexForLogicalOffset(size_t logical_offset)
{
	// rule out bad offset
	if (logical_offset > mLogicalFileSize)
		throw fnd::Exception(kModuleName, "illegal logical offset");

	size_t entry_index = 0;

	// try the current comp entry 
	if (mCompEntries[mCurrentEntryIndex].virtual_offset <= logical_offset && \
		mCompEntries[mCurrentEntryIndex].virtual_offset + mCompEntries[mCurrentEntryIndex].virtual_size >= logical_offset)
	{
		entry_index = mCurrentEntryIndex;
	}
	else
	{
		for (size_t index = 0; index < mCompEntries.size(); index++)
		{
			if (mCompEntries[index].virtual_offset <= logical_offset && \
				mCompEntries[index].virtual_offset + mCompEntries[index].virtual_size >= logical_offset)
			{
				entry_index = index;
			}
		}
	}

	return entry_index;
}