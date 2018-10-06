#include <fnd/AesCtrWrappedIFile.h>

fnd::AesCtrWrappedIFile::AesCtrWrappedIFile(const fnd::SharedPtr<fnd::IFile>& file, const fnd::aes::sAes128Key& key, const fnd::aes::sAesIvCtr& ctr) :
	mFile(file),
	mKey(key),
	mBaseCtr(ctr),
	mFileOffset(0)
{
	mCache.alloc(kCacheSizeAllocSize);
}

size_t fnd::AesCtrWrappedIFile::size()
{
	return (*mFile)->size();
}

void fnd::AesCtrWrappedIFile::seek(size_t offset)
{
	mFileOffset = offset;
}

void fnd::AesCtrWrappedIFile::read(byte_t* out, size_t len)
{
	size_t read_len;
	size_t read_pos; 

	size_t cache_reads = (len / kCacheSize) + ((len % kCacheSize) != 0);

	for (size_t i = 0; i < cache_reads; i++)
	{
		read_len = _MIN(len - (i * kCacheSize), kCacheSize);
		read_pos = ((mFileOffset >> 4) << 4) + (i * kCacheSize);
		
		(*mFile)->seek(read_pos);
		(*mFile)->read(mCache.data(), kCacheSizeAllocSize);

		fnd::aes::AesIncrementCounter(mBaseCtr.iv, read_pos>>4, mCurrentCtr.iv);
		fnd::aes::AesCtr(mCache.data(), kCacheSizeAllocSize, mKey.key, mCurrentCtr.iv, mCache.data());

		memcpy(out + (i * kCacheSize), mCache.data() + (mFileOffset & 0xf), read_len);
	}

	seek(mFileOffset + len);
}

void fnd::AesCtrWrappedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void fnd::AesCtrWrappedIFile::write(const byte_t* in, size_t len)
{
	size_t write_len;
	size_t write_pos; 

	size_t cache_writes = (len / kCacheSize) + ((len % kCacheSize) != 0);

	for (size_t i = 0; i < cache_writes; i++)
	{
		write_len = _MIN(len - (i * kCacheSize), kCacheSize);
		write_pos = ((mFileOffset >> 4) << 4) + (i * kCacheSize);
		
		memcpy(mCache.data() + (mFileOffset & 0xf), in + (i * kCacheSize), write_len);

		fnd::aes::AesIncrementCounter(mBaseCtr.iv, write_pos>>4, mCurrentCtr.iv);
		fnd::aes::AesCtr(mCache.data(), kCacheSizeAllocSize, mKey.key, mCurrentCtr.iv, mCache.data());

		(*mFile)->seek(write_pos);
		(*mFile)->write(mCache.data(), kCacheSizeAllocSize);
	}

	seek(mFileOffset + len);
}

void fnd::AesCtrWrappedIFile::write(const byte_t* in, size_t offset, size_t len)
{
	seek(offset);
	write(in, len);
}