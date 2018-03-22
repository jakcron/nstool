#include <nx/PfsHeader.h>



nx::PfsHeader::PfsHeader()
{}

nx::PfsHeader::PfsHeader(const PfsHeader & other)
{
	copyFrom(other);
}

nx::PfsHeader::PfsHeader(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

void nx::PfsHeader::exportBinary()
{
	// calculate name table size
	size_t name_table_size = 0;
	for (size_t i = 0; i < mFileList.getSize(); i++)
	{
		name_table_size += mFileList[i].name.length() + 1;
	}

	size_t pfs_header_size = align(sizeof(sPfsHeader) + sizeof(sPfsFile) * mFileList.getSize() + name_table_size, kPfsAlign);
	
	// align name_table_size
	name_table_size = pfs_header_size - (sizeof(sPfsHeader) + sizeof(sPfsFile) * mFileList.getSize());

	// allocate pfs header binary
	mBinaryBlob.alloc(pfs_header_size);
	sPfsHeader* hdr = (sPfsHeader*)mBinaryBlob.getBytes();

	// set header fields
	strncpy(hdr->signature, kPfsStructSig.c_str(), 4);
	hdr->file_num = mFileList.getSize();
	hdr->name_table_size = name_table_size;

	// set file entries
	sPfsFile* raw_files = (sPfsFile*)(mBinaryBlob.getBytes() + sizeof(sPfsHeader));
	char* raw_name_table = (char*)(mBinaryBlob.getBytes() + sizeof(sPfsHeader) + sizeof(sPfsFile) * mFileList.getSize());
	size_t raw_name_table_pos = 0;

	calculateOffsets(pfs_header_size);
	for (size_t i = 0; i < mFileList.getSize(); i++)
	{
		raw_files[i].data_offset = (mFileList[i].offset - pfs_header_size);
		raw_files[i].size = (mFileList[i].size);
		raw_files[i].name_offset = (raw_name_table_pos);

		strcpy(raw_name_table + raw_name_table_pos, mFileList[i].name.c_str());
		raw_name_table_pos += mFileList[i].name.length() + 1;
	}
}

void nx::PfsHeader::importBinary(const byte_t * bytes, size_t len)
{
	// check input length meets minimum size
	if (len < sizeof(sPfsHeader))
	{
		throw fnd::Exception(kModuleName, "PFS header too small");
	}
	
	// import minimum header
	mBinaryBlob.alloc(sizeof(sPfsHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	const sPfsHeader* hdr = (const sPfsHeader*)mBinaryBlob.getBytes();

	// check struct signature
	if (memcmp(hdr->signature, kPfsStructSig.c_str(), 4) != 0)
	{
		throw fnd::Exception(kModuleName, "PFS header corrupt");
	}

	// determine complete header size
	size_t pfs_full_header_size = sizeof(sPfsHeader) + sizeof(sPfsFile) * hdr->file_num.get() + hdr->name_table_size.get();

	// check input length meets complete size
	if (len < pfs_full_header_size)
	{
		throw fnd::Exception(kModuleName, "PFS header too small");
	}

	// import full header
	mBinaryBlob.alloc(pfs_full_header_size);
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	hdr = (const sPfsHeader*)mBinaryBlob.getBytes();

	// clear variables
	clear();

	// get pointers to raw data
	const sPfsFile* raw_files = (const sPfsFile*)(mBinaryBlob.getBytes() + sizeof(sPfsHeader));
	const char* raw_name_table = (const char*)(mBinaryBlob.getBytes() + sizeof(sPfsHeader) + sizeof(sPfsFile) * hdr->file_num.get());

	// process file entries
	for (size_t i = 0; i < hdr->file_num.get(); i++)
	{
		mFileList.addElement({ std::string(raw_name_table + raw_files[i].name_offset.get()), raw_files[i].data_offset.get() + pfs_full_header_size, raw_files[i].size.get() });
	}
}

void nx::PfsHeader::clear()
{
	mBinaryBlob.clear();
	mFileList.clear();
}

const fnd::List<nx::PfsHeader::sFile>& nx::PfsHeader::getFileList() const
{
	return mFileList;
}

void nx::PfsHeader::addFile(const std::string & name, size_t size)
{
	mFileList.addElement({ name, 0, size });
}

void nx::PfsHeader::calculateOffsets(size_t data_offset)
{
	for (size_t i = 0; i < mFileList.getSize(); i++)
	{
		mFileList[i].offset = (i == 0) ? data_offset : mFileList[i - 1].offset + mFileList[i - 1].size;
	}
}

bool nx::PfsHeader::isEqual(const PfsHeader & other) const
{
	return mFileList == other.mFileList;
}

void nx::PfsHeader::copyFrom(const PfsHeader & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		clear();
		mFileList = other.mFileList;
	}
}

bool nx::PfsHeader::operator==(const PfsHeader & other) const
{
	return isEqual(other);
}

bool nx::PfsHeader::operator!=(const PfsHeader & other) const
{
	return !isEqual(other);
}

void nx::PfsHeader::operator=(const PfsHeader & other)
{
	copyFrom(other);
}

const byte_t * nx::PfsHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::PfsHeader::getSize() const
{
	return mBinaryBlob.getSize();
}
