#include "nstool.h"
#include "HashTreeWrappedIFile.h"
#include "OffsetAdjustedIFile.h"

HashTreeWrappedIFile::HashTreeWrappedIFile(fnd::IFile* file, bool ownIFile, const HashTreeMeta& hdr) :
	mOwnIFile(ownIFile),
	mFile(file),
	mData(nullptr),
	mDataHashLayer(),
	mAlignHashCalcToBlock(false)
{
	initialiseDataLayer(hdr);
}

HashTreeWrappedIFile::~HashTreeWrappedIFile()
{
	if (mOwnIFile)
	{
		delete mFile;
	}	
	delete mData;
}

size_t HashTreeWrappedIFile::size()
{
	return mData->size();
}

void HashTreeWrappedIFile::seek(size_t offset)
{
	mDataOffset = offset;	
}

void HashTreeWrappedIFile::read(byte_t* out, size_t len)
{
	size_t offset_in_start_block = getOffsetInBlock(mDataOffset);
	size_t offset_in_end_block = getOffsetInBlock(offset_in_start_block + len);

	size_t start_block = getOffsetBlock(mDataOffset);
	size_t block_num = align(offset_in_start_block + len, mDataBlockSize) / mDataBlockSize;

	size_t partial_last_block_num = block_num % mCacheBlockNum;
	bool has_partial_block_num = partial_last_block_num > 0;
	size_t read_iterations = (block_num / mCacheBlockNum) + has_partial_block_num;

	size_t block_read_len;
	size_t block_export_offset;
	size_t block_export_size;
	size_t block_export_pos = 0;
	for (size_t i = 0; i < read_iterations; i++)
	{
		// how many blocks to read from source file
		block_read_len = (i+1 == read_iterations && has_partial_block_num) ? partial_last_block_num : mCacheBlockNum;

		// offset in this current read to copy from
		block_export_offset = (i == 0) ? offset_in_start_block : 0;

		// size of current read to copy
		block_export_size = (block_read_len * mDataBlockSize) - block_export_offset;

		// if last read, reduce the export size by one block less offset_in_end_block
		if (i+1 == read_iterations)
		{
			block_export_size -= (mDataBlockSize - offset_in_end_block);
		}

		// read the blocks
		readData(start_block + (i * mCacheBlockNum), block_read_len);

		// export the section of data that is relevant
		memcpy(out + block_export_pos, mCache.data() + block_export_offset, block_export_size);

		// update export position
		block_export_pos += block_export_size;
	}

	// update offset
	seek(mDataOffset + len);
}

void HashTreeWrappedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void HashTreeWrappedIFile::write(const byte_t* out, size_t len)
{
	throw fnd::Exception(kModuleName, "write() not supported");
}

void HashTreeWrappedIFile::write(const byte_t* out, size_t offset, size_t len)
{
	throw fnd::Exception(kModuleName, "write() not supported");
}

void HashTreeWrappedIFile::initialiseDataLayer(const HashTreeMeta& hdr)
{
	fnd::sha::sSha256Hash hash;
	fnd::Vec<byte_t> cur, prev;

	mAlignHashCalcToBlock = hdr.getAlignHashToBlock();

	// copy master hash into prev
	prev.alloc(sizeof(fnd::sha::sSha256Hash) * hdr.getMasterHashList().size());
	for (size_t i = 0; i < hdr.getMasterHashList().size(); i++)
	{
		((fnd::sha::sSha256Hash*)prev.data())[i] = hdr.getMasterHashList()[i];
	}
	
	// check each hash layer
	for (size_t i = 0; i < hdr.getHashLayerInfo().size(); i++)
	{
		// get block size
		const HashTreeMeta::sLayer& layer = hdr.getHashLayerInfo()[i];

		// allocate layer
		cur.alloc(align(layer.size, layer.block_size));

		// read layer
		mFile->read(cur.data(), layer.offset, layer.size);
		
		// validate blocks
		size_t validate_size;
		for (size_t j = 0; j < cur.size() / layer.block_size; j++)
		{
			validate_size = mAlignHashCalcToBlock? layer.block_size : _MIN(layer.size - (j * layer.block_size), layer.block_size);
			fnd::sha::Sha256(cur.data() + (j * layer.block_size), validate_size, hash.bytes);
			if (hash.compare(prev.data() + j * sizeof(fnd::sha::sSha256Hash)) == false)
			{
				mErrorSs << "Hash tree layer verification failed (layer: " << i << ", block: " << j << ")";
				throw fnd::Exception(kModuleName, mErrorSs.str());
			}
		}

		// set prev to cur
		prev = cur;
	}

	// save last layer as hash table for data layer
	fnd::sha::sSha256Hash* hash_list = (fnd::sha::sSha256Hash*)prev.data();
	for (size_t i = 0; i < prev.size() / sizeof(fnd::sha::sSha256Hash); i++)
	{
		mDataHashLayer.addElement(hash_list[i]);
	}

	// generate reader for data layer
	mData = new OffsetAdjustedIFile(mFile, SHARED_IFILE, hdr.getDataLayer().offset, hdr.getDataLayer().size);
	mDataOffset = 0;
	mDataBlockSize = hdr.getDataLayer().block_size;

	// allocate scratchpad
	//mScratch.alloc(mDataBlockSize * 0x10);
	size_t cache_size = align(kDefaultCacheSize, mDataBlockSize);
	mCacheBlockNum = cache_size / mDataBlockSize;
	//printf("Block Size: 0x%" PRIx64 "\n", mDataBlockSize);
	//printf("Cache size: 0x%" PRIx64 ", (block_num: %" PRId64 ")\n", cache_size, mCacheBlockNum);
	mCache.alloc(cache_size);
}

void HashTreeWrappedIFile::readData(size_t block_offset, size_t block_num)
{
	mData->seek(block_offset * mDataBlockSize);
	fnd::sha::sSha256Hash hash;

	// determine read size
	size_t read_len = 0;
	if ((block_offset + block_num) == getBlockNum(mData->size()))
	{
		read_len = (block_num-1) * mDataBlockSize + getRemanderBlockReadSize(mData->size());
		memset(mCache.data(), 0, block_num * mDataBlockSize);
	}
	else if ((block_offset + block_num) < getBlockNum(mData->size()))
	{
		read_len = block_num * mDataBlockSize;
	}
	else
	{
		throw fnd::Exception(kModuleName, "Out of bounds file read");
	}

	// read
	mData->read(mCache.data(), block_offset * mDataBlockSize, read_len);

	if (block_num > mCacheBlockNum)
	{
		throw fnd::Exception(kModuleName, "Read excessive of cache size");
	}

	//printf("readlen=0x%" PRIx64 "\n", read_len);

	// validate blocks
	size_t validate_size;
	for (size_t i = 0; i < block_num; i++)
	{
		validate_size = mAlignHashCalcToBlock? mDataBlockSize : _MIN(read_len - (i * mDataBlockSize), mDataBlockSize);
		fnd::sha::Sha256(mCache.data() + (i * mDataBlockSize), validate_size, hash.bytes);
		if (hash != mDataHashLayer[block_offset + i])
		{
			mErrorSs << "Hash tree layer verification failed (layer: data, block: " << (block_offset + i) << " ( " << i << "/" << block_num-1 << " ), offset: 0x" << std::hex << ((block_offset + i) * mDataBlockSize) << ", size: 0x" << std::hex <<  validate_size <<")";
			throw fnd::Exception(kModuleName, mErrorSs.str());
		}
	}
}