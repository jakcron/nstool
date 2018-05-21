

#include "HashTreeWrappedIFile.h"
#include "CopiedIFile.h"
#include "OffsetAdjustedIFile.h"


HashTreeWrappedIFile::HashTreeWrappedIFile(fnd::IFile* file, const HashTreeMeta& hdr) :
	mOwnIFile(true),
	mFile(file),
	mData(nullptr),
	mDataHashLayer(),
	mAlignHashCalcToBlock(false)
{
	initialiseDataLayer(hdr);
}

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

	size_t scratch_block_capacity = mScratch.getSize() / mDataBlockSize;

	size_t partial_last_block_num = block_num % scratch_block_capacity;
	bool has_partial_block_num = partial_last_block_num > 0;
	size_t read_iterations = (block_num / scratch_block_capacity) + has_partial_block_num;


	size_t block_read_len;
	size_t block_export_offset;
	size_t block_export_size;
	size_t block_export_pos = 0;
	for (size_t i = 0; i < read_iterations; i++)
	{
		// how many blocks to read from source file
		block_read_len = (i+1 == read_iterations && has_partial_block_num) ? partial_last_block_num : scratch_block_capacity;

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
		readData(start_block + (i * scratch_block_capacity), block_read_len);

		// export the section of data that is relevant
		memcpy(out + block_export_pos, mScratch.getBytes() + block_export_offset, block_export_size);

		// update export position
		block_export_pos += block_export_size;
	}

	// update offset
	mDataOffset += len;
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
	crypto::sha::sSha256Hash hash;
	fnd::MemoryBlob cur, prev;

	mAlignHashCalcToBlock = hdr.getAlignHashToBlock();

	// copy master hash into prev
	prev.alloc(sizeof(crypto::sha::sSha256Hash) * hdr.getMasterHashList().getSize());
	for (size_t i = 0; i < hdr.getMasterHashList().getSize(); i++)
	{
		((crypto::sha::sSha256Hash*)prev.getBytes())[i] = hdr.getMasterHashList()[i];
	}
	
	// check each hash layer
	for (size_t i = 0; i < hdr.getHashLayerInfo().getSize(); i++)
	{
		// get block size
		const HashTreeMeta::sLayer& layer = hdr.getHashLayerInfo()[i];

		// allocate layer
		cur.alloc(align(layer.size, layer.block_size));

		// read layer
		mFile->read(cur.getBytes(), layer.offset, layer.size);
		
		// validate blocks
		size_t validate_size;
		for (size_t j = 0; j < cur.getSize() / layer.block_size; j++)
		{
			validate_size = mAlignHashCalcToBlock? layer.block_size : MIN(layer.size - (j * layer.block_size), layer.block_size);
			crypto::sha::Sha256(cur.getBytes() + (j * layer.block_size), validate_size, hash.bytes);
			if (hash.compare(prev.getBytes() + j * sizeof(crypto::sha::sSha256Hash)) == false)
			{
				mErrorSs << "Hash tree layer verification failed (layer: " << i << ", block: " << j << ")";
				throw fnd::Exception(kModuleName, mErrorSs.str());
			}
		}

		// set prev to cur
		prev = cur;
	}

	// save last layer as hash table for data layer
	crypto::sha::sSha256Hash* hash_list = (crypto::sha::sSha256Hash*)prev.getBytes();
	for (size_t i = 0; i < prev.getSize() / sizeof(crypto::sha::sSha256Hash); i++)
	{
		mDataHashLayer.addElement(hash_list[i]);
	}

	// generate reader for data layer
	mData = new OffsetAdjustedIFile(mFile, false, hdr.getDataLayer().offset, hdr.getDataLayer().size);
	mDataOffset = 0;
	mDataBlockSize = hdr.getDataLayer().block_size;

	// allocate scratchpad
	mScratch.alloc(mDataBlockSize * 0x10);
}

void HashTreeWrappedIFile::readData(size_t block_offset, size_t block_num)
{
	seek(block_offset * mDataBlockSize);
	crypto::sha::sSha256Hash hash;

	// determine read size
	size_t read_len = 0;
	if ((block_offset + block_num) == getBlockNum(mData->size()))
	{
		read_len = (block_num-1) * mDataBlockSize + getRemanderBlockReadSize(mData->size());
		memset(mScratch.getBytes(), 0, block_num * mDataBlockSize);
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
	mData->read(mScratch.getBytes(), block_offset * mDataBlockSize, read_len);



	// validate blocks
	size_t validate_size;
	for (size_t i = 0; i < block_num; i++)
	{
		validate_size = mAlignHashCalcToBlock? mDataBlockSize : MIN(read_len - (i * mDataBlockSize), mDataBlockSize);
		crypto::sha::Sha256(mScratch.getBytes() + (i * mDataBlockSize), validate_size, hash.bytes);
		if (hash != mDataHashLayer[block_offset + i])
		{
			mErrorSs << "Hash tree layer verification failed (layer: data, block: " << (block_offset + i) << ", offset: 0x" << std::hex << ((block_offset + i) * mDataBlockSize) << ", size: 0x" << std::hex <<  validate_size <<")";
			throw fnd::Exception(kModuleName, mErrorSs.str());
		}
	}
}