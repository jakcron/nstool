#pragma once
#include <sstream>
#include <fnd/IFile.h>
#include <fnd/MemoryBlob.h>
#include <crypto/sha.h>
#include "HashTreeMeta.h"


class HashTreeWrappedIFile : public fnd::IFile
{
public:
	HashTreeWrappedIFile(fnd::IFile* file, const HashTreeMeta& hdr);
	HashTreeWrappedIFile(fnd::IFile* file, bool ownIFile, const HashTreeMeta& hdr);
	~HashTreeWrappedIFile();

	size_t size();
	void seek(size_t offset);
	void read(byte_t* out, size_t len);
	void read(byte_t* out, size_t offset, size_t len);
	void write(const byte_t* out, size_t len);
	void write(const byte_t* out, size_t offset, size_t len);
private:
	const std::string kModuleName = "HashTreeWrappedIFile";
	std::stringstream mErrorSs;

	bool mOwnIFile;
	fnd::IFile* mFile;
	


	// data file
	fnd::IFile* mData;
	size_t mDataOffset;
	size_t mDataBlockSize;
	fnd::List<crypto::sha::sSha256Hash> mDataHashLayer;
	bool mAlignHashCalcToBlock;

	fnd::MemoryBlob mScratch;

	inline size_t getOffsetBlock(size_t offset) const { return offset / mDataBlockSize; }
	inline size_t getOffsetInBlock(size_t offset) const { return offset % mDataBlockSize; }
	inline size_t getRemanderBlockReadSize(size_t total_size) const { return total_size % mDataBlockSize; }
	inline size_t getBlockNum(size_t total_size) const { return (total_size / mDataBlockSize) + (getRemanderBlockReadSize(total_size) > 0); }

	void initialiseDataLayer(const HashTreeMeta& hdr);
	void readData(size_t block_offset, size_t block_num);
};