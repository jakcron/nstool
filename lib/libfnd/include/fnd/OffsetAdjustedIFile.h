#pragma once
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>

namespace fnd
{
	class OffsetAdjustedIFile : public fnd::IFile
	{
	public:
		OffsetAdjustedIFile(const fnd::SharedPtr<fnd::IFile>& file, size_t offset, size_t size);

		size_t size();
		void seek(size_t offset);
		void read(byte_t* out, size_t len);
		void read(byte_t* out, size_t offset, size_t len);
		void write(const byte_t* out, size_t len);
		void write(const byte_t* out, size_t offset, size_t len);
	private:
		fnd::SharedPtr<fnd::IFile> mFile;
		size_t mBaseOffset, mCurrentOffset;
		size_t mSize;
	};
}