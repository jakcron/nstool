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
	mScratch.alloc(kAesCtrScratchAllocSize);
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
	internalSeek();

	for (size_t i = 0; i < (len / kAesCtrScratchSize); i++)
	{
		mFile->read(mScratch.getBytes() + mBlockOffset, kAesCtrScratchSize);
		crypto::aes::AesCtr(mScratch.getBytes(), kAesCtrScratchAllocSize, mKey.key, mCurrentCtr.iv, mScratch.getBytes());
		memcpy(out + (i * kAesCtrScratchSize), mScratch.getBytes() + mBlockOffset, kAesCtrScratchSize);
	}

	if (len % kAesCtrScratchSize)
	{
		size_t read_len = len % kAesCtrScratchSize;
		size_t read_pos = ((len / kAesCtrScratchSize) * kAesCtrScratchSize); 
		mFile->read(mScratch.getBytes() + mBlockOffset, read_len);
		crypto::aes::AesCtr(mScratch.getBytes(), kAesCtrScratchAllocSize, mKey.key, mCurrentCtr.iv, mScratch.getBytes());
		memcpy(out + read_pos, mScratch.getBytes() + mBlockOffset, read_len);
	}

	seek(mFileOffset + len);
}

void AesCtrWrappedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void AesCtrWrappedIFile::write(const byte_t* out, size_t len)
{
	internalSeek();

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

	seek(mFileOffset + len);
}

void AesCtrWrappedIFile::write(const byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	write(out, len);
}

void AesCtrWrappedIFile::internalSeek()
{
	mFile->seek(mFileOffset);
	crypto::aes::AesIncrementCounter(mBaseCtr.iv, mFileOffset>>4, mCurrentCtr.iv);
	mBlockOffset = mFileOffset & 0xf;
}