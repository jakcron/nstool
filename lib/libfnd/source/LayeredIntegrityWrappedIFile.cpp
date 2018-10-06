#include <fnd/LayeredIntegrityWrappedIFile.h>
#include <fnd/OffsetAdjustedIFile.h>

fnd::LayeredIntegrityWrappedIFile::LayeredIntegrityWrappedIFile(const fnd::SharedPtr<fnd::IFile>& file, const fnd::LayeredIntegrityMetadata& hdr) :
	mFile(file),
	mData(nullptr),
	mDataHashLayer(),
	mAlignHashCalcToBlock(false)
{
	initialiseDataLayer(hdr);
}

size_t fnd::LayeredIntegrityWrappedIFile::size()
{
	return (*mData)->size();
}

void fnd::LayeredIntegrityWrappedIFile::seek(size_t offset)
{
	mDataOffset = offset;	
}

void fnd::LayeredIntegrityWrappedIFile::read(byte_t* out, size_t len)
{
	struct sBlockPosition 
	{
		size_t index;
		size_t pos;
	} start_blk, end_blk;

	start_blk.index = getOffsetBlock(mDataOffset);
	start_blk.pos = getOffsetInBlock(mDataOffset);

	end_blk.index = getOffsetBlock(mDataOffset + len);
	end_blk.pos = getOffsetInBlock(mDataOffset + len);
	if (end_blk.pos == 0 && len != 0)
	{
		end_blk.index -= 1;
		end_blk.pos = mDataBlockSize;
	}

	size_t total_blk_num = (end_blk.index - start_blk.index) + 1;

	size_t read_blk_num = 0;
	size_t cache_export_start_pos, cache_export_end_pos, cache_export_size;
	size_t export_pos = 0;
	for (size_t i = 0; i < total_blk_num; i += read_blk_num, export_pos += cache_export_size)
	{
		read_blk_num = _MIN(mCacheBlockNum, (total_blk_num - i));
		readData(start_blk.index + i, read_blk_num);

		// if this is the first read, adjust offset
		if (i == 0)
			cache_export_start_pos = start_blk.pos;
		else
			cache_export_start_pos = 0;
		
		// if this is the last block, adjust end offset
		if ((i + read_blk_num) == total_blk_num)
			cache_export_end_pos = ((read_blk_num - 1) * mDataBlockSize) + end_blk.pos;
		else
			cache_export_end_pos = read_blk_num * mDataBlockSize;

		// determine cache export size
		cache_export_size = cache_export_end_pos - cache_export_start_pos;

		memcpy(out + export_pos, mCache.data() + cache_export_start_pos, cache_export_size);
	}

	// update offset
	seek(mDataOffset + len);
}

void fnd::LayeredIntegrityWrappedIFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void fnd::LayeredIntegrityWrappedIFile::write(const byte_t* out, size_t len)
{
	throw fnd::Exception(kModuleName, "write() not supported");
}

void fnd::LayeredIntegrityWrappedIFile::write(const byte_t* out, size_t offset, size_t len)
{
	throw fnd::Exception(kModuleName, "write() not supported");
}

void fnd::LayeredIntegrityWrappedIFile::initialiseDataLayer(const fnd::LayeredIntegrityMetadata& hdr)
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
		const fnd::LayeredIntegrityMetadata::sLayer& layer = hdr.getHashLayerInfo()[i];

		// allocate layer
		cur.alloc(align(layer.size, layer.block_size));

		// read layer
		(*mFile)->read(cur.data(), layer.offset, layer.size);
		
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
	mData = new fnd::OffsetAdjustedIFile(mFile, hdr.getDataLayer().offset, hdr.getDataLayer().size);
	mDataOffset = 0;
	mDataBlockSize = hdr.getDataLayer().block_size;

	// allocate cache
	size_t cache_size = align(kDefaultCacheSize, mDataBlockSize);
	mCacheBlockNum = cache_size / mDataBlockSize;
	mCache.alloc(cache_size);
}

void fnd::LayeredIntegrityWrappedIFile::readData(size_t block_offset, size_t block_num)
{
	fnd::sha::sSha256Hash hash;

	// determine read size
	size_t read_len = 0;
	if ((block_offset + block_num) == getBlockNum((*mData)->size()))
	{
		read_len = (block_num-1) * mDataBlockSize + getRemanderBlockReadSize((*mData)->size());
		memset(mCache.data(), 0, block_num * mDataBlockSize);
	}
	else if ((block_offset + block_num) < getBlockNum((*mData)->size()))
	{
		read_len = block_num * mDataBlockSize;
	}
	else
	{
		throw fnd::Exception(kModuleName, "Out of bounds file read");
	}

	// read
	(*mData)->read(mCache.data(), block_offset * mDataBlockSize, read_len);

	if (block_num > mCacheBlockNum)
	{
		throw fnd::Exception(kModuleName, "Read excessive of cache size");
	}

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