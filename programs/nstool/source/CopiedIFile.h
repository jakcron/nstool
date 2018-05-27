#pragma once
#include <fnd/IFile.h>

class CopiedIFile : public fnd::IFile
{
public:
	inline CopiedIFile(fnd::IFile* file) : mFile(file) {}

	inline size_t size() { return mFile->size(); }
	inline void seek(size_t offset) { mFile->seek(offset); }
	inline void read(byte_t* out, size_t len) { mFile->read(out, len); }
	inline void read(byte_t* out, size_t offset, size_t len) { mFile->read(out, offset, len); }
	inline void write(const byte_t* out, size_t len) { mFile->write(out, len); }
	inline void write(const byte_t* out, size_t offset, size_t len) { mFile->write(out, offset, len); }
private:
	fnd::IFile* mFile;
};