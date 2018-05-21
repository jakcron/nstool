#include "OffsetAdjustedIFile.h"

OffsetAdjustedIFile::OffsetAdjustedIFile(fnd::IFile* file, size_t offset, size_t size) :
	mOwnIFile(false),
	mFile(file),
	mBaseOffset(offset),
	mCurrentOffset(0),
	mSize(size)
{

}

OffsetAdjustedIFile::OffsetAdjustedIFile(fnd::IFile* file, bool ownIFile, size_t offset, size_t size) :
	mOwnIFile(ownIFile),
	mFile(file),
	mBaseOffset(offset),
	mCurrentOffset(0),
	mSize(size)
{

}

OffsetAdjustedIFile::~OffsetAdjustedIFile()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

size_t OffsetAdjustedIFile::size()
{
	return mSize;
}

void OffsetAdjustedIFile::seek(size_t offset)
{
	mCurrentOffset = MIN(offset, mSize);
	mFile->seek(offset + mBaseOffset);
}

void OffsetAdjustedIFile::read(byte_t* out, size_t len)
{
	seek(mCurrentOffset);
	mFile->read(out, len);
}

void OffsetAdjustedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void OffsetAdjustedIFile::write(const byte_t* out, size_t len)
{
	seek(mCurrentOffset);
	mFile->write(out, len);
}

void OffsetAdjustedIFile::write(const byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	write(out, len);
}