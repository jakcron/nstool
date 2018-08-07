#include <nn/hac/PfsHeader.h>

nn::hac::PfsHeader::PfsHeader()
{
	clear();
}

nn::hac::PfsHeader::PfsHeader(const PfsHeader & other)
{
	*this = other;
}

void nn::hac::PfsHeader::operator=(const PfsHeader & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mFsType = other.mFsType;
		mFileList = other.mFileList;
	}
}

bool nn::hac::PfsHeader::operator==(const PfsHeader & other) const
{
	return (mFsType == other.mFsType) \
		&& (mFileList == other.mFileList);
}

bool nn::hac::PfsHeader::operator!=(const PfsHeader & other) const
{
	return !(*this == other);
}

const fnd::Vec<byte_t>& nn::hac::PfsHeader::getBytes() const
{
	return mRawBinary;
}


void nn::hac::PfsHeader::toBytes()
{
	// calculate name table size
	size_t name_table_size = 0;
	for (size_t i = 0; i < mFileList.size(); i++)
	{
		name_table_size += mFileList[i].name.length() + 1;
	}

	size_t pfs_header_size = align(sizeof(sPfsHeader) + getFileEntrySize(mFsType) * mFileList.size() + name_table_size, pfs::kHeaderAlign);
	
	// align name_table_size
	name_table_size = pfs_header_size - (sizeof(sPfsHeader) + getFileEntrySize(mFsType) * mFileList.size());

	// allocate pfs header binary
	mRawBinary.alloc(pfs_header_size);
	sPfsHeader* hdr = (sPfsHeader*)mRawBinary.data();

	// set header fields
	switch (mFsType)
	{
		case (TYPE_PFS0):
			hdr->st_magic = pfs::kPfsStructMagic;
			break;
		case (TYPE_HFS0):
			hdr->st_magic = pfs::kHashedPfsStructMagic;
			break;
	}
	
	hdr->file_num = (uint32_t)mFileList.size();
	hdr->name_table_size = (uint32_t)name_table_size;

	// set file entries
	if (mFsType == TYPE_PFS0)
	{
		sPfsFile* raw_files = (sPfsFile*)(mRawBinary.data() + sizeof(sPfsHeader));
		char* raw_name_table = (char*)(mRawBinary.data() + sizeof(sPfsHeader) + sizeof(sPfsFile) * mFileList.size());
		size_t raw_name_table_pos = 0;

		calculateOffsets(pfs_header_size);
		for (size_t i = 0; i < mFileList.size(); i++)
		{
			raw_files[i].data_offset = (mFileList[i].offset - pfs_header_size);
			raw_files[i].size = mFileList[i].size;
			raw_files[i].name_offset = (uint32_t)raw_name_table_pos;

			strcpy(raw_name_table + raw_name_table_pos, mFileList[i].name.c_str());
			raw_name_table_pos += (uint32_t)(mFileList[i].name.length() + 1);
		}
	}
	else if (mFsType == TYPE_HFS0)
	{
		sHashedPfsFile* raw_files = (sHashedPfsFile*)(mRawBinary.data() + sizeof(sPfsHeader));
		char* raw_name_table = (char*)(mRawBinary.data() + sizeof(sPfsHeader) + sizeof(sHashedPfsFile) * mFileList.size());
		size_t raw_name_table_pos = 0;

		calculateOffsets(pfs_header_size);
		for (size_t i = 0; i < mFileList.size(); i++)
		{
			raw_files[i].data_offset = (mFileList[i].offset - pfs_header_size);
			raw_files[i].size = mFileList[i].size;
			raw_files[i].name_offset = (uint32_t)raw_name_table_pos;
			raw_files[i].hash_protected_size = (uint32_t)mFileList[i].hash_protected_size;
			raw_files[i].hash = mFileList[i].hash;

			strcpy(raw_name_table + raw_name_table_pos, mFileList[i].name.c_str());
			raw_name_table_pos += mFileList[i].name.length() + 1;
		}
	}
	
}

void nn::hac::PfsHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input length meets minimum size
	if (len < sizeof(sPfsHeader))
	{
		throw fnd::Exception(kModuleName, "PFS header too small");
	}

	// clear variables
	clear();
	
	// import minimum header
	mRawBinary.alloc(sizeof(sPfsHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());
	const sPfsHeader* hdr = (const sPfsHeader*)mRawBinary.data();

	// check struct signature
	FsType fs_type;
	switch(hdr->st_magic.get())
	{
		case (pfs::kPfsStructMagic):
			fs_type = TYPE_PFS0;
			break;
		case (pfs::kHashedPfsStructMagic):
			fs_type = TYPE_HFS0;
			break;	
		default:
			throw fnd::Exception(kModuleName, "PFS header corrupt");
	}

	// determine complete header size
	size_t pfs_full_header_size = sizeof(sPfsHeader) + getFileEntrySize(fs_type) * hdr->file_num.get() + hdr->name_table_size.get();

	// check input length meets complete size
	if (len < pfs_full_header_size)
	{
		throw fnd::Exception(kModuleName, "PFS header too small");
	}

	// import full header
	mRawBinary.alloc(pfs_full_header_size);
	memcpy(mRawBinary.data(), data, mRawBinary.size());
	hdr = (const sPfsHeader*)mRawBinary.data();

	mFsType = fs_type;
	if (mFsType == TYPE_PFS0)
	{
		// get pointers to raw data
		const sPfsFile* raw_files = (const sPfsFile*)(mRawBinary.data() + sizeof(sPfsHeader));
		const char* raw_name_table = (const char*)(mRawBinary.data() + sizeof(sPfsHeader) + sizeof(sPfsFile) * hdr->file_num.get());

		// process file entries
		for (size_t i = 0; i < hdr->file_num.get(); i++)
		{
			mFileList.addElement({ 
				std::string(raw_name_table + raw_files[i].name_offset.get()), 
				raw_files[i].data_offset.get() + pfs_full_header_size, 
				raw_files[i].size.get() 
				});
		}
	}
	else if (mFsType == TYPE_HFS0)
	{
		// get pointers to raw data
		const sHashedPfsFile* raw_files = (const sHashedPfsFile*)(mRawBinary.data() + sizeof(sPfsHeader));
		const char* raw_name_table = (const char*)(mRawBinary.data() + sizeof(sPfsHeader) + sizeof(sHashedPfsFile) * hdr->file_num.get());

		// process file entries
		for (size_t i = 0; i < hdr->file_num.get(); i++)
		{
			mFileList.addElement({ 
				std::string(raw_name_table + raw_files[i].name_offset.get()), 
				raw_files[i].data_offset.get() + pfs_full_header_size, 
				raw_files[i].size.get(),
				raw_files[i].hash_protected_size.get(),
				raw_files[i].hash 
				});
		}
	}
	
}

void nn::hac::PfsHeader::clear()
{
	mRawBinary.clear();
	mFsType = TYPE_PFS0;
	mFileList.clear();
}

nn::hac::PfsHeader::FsType nn::hac::PfsHeader::getFsType() const
{
	return mFsType;
}

void nn::hac::PfsHeader::setFsType(FsType type)
{
	mFsType = type;
}

const fnd::List<nn::hac::PfsHeader::sFile>& nn::hac::PfsHeader::getFileList() const
{
	return mFileList;
}

void nn::hac::PfsHeader::addFile(const std::string & name, size_t size)
{
	mFileList.addElement({ name, 0, size, 0 });
}

void nn::hac::PfsHeader::addFile(const std::string & name, size_t size, size_t hash_protected_size, const fnd::sha::sSha256Hash& hash)
{
	mFileList.addElement({ name, 0, size, hash_protected_size, hash });
}

size_t nn::hac::PfsHeader::getFileEntrySize(FsType fs_type)
{
	size_t size = 0;
	switch(fs_type)
	{
		case (TYPE_PFS0):
			size = sizeof(sPfsFile);
			break;
		case (TYPE_HFS0):
			size = sizeof(sHashedPfsFile);
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown PFS type");
	}
	return size;
}

void nn::hac::PfsHeader::calculateOffsets(size_t data_offset)
{
	for (size_t i = 0; i < mFileList.size(); i++)
	{
		mFileList[i].offset = (i == 0) ? data_offset : mFileList[i - 1].offset + mFileList[i - 1].size;
	}
}