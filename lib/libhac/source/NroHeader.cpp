#include <nn/hac/NroHeader.h>

nn::hac::NroHeader::NroHeader()
{
	clear();
}

nn::hac::NroHeader::NroHeader(const NroHeader& other)
{
	*this = other;
}

void nn::hac::NroHeader::operator=(const NroHeader& other)
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

bool nn::hac::NroHeader::operator==(const NroHeader& other) const
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

bool nn::hac::NroHeader::operator!=(const NroHeader& other) const
{
	return !(*this == other);
}

void nn::hac::NroHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sNroHeader));
	nn::hac::sNroHeader* hdr = (nn::hac::sNroHeader*)mRawBinary.data();

	// set header identifers
	hdr->st_magic = nro::kNroStructMagic;
	hdr->format_version = nro::kDefaultFormatVersion;
	hdr->flags = 0;

	// set ro crt
	memcpy(hdr->ro_crt, mRoCrt.data, nro::kRoCrtSize);

	// set nro size
	hdr->size = mNroSize;

	// set text section
	hdr->text.memory_offset = mTextInfo.memory_offset;
	hdr->text.size = mTextInfo.size;

	// set ro section
	hdr->ro.memory_offset = mRoInfo.memory_offset;
	hdr->ro.size = mRoInfo.size;

	// set data section
	hdr->data.memory_offset = mDataInfo.memory_offset;
	hdr->data.size = mDataInfo.size;

	// set bss size
	hdr->bss_size = mBssSize;

	// set moduleid
	memcpy(hdr->module_id, mModuleId.data, nro::kModuleIdSize);

	// set ro embedded info
	hdr->embedded.memory_offset = mRoEmbeddedInfo.memory_offset;
	hdr->embedded.size = mRoEmbeddedInfo.size;

	// set ro dyn str info
	hdr->dyn_str.memory_offset = mRoDynStrInfo.memory_offset;
	hdr->dyn_str.size = mRoDynStrInfo.size;

	// set ro dyn sym info
	hdr->dyn_sym.memory_offset = mRoDynSymInfo.memory_offset;
	hdr->dyn_sym.size = mRoDynSymInfo.size;
}

void nn::hac::NroHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input data size
	if (len < sizeof(sNroHeader))
	{
		throw fnd::Exception(kModuleName, "NRO header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sNroHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// get sNroHeader ptr
	const nn::hac::sNroHeader* hdr = (const nn::hac::sNroHeader*)mRawBinary.data();
	
	// check NRO signature
	if (hdr->st_magic.get() != nro::kNroStructMagic)
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

const fnd::Vec<byte_t>& nn::hac::NroHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::NroHeader::clear()
{
	mRawBinary.clear();
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

const nn::hac::NroHeader::sRoCrt& nn::hac::NroHeader::getRoCrt() const
{
	return mRoCrt;
}

void nn::hac::NroHeader::setRoCrt(const sRoCrt& ro_crt)
{
	mRoCrt = ro_crt;
}

uint32_t nn::hac::NroHeader::getNroSize() const
{
	return mNroSize;
}

void nn::hac::NroHeader::setNroSize(uint32_t size)
{
	mNroSize = size;
}

const nn::hac::NroHeader::sSection& nn::hac::NroHeader::getTextInfo() const
{
	return mTextInfo;
}

void nn::hac::NroHeader::setTextInfo(const sSection& info)
{
	mTextInfo = info;
}

const nn::hac::NroHeader::sSection& nn::hac::NroHeader::getRoInfo() const
{
	return mRoInfo;
}

void nn::hac::NroHeader::setRoInfo(const sSection& info)
{
	mRoInfo = info;
}

const nn::hac::NroHeader::sSection& nn::hac::NroHeader::getDataInfo() const
{
	return mDataInfo;
}

void nn::hac::NroHeader::setDataInfo(const sSection& info)
{
	mDataInfo = info;
}

uint32_t nn::hac::NroHeader::getBssSize() const
{
	return mBssSize;
}

void nn::hac::NroHeader::setBssSize(uint32_t size)
{
	mBssSize = size;
}

const nn::hac::NroHeader::sModuleId& nn::hac::NroHeader::getModuleId() const
{
	return mModuleId;
}

void nn::hac::NroHeader::setModuleId(const sModuleId& id)
{
	mModuleId = id;
}

const nn::hac::NroHeader::sSection& nn::hac::NroHeader::getRoEmbeddedInfo() const
{
	return mRoEmbeddedInfo;
}

void nn::hac::NroHeader::setRoEmbeddedInfo(const sSection& info)
{
	mRoEmbeddedInfo = info;
}

const nn::hac::NroHeader::sSection& nn::hac::NroHeader::getRoDynStrInfo() const
{
	return mRoDynStrInfo;
}

void nn::hac::NroHeader::setRoDynStrInfo(const sSection& info)
{
	mRoDynStrInfo = info;
}

const nn::hac::NroHeader::sSection& nn::hac::NroHeader::getRoDynSymInfo() const
{
	return mRoDynSymInfo;
}

void nn::hac::NroHeader::setRoDynSymInfo(const sSection& info)
{
	mRoDynSymInfo = info;
}