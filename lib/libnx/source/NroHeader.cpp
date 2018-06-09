#include <nx/NroHeader.h>

nx::NroHeader::NroHeader()
{
	clear();
}

nx::NroHeader::NroHeader(const NroHeader& other)
{
	copyFrom(other);
}

nx::NroHeader::NroHeader(const byte_t* bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::NroHeader::operator==(const NroHeader& other) const
{
	return isEqual(other);
}

bool nx::NroHeader::operator!=(const NroHeader& other) const
{
	return !(*this != other);
}

void nx::NroHeader::operator=(const NroHeader& other)
{
	copyFrom(other);
}

const byte_t* nx::NroHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::NroHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::NroHeader::exportBinary()
{
	throw fnd::Exception(kModuleName, "exportBinary() unsupported");
}

void nx::NroHeader::importBinary(const byte_t* bytes, size_t len)
{
	// check input data size
	if (len < sizeof(sNroHeader))
	{
		throw fnd::Exception(kModuleName, "NRO header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mBinaryBlob.alloc(sizeof(sNroHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	// get sNroHeader ptr
	const nx::sNroHeader* hdr = (const nx::sNroHeader*)mBinaryBlob.getBytes();
	
	// check NRO signature
	if (hdr->signature.get() != nro::kNroSig)
	{
		throw fnd::Exception(kModuleName, "NRO header corrupt (unrecognised header signature)");
	}

	// check NRO format version
	if (hdr->format_version.get() != nro::kDefaultFormatVersion)
	{
		throw fnd::Exception(kModuleName, "NRO header corrupt (unsupported format version)");
	}

	// check NRO flags
	if (hdr->flags.get() != 0)
	{
		throw fnd::Exception(kModuleName, "NRO header corrupt (unsupported flag)");
	}

	memcpy(mRoCrt.data, hdr->ro_crt, nro::kRoCrtSize);
	mNroSize = hdr->size.get();
	mTextInfo.memory_offset = hdr->text.memory_offset.get();
	mTextInfo.size = hdr->text.size.get();
	mRoInfo.memory_offset = hdr->ro.memory_offset.get();
	mRoInfo.size = hdr->ro.size.get();
	mDataInfo.memory_offset = hdr->data.memory_offset.get();
	mDataInfo.size = hdr->data.size.get();
	mBssSize = hdr->bss_size.get();
	memcpy(mModuleId.data, hdr->module_id, nro::kModuleIdSize);

	mRoEmbeddedInfo.memory_offset = hdr->embedded.memory_offset.get();
	mRoEmbeddedInfo.size = hdr->embedded.size.get();

	mRoDynStrInfo.memory_offset = hdr->dyn_str.memory_offset.get();
	mRoDynStrInfo.size = hdr->dyn_str.size.get();

	mRoDynSymInfo.memory_offset = hdr->dyn_sym.memory_offset.get();
	mRoDynSymInfo.size = hdr->dyn_sym.size.get();
}

void nx::NroHeader::clear()
{
	mBinaryBlob.clear();
	memset(&mRoCrt, 0, sizeof(mRoCrt));
	memset(&mTextInfo, 0, sizeof(mTextInfo));
	memset(&mRoInfo, 0, sizeof(mRoInfo));
	memset(&mDataInfo, 0, sizeof(mDataInfo));
	mBssSize = 0;
	memset(&mModuleId, 0, sizeof(mModuleId));
	memset(&mRoEmbeddedInfo, 0, sizeof(mRoEmbeddedInfo));
	memset(&mRoDynStrInfo, 0, sizeof(mRoDynStrInfo));
	memset(&mRoDynSymInfo, 0, sizeof(mRoDynSymInfo));
}

const nx::NroHeader::sRoCrt& nx::NroHeader::getRoCrt() const
{
	return mRoCrt;
}

void nx::NroHeader::setRoCrt(const sRoCrt& ro_crt)
{
	mRoCrt = ro_crt;
}

uint32_t nx::NroHeader::getNroSize() const
{
	return mNroSize;
}

void nx::NroHeader::setNroSize(uint32_t size)
{
	mNroSize = size;
}

const nx::NroHeader::sSection& nx::NroHeader::getTextInfo() const
{
	return mTextInfo;
}

void nx::NroHeader::setTextInfo(const sSection& info)
{
	mTextInfo = info;
}

const nx::NroHeader::sSection& nx::NroHeader::getRoInfo() const
{
	return mRoInfo;
}

void nx::NroHeader::setRoInfo(const sSection& info)
{
	mRoInfo = info;
}

const nx::NroHeader::sSection& nx::NroHeader::getDataInfo() const
{
	return mDataInfo;
}

void nx::NroHeader::setDataInfo(const sSection& info)
{
	mDataInfo = info;
}

uint32_t nx::NroHeader::getBssSize() const
{
	return mBssSize;
}

void nx::NroHeader::setBssSize(uint32_t size)
{
	mBssSize = size;
}

const nx::NroHeader::sModuleId& nx::NroHeader::getModuleId() const
{
	return mModuleId;
}

void nx::NroHeader::setModuleId(const sModuleId& id)
{
	mModuleId = id;
}

const nx::NroHeader::sSection& nx::NroHeader::getRoEmbeddedInfo() const
{
	return mRoEmbeddedInfo;
}

void nx::NroHeader::setRoEmbeddedInfo(const sSection& info)
{
	mRoEmbeddedInfo = info;
}

const nx::NroHeader::sSection& nx::NroHeader::getRoDynStrInfo() const
{
	return mRoDynStrInfo;
}

void nx::NroHeader::setRoDynStrInfo(const sSection& info)
{
	mRoDynStrInfo = info;
}

const nx::NroHeader::sSection& nx::NroHeader::getRoDynSymInfo() const
{
	return mRoDynSymInfo;
}

void nx::NroHeader::setRoDynSymInfo(const sSection& info)
{
	mRoDynSymInfo = info;
}

bool nx::NroHeader::isEqual(const NroHeader& other) const
{
	return (mRoCrt == other.mRoCrt) \
		&& (mNroSize == other.mNroSize) \
		&& (mTextInfo == other.mTextInfo) \
		&& (mTextInfo == other.mTextInfo) \
		&& (mRoInfo == other.mRoInfo) \
		&& (mDataInfo == other.mDataInfo) \
		&& (mBssSize == other.mBssSize) \
		&& (mModuleId == other.mModuleId) \
		&& (mRoEmbeddedInfo == other.mRoEmbeddedInfo) \
		&& (mRoDynStrInfo == other.mRoDynStrInfo) \
		&& (mRoDynSymInfo == other.mRoDynSymInfo);
}

void nx::NroHeader::copyFrom(const NroHeader& other)
{
	clear();
	mRoCrt = other.mRoCrt;
	mNroSize = other.mNroSize;
	mTextInfo = other.mTextInfo;
	mTextInfo = other.mTextInfo;
	mRoInfo = other.mRoInfo;
	mDataInfo = other.mDataInfo;
	mBssSize = other.mBssSize;
	mModuleId = other.mModuleId;
	mRoEmbeddedInfo = other.mRoEmbeddedInfo;
	mRoDynStrInfo = other.mRoDynStrInfo;
	mRoDynSymInfo = other.mRoDynSymInfo;
}