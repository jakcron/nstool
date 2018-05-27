#include "AesCtrWrappedIFile.h"

AesCtrWrappedIFile::AesCtrWrappedIFile(fnd::IFile* file, const crypto::aes::sAes128Key& key, const crypto::aes::sAesIvCtr& ctr) :
	AesCtrWrappedIFile(file, false, key, ctr)
{
}

AesCtrWrappedIFile::AesCtrWrappedIFile(fnd::IFile* file, bool ownIfile, const crypto::aes::sAes128Key& key, const crypto::aes::sAesIvCtr& ctr) :
	mOwnIFile(ownIfile),
	mFile(file),
	mKey(key),
	mBaseCtr(ctr),
	mFileOffset(0)
{
	mCache.alloc(kCacheSizeAllocSize);
}

AesCtrWrappedIFile::~AesCtrWrappedIFile()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

size_t AesCtrWrappedIFile::size()
{
	return mFile->size();
}

void AesCtrWrappedIFile::seek(size_t offset)
{
	mFileOffset = offset;
}

void AesCtrWrappedIFile::read(byte_t* out, size_t len)
{
	//printf("[%x] AesCtrWrappedIFile::read(offset=0x%" PRIx64 ", size=0x%" PRIx64 ")\n", this, mFileOffset, len);

	size_t read_len;
	size_t read_pos; 

	size_t cache_reads = (len / kCacheSize) + ((len % kCacheSize) != 0);

	for (size_t i = 0; i < cache_reads; i++)
	{
		read_len = MIN(len - (i * kCacheSize), kCacheSize);
		read_pos = ((mFileOffset >> 4) << 4) + (i * kCacheSize);

		//printf("[%x] AesCtrWrappedIFile::read() CACHE READ: readlen=%" PRIx64 "\n", this, read_len);
		
		mFile->seek(read_pos);
		mFile->read(mCache.getBytes(), kCacheSizeAllocSize);

		crypto::aes::AesIncrementCounter(mBaseCtr.iv, read_pos>>4, mCurrentCtr.iv);
		crypto::aes::AesCtr(mCache.getBytes(), kCacheSizeAllocSize, mKey.key, mCurrentCtr.iv, mCache.getBytes());

		memcpy(out + (i * kCacheSize), mCache.getBytes() + (mFileOffset & 0xf), read_len);
	}

	seek(mFileOffset + len);
}

void AesCtrWrappedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void AesCtrWrappedIFile::write(const byte_t* in, size_t len)
{
	size_t write_len;
	size_t write_pos; 

	size_t cache_writes = (len / kCacheSize) + ((len % kCacheSize) != 0);

	for (size_t i = 0; i < cache_writes; i++)
	{
		write_len = MIN(len - (i * kCacheSize), kCacheSize);
		write_pos = ((mFileOffset >> 4) << 4) + (i * kCacheSize);

		//printf("[%x] AesCtrWrappedIFile::read() CACHE READ: readlen=%" PRIx64 "\n", this, read_len);
		
		memcpy(mCache.getBytes() + (mFileOffset & 0xf), in + (i * kCacheSize), write_len);

		crypto::aes::AesIncrementCounter(mBaseCtr.iv, write_pos>>4, mCurrentCtr.iv);
		crypto::aes::AesCtr(mCache.getBytes(), kCacheSizeAllocSize, mKey.key, mCurrentCtr.iv, mCache.getBytes());

		mFile->seek(write_pos);
		mFile->write(mCache.getBytes(), kCacheSizeAllocSize);
	}

	seek(mFileOffset + len);

	/*
	for (size_t i = 0; i < (len / kAesCtrScratchSize); i++)
	{
		memcpy(mScratch.getBytes() + mBlockOffset, out + (i * kAesCtrScratchSize), kAesCtrScratchSize);
		crypto::aes::AesCtr(mScratch.getBytes(), kAesCtrScratchAllocSize, mKey.key, mCurrentCtr.iv, mScratch.getBytes());
		mFile->write(mScratch.getBytes() + mBlockOffset, kAesCtrScratchSize);
	}

	if (len % kAesCtrScratchSize)
	{
		size_t write_len = len % kAesCtrScratchSize;
		size_t write_pos = ((len / kAesCtrScratchSize) * kAesCtrScratchSize); 
		memcpy(mScratch.getBytes() + mBlockOffset, out + write_pos, write_len);
		crypto::aes::AesCtr(mScratch.getBytes(), kAesCtrScratchAllocSize, mKey.key, mCurrentCtr.iv, mScratch.getBytes());
		mFile->write(mScratch.getBytes() + mBlockOffset, write_len);
	}
	*/
	seek(mFileOffset + len);
}

void AesCtrWrappedIFile::write(const byte_t* in, size_t offset, size_t len)
{
	seek(offset);
	write(in, len);
}