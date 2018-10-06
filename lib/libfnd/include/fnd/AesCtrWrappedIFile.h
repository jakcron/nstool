#pragma once
#include <fnd/IFile.h>
#include <fnd/SharedPtr.h>
#include <fnd/Vec.h>
#include <fnd/aes.h>

namespace fnd
{
	class AesCtrWrappedIFile : public fnd::IFile
	{
	public:
		AesCtrWrappedIFile(const fnd::SharedPtr<fnd::IFile>& file, const fnd::aes::sAes128Key& key, const fnd::aes::sAesIvCtr& ctr);

		size_t size();
		void seek(size_t offset);
		void read(byte_t* out, size_t len);
		void read(byte_t* out, size_t offset, size_t len);
		void write(const byte_t* out, size_t len);
		void write(const byte_t* out, size_t offset, size_t len);
	private:
		const std::string kModuleName = "AesCtrWrappedIFile";
		static const size_t kCacheSize = 0x10000;
		static const size_t kCacheSizeAllocSize = kCacheSize + fnd::aes::kAesBlockSize;

		fnd::SharedPtr<fnd::IFile> mFile;
		fnd::aes::sAes128Key mKey;
		fnd::aes::sAesIvCtr mBaseCtr, mCurrentCtr;
		size_t mFileOffset;

		fnd::Vec<byte_t> mCache;
	};
}