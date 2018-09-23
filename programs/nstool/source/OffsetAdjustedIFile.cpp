#include "OffsetAdjustedIFile.h"

OffsetAdjustedIFile::OffsetAdjustedIFile(const fnd::SharedPtr<fnd::IFile>& file, size_t offset, size_t size) :
	mFile(file),
	mBaseOffset(offset),
	mCurrentOffset(0),
	mSize(size)
{
}

size_t OffsetAdjustedIFile::size()
{
	return mSize;
}

void OffsetAdjustedIFile::seek(size_t offset)
{
	mCurrentOffset = _MIN(offset, mSize);
}

void OffsetAdjustedIFile::read(byte_t* out, size_t len)
{
	// assert proper position in file
	(*mFile)->seek(mCurrentOffset + mBaseOffset);
	(*mFile)->read(out, len);
	seek(mCurrentOffset + len);
}

void OffsetAdjustedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void OffsetAdjustedIFile::write(const byte_t* out, size_t len)
{
	// assert proper position in file
	(*mFile)->seek(mCurrentOffset + mBaseOffset);
	(*mFile)->write(out, len);
	seek(mCurrentOffset + len);
}

void OffsetAdjustedIFile::write(const byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	write(out, len);
}