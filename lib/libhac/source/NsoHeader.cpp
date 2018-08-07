#include <nn/hac/NsoHeader.h>

nn::hac::NsoHeader::NsoHeader()
{
	clear();
}

nn::hac::NsoHeader::NsoHeader(const NsoHeader& other)
{
	*this = other;
}

void nn::hac::NsoHeader::operator=(const NsoHeader& other)
{
	clear();
	mModuleId = other.mModuleId;
	mBssSize = other.mBssSize;
	mTextSegmentInfo = other.mTextSegmentInfo;
	mRoSegmentInfo = other.mRoSegmentInfo;
	mDataSegmentInfo = other.mDataSegmentInfo;
	mModuleNameInfo = other.mModuleNameInfo;
	mRoEmbeddedInfo = other.mRoEmbeddedInfo;
	mRoDynStrInfo = other.mRoDynStrInfo;
	mRoDynSymInfo = other.mRoDynSymInfo;
}


bool nn::hac::NsoHeader::operator==(const NsoHeader& other) const
{
	return (mModuleId == other.mModuleId) \
		&& (mBssSize == other.mBssSize) \
		&& (mTextSegmentInfo == other.mTextSegmentInfo) \
		&& (mRoSegmentInfo == other.mRoSegmentInfo) \
		&& (mDataSegmentInfo == other.mDataSegmentInfo) \
		&& (mModuleNameInfo == other.mModuleNameInfo) \
		&& (mRoEmbeddedInfo == other.mRoEmbeddedInfo) \
		&& (mRoDynStrInfo == other.mRoDynStrInfo) \
		&& (mRoDynSymInfo == other.mRoDynSymInfo);
}

bool nn::hac::NsoHeader::operator!=(const NsoHeader& other) const
{
	return !(*this == other);
}

void nn::hac::NsoHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sNsoHeader));
	nn::hac::sNsoHeader* hdr = (nn::hac::sNsoHeader*)mRawBinary.data();

	// set header identifers
	hdr->st_magic = nso::kNsoStructMagic;
	hdr->format_version = nso::kDefaultFormatVersion;
	
	// variable to store flags before commiting to header
	uint32_t flags = 0;

	// set moduleid
	memcpy(hdr->module_id, mModuleId.data, nso::kModuleIdSize);

	// set bss size
	hdr->bss_size = mBssSize;

	// set text segment
	hdr->text.file_offset = mTextSegmentInfo.file_layout.offset;
	hdr->text.memory_offset = mTextSegmentInfo.memory_layout.offset;
	hdr->text.size = mTextSegmentInfo.memory_layout.size;
	hdr->text_file_size = mTextSegmentInfo.file_layout.size;
	if (mTextSegmentInfo.is_compressed)
	{
		flags |= _BIT(nso::FLAG_TEXT_COMPRESS);
	}
	if (mTextSegmentInfo.is_hashed)
	{
		flags |= _BIT(nso::FLAG_TEXT_HASH);
		hdr->text_hash = mTextSegmentInfo.hash;
	}

	// set ro segment
	hdr->ro.file_offset = mRoSegmentInfo.file_layout.offset;
	hdr->ro.memory_offset = mRoSegmentInfo.memory_layout.offset;
	hdr->ro.size = mRoSegmentInfo.memory_layout.size;
	hdr->ro_file_size = mRoSegmentInfo.file_layout.size;
	if (mRoSegmentInfo.is_compressed)
	{
		flags |= _BIT(nso::FLAG_RO_COMPRESS);
	}
	if (mRoSegmentInfo.is_hashed)
	{
		flags |= _BIT(nso::FLAG_RO_HASH);
		hdr->ro_hash = mRoSegmentInfo.hash;
	}

	// set data segment
	hdr->data.file_offset = mDataSegmentInfo.file_layout.offset;
	hdr->data.memory_offset = mDataSegmentInfo.memory_layout.offset;
	hdr->data.size = mDataSegmentInfo.memory_layout.size;
	hdr->data_file_size = mDataSegmentInfo.file_layout.size;
	if (mDataSegmentInfo.is_compressed)
	{
		flags |= _BIT(nso::FLAG_DATA_COMPRESS);
	}
	if (mDataSegmentInfo.is_hashed)
	{
		flags |= _BIT(nso::FLAG_DATA_HASH);
		hdr->data_hash = mDataSegmentInfo.hash;
	}
		
	// set module name info
	hdr->module_name_offset = mModuleNameInfo.offset;
	hdr->module_name_size = mModuleNameInfo.size;

	// set ro embedded info
	hdr->embedded.offset = mRoEmbeddedInfo.offset;
	hdr->embedded.size = mRoEmbeddedInfo.size;

	// set ro dyn str info
	hdr->dyn_str.offset = mRoDynStrInfo.offset;
	hdr->dyn_str.size = mRoDynStrInfo.size;

	// set ro dyn sym info
	hdr->dyn_sym.offset = mRoDynSymInfo.offset;
	hdr->dyn_sym.size = mRoDynSymInfo.size;

	hdr->flags = flags;
}

void nn::hac::NsoHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input data size
	if (len < sizeof(sNsoHeader))
	{
		throw fnd::Exception(kModuleName, "NSO header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sNsoHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// get sNsoHeader ptr
	const nn::hac::sNsoHeader* hdr = (const nn::hac::sNsoHeader*)mRawBinary.data();
	
	// check NSO signature
	if (hdr->st_magic.get() != nso::kNsoStructMagic)
	{
		throw fnd::Exception(kModuleName, "NSO header corrupt (unrecognised header signature)");
	}

	// check NSO format version
	if (hdr->format_version.get() != nso::kDefaultFormatVersion)
	{
		throw fnd::Exception(kModuleName, "NSO header corrupt (unsupported format version)");
	}

	memcpy(mModuleId.data, hdr->module_id, nso::kModuleIdSize);

	mBssSize = hdr->bss_size.get();

	mTextSegmentInfo.file_layout.offset = hdr->text.file_offset.get();
	mTextSegmentInfo.file_layout.size = hdr->text_file_size.get();
	mTextSegmentInfo.memory_layout.offset = hdr->text.memory_offset.get();
	mTextSegmentInfo.memory_layout.size = hdr->text.size.get();
	mTextSegmentInfo.is_compressed = _HAS_BIT(hdr->flags.get(), nso::FLAG_TEXT_COMPRESS);
	mTextSegmentInfo.is_hashed = _HAS_BIT(hdr->flags.get(), nso::FLAG_TEXT_HASH);
	mTextSegmentInfo.hash = hdr->text_hash;

	mRoSegmentInfo.file_layout.offset = hdr->ro.file_offset.get();
	mRoSegmentInfo.file_layout.size = hdr->ro_file_size.get();
	mRoSegmentInfo.memory_layout.offset = hdr->ro.memory_offset.get();
	mRoSegmentInfo.memory_layout.size = hdr->ro.size.get();
	mRoSegmentInfo.is_compressed = _HAS_BIT(hdr->flags.get(), nso::FLAG_RO_COMPRESS);
	mRoSegmentInfo.is_hashed = _HAS_BIT(hdr->flags.get(), nso::FLAG_RO_HASH);
	mRoSegmentInfo.hash = hdr->ro_hash;

	mDataSegmentInfo.file_layout.offset = hdr->data.file_offset.get();
	mDataSegmentInfo.file_layout.size = hdr->data_file_size.get();
	mDataSegmentInfo.memory_layout.offset = hdr->data.memory_offset.get();
	mDataSegmentInfo.memory_layout.size = hdr->data.size.get();
	mDataSegmentInfo.is_compressed = _HAS_BIT(hdr->flags.get(), nso::FLAG_RO_COMPRESS);
	mDataSegmentInfo.is_hashed = _HAS_BIT(hdr->flags.get(), nso::FLAG_RO_HASH);
	mDataSegmentInfo.hash = hdr->data_hash;

	mModuleNameInfo.offset = hdr->module_name_offset.get();
	mModuleNameInfo.size = hdr->module_name_size.get();

	mRoEmbeddedInfo.offset = hdr->embedded.offset.get();
	mRoEmbeddedInfo.size = hdr->embedded.size.get();

	mRoDynStrInfo.offset = hdr->dyn_str.offset.get();
	mRoDynStrInfo.size = hdr->dyn_str.size.get();

	mRoDynSymInfo.offset = hdr->dyn_sym.offset.get();
	mRoDynSymInfo.size = hdr->dyn_sym.size.get();
}

const fnd::Vec<byte_t>& nn::hac::NsoHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::NsoHeader::clear()
{
	mRawBinary.clear();
	memset(&mModuleId, 0, sizeof(mModuleId));
	mBssSize = 0;
	memset(&mTextSegmentInfo, 0, sizeof(mTextSegmentInfo));
	memset(&mRoSegmentInfo, 0, sizeof(mRoSegmentInfo));
	memset(&mDataSegmentInfo, 0, sizeof(mDataSegmentInfo));
	memset(&mModuleNameInfo, 0, sizeof(mModuleNameInfo));
	memset(&mRoEmbeddedInfo, 0, sizeof(mRoEmbeddedInfo));
	memset(&mRoDynStrInfo, 0, sizeof(mRoDynStrInfo));
	memset(&mRoDynSymInfo, 0, sizeof(mRoDynSymInfo));
}

const nn::hac::NsoHeader::sModuleId& nn::hac::NsoHeader::getModuleId() const
{
	return mModuleId;
}

void nn::hac::NsoHeader::setModuleId(const sModuleId& id)
{
	mModuleId = id;
}

uint32_t nn::hac::NsoHeader::getBssSize() const
{
	return mBssSize;
}

void nn::hac::NsoHeader::setBssSize(uint32_t size)
{
	mBssSize = size;
}

const nn::hac::NsoHeader::sCodeSegment& nn::hac::NsoHeader::getTextSegmentInfo() const
{
	return mTextSegmentInfo;
}

void nn::hac::NsoHeader::setTextSegmentInfo(const sCodeSegment& info)
{
	mTextSegmentInfo = info;
}

const nn::hac::NsoHeader::sCodeSegment& nn::hac::NsoHeader::getRoSegmentInfo() const
{
	return mRoSegmentInfo;
}

void nn::hac::NsoHeader::setRoSegmentInfo(const sCodeSegment& info)
{
	mRoSegmentInfo = info;
}

const nn::hac::NsoHeader::sCodeSegment& nn::hac::NsoHeader::getDataSegmentInfo() const
{
	return mDataSegmentInfo;
}

void nn::hac::NsoHeader::setDataSegmentInfo(const sCodeSegment& info)
{
	mDataSegmentInfo = info;
}

const nn::hac::NsoHeader::sLayout& nn::hac::NsoHeader::getModuleNameInfo() const
{
	return mModuleNameInfo;
}

void nn::hac::NsoHeader::setModuleNameInfo(const sLayout& info)
{
	mModuleNameInfo = info;
}

const nn::hac::NsoHeader::sLayout& nn::hac::NsoHeader::getRoEmbeddedInfo() const
{
	return mRoEmbeddedInfo;
}

void nn::hac::NsoHeader::setRoEmbeddedInfo(const sLayout& info)
{
	mRoEmbeddedInfo = info;
}

const nn::hac::NsoHeader::sLayout& nn::hac::NsoHeader::getRoDynStrInfo() const
{
	return mRoDynStrInfo;
}

void nn::hac::NsoHeader::setRoDynStrInfo(const sLayout& info)
{
	mRoDynStrInfo = info;
}

const nn::hac::NsoHeader::sLayout& nn::hac::NsoHeader::getRoDynSymInfo() const
{
	return mRoDynSymInfo;
}

void nn::hac::NsoHeader::setRoDynSymInfo(const sLayout& info)
{
	mRoDynSymInfo = info;
}