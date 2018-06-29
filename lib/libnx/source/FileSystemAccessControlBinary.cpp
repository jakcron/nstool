#include <nx/FileSystemAccessControlBinary.h>

#include <fnd/SimpleTextOutput.h>

nx::FileSystemAccessControlBinary::FileSystemAccessControlBinary()
{
	clear();
}

nx::FileSystemAccessControlBinary::FileSystemAccessControlBinary(const FileSystemAccessControlBinary & other)
{
	*this = other;
}

void nx::FileSystemAccessControlBinary::operator=(const FileSystemAccessControlBinary & other)
{
	mRawBinary = other.mRawBinary;
	mVersion = other.mVersion;
	mFsaRights = other.mFsaRights;
	mContentOwnerIdList = other.mContentOwnerIdList;
	mSaveDataOwnerIdList = other.mSaveDataOwnerIdList;
}

bool nx::FileSystemAccessControlBinary::operator==(const FileSystemAccessControlBinary & other) const
{
	return (mVersion == other.mVersion) \
		&& (mFsaRights == other.mFsaRights) \
		&& (mContentOwnerIdList == other.mContentOwnerIdList) \
		&& (mSaveDataOwnerIdList == other.mSaveDataOwnerIdList);
}

bool nx::FileSystemAccessControlBinary::operator!=(const FileSystemAccessControlBinary & other) const
{
	return !(*this == other);
}

void nx::FileSystemAccessControlBinary::toBytes()
{
	// determine section layout
	struct sLayout {
		uint32_t offset, size;
	} content, savedata;

	content.offset = align(sizeof(sFacHeader), fac::kSectionAlignSize);
	content.size = (uint32_t)(mContentOwnerIdList.size() * sizeof(uint32_t));
	savedata.offset = content.offset + align(content.size, fac::kSectionAlignSize);
	savedata.size = (uint32_t)(mSaveDataOwnerIdList.size() * sizeof(uint32_t));

	// get total size
	size_t total_size = _MAX(_MAX(content.offset + content.size, savedata.offset + savedata.size), align(sizeof(sFacHeader), fac::kSectionAlignSize)); 

	mRawBinary.alloc(total_size);
	sFacHeader* hdr = (sFacHeader*)mRawBinary.data();

	// set type
	hdr->version = mVersion;

	// flags
	uint64_t flag = 0;
	for (size_t i = 0; i < mFsaRights.size(); i++)
	{
		flag |= _BIT((uint64_t)mFsaRights[i]);
	}
	hdr->fac_flags = flag;

	// set offset/size
	hdr->content_owner_ids.start = content.offset;
	if (content.size > 0)
		hdr->content_owner_ids.end = content.offset + content.size;
	hdr->save_data_owner_ids.start = savedata.offset;
	if (savedata.size > 0)
		hdr->save_data_owner_ids.end = savedata.offset + savedata.size;

	// set ids
	le_uint32_t* content_owner_ids = (le_uint32_t*)(mRawBinary.data() + content.offset);
	for (size_t i = 0; i < mContentOwnerIdList.size(); i++)
	{
		content_owner_ids[i] = mContentOwnerIdList[i];
	}

	le_uint32_t* save_data_owner_ids = (le_uint32_t*)(mRawBinary.data() + savedata.offset);
	for (size_t i = 0; i < mSaveDataOwnerIdList.size(); i++)
	{
		save_data_owner_ids[i] = mSaveDataOwnerIdList[i];
	}
}

void nx::FileSystemAccessControlBinary::fromBytes(const byte_t* data, size_t len)
{
	// check size
	if (len < sizeof(sFacHeader))
	{
		throw fnd::Exception(kModuleName, "FileSystemAccessControlInfo binary is too small");
	}
	
	// clear variables
	clear();

	// save a copy of the header
	sFacHeader hdr;
	memcpy((void*)&hdr, data, sizeof(sFacHeader));

	// check format version
	if (hdr.version.get() != fac::kFacFormatVersion)
	{
		throw fnd::Exception(kModuleName, "FileSystemAccessControlInfo format version unsupported");
	}
	
	// get total size
	size_t total_size = _MAX(_MAX(hdr.content_owner_ids.end.get(), hdr.save_data_owner_ids.end.get()), align(sizeof(sFacHeader), fac::kSectionAlignSize)); 

	// validate binary size
	if (len < total_size)
	{
		throw fnd::Exception(kModuleName, "FileSystemAccessControlInfo binary is too small");
	}

	// allocate memory
	mRawBinary.alloc(total_size);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// save variables
	mVersion = hdr.version.get();
	for (size_t i = 0; i < 64; i++)
	{
		if (_HAS_BIT(hdr.fac_flags.get(), i))
		{
			mFsaRights.addElement((fac::FsAccessFlag)i);
		}
	}

	// save ids
	if (hdr.content_owner_ids.end.get() > hdr.content_owner_ids.start.get())
	{
		le_uint32_t* content_owner_ids = (le_uint32_t*)(mRawBinary.data() + hdr.content_owner_ids.start.get());
		size_t content_owner_id_num = (hdr.content_owner_ids.end.get() - hdr.content_owner_ids.start.get()) / sizeof(uint32_t);
		for (size_t i = 0; i < content_owner_id_num; i++)
		{
			mContentOwnerIdList.addElement(content_owner_ids[i].get());
		}
	}
	if (hdr.save_data_owner_ids.end.get() > hdr.save_data_owner_ids.start.get())
	{
		le_uint32_t* save_data_owner_ids = (le_uint32_t*)(mRawBinary.data() + hdr.save_data_owner_ids.start.get());
		size_t save_data_owner_id_num = (hdr.save_data_owner_ids.end.get() - hdr.save_data_owner_ids.start.get()) / sizeof(uint32_t);
		for (size_t i = 0; i < save_data_owner_id_num; i++)
		{
			mSaveDataOwnerIdList.addElement(save_data_owner_ids[i].get());
		}
	}
}

const fnd::Vec<byte_t>& nx::FileSystemAccessControlBinary::getBytes() const
{
	return mRawBinary;
}

void nx::FileSystemAccessControlBinary::clear()
{
	mRawBinary.clear();
	mVersion = 0;
	mFsaRights.clear();
	mContentOwnerIdList.clear();
	mSaveDataOwnerIdList.clear();
}

uint32_t nx::FileSystemAccessControlBinary::getFormatVersion() const
{
	return mVersion;
}

void nx::FileSystemAccessControlBinary::setFormatVersion(uint32_t format_version)
{
	mVersion = format_version;
}

const fnd::List<nx::fac::FsAccessFlag>& nx::FileSystemAccessControlBinary::getFsaRightsList() const
{
	return mFsaRights;
}

void nx::FileSystemAccessControlBinary::setFsaRightsList(const fnd::List<fac::FsAccessFlag>& list)
{
	mFsaRights = list;
}

const fnd::List<uint32_t>& nx::FileSystemAccessControlBinary::getContentOwnerIdList() const
{
	return mContentOwnerIdList;
}

void nx::FileSystemAccessControlBinary::setContentOwnerIdList(const fnd::List<uint32_t>& list)
{
	mContentOwnerIdList = list;
}

const fnd::List<uint32_t>& nx::FileSystemAccessControlBinary::getSaveDataOwnerIdList() const
{
	return mSaveDataOwnerIdList;
}

void nx::FileSystemAccessControlBinary::setSaveDataOwnerIdList(const fnd::List<uint32_t>& list)
{
	mSaveDataOwnerIdList = list;
}