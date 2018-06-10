#include <nx/NroAssetHeader.h>

nx::NroAssetHeader::NroAssetHeader()
{
	clear();
}

nx::NroAssetHeader::NroAssetHeader(const NroAssetHeader& other)
{
	copyFrom(other);
}

nx::NroAssetHeader::NroAssetHeader(const byte_t* bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::NroAssetHeader::operator==(const NroAssetHeader& other) const
{
	return isEqual(other);
}

bool nx::NroAssetHeader::operator!=(const NroAssetHeader& other) const
{
	return !(*this == other);
}

void nx::NroAssetHeader::operator=(const NroAssetHeader& other)
{
	copyFrom(other);
}

const byte_t* nx::NroAssetHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::NroAssetHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::NroAssetHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sNroAssetHeader));
	nx::sNroAssetHeader* hdr = (nx::sNroAssetHeader*)mBinaryBlob.getBytes();

	// set header identifers
	hdr->signature = nro::kAssetSig;
	hdr->format_version = nro::kDefaultAssetFormatVersion;

	// set icon section
	hdr->icon.offset = mIconInfo.offset;
	hdr->icon.size = mIconInfo.size;

	// set nacp section
	hdr->nacp.offset = mNacpInfo.offset;
	hdr->nacp.size = mNacpInfo.size;

	// set romfs section
	hdr->romfs.offset = mRomfsInfo.offset;
	hdr->romfs.size = mRomfsInfo.size;
}

void nx::NroAssetHeader::importBinary(const byte_t* bytes, size_t len)
{
	// check input data size
	if (len < sizeof(sNroAssetHeader))
	{
		throw fnd::Exception(kModuleName, "NRO Asset header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mBinaryBlob.alloc(sizeof(sNroAssetHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	// get sNroAssetHeader ptr
	const nx::sNroAssetHeader* hdr = (const nx::sNroAssetHeader*)mBinaryBlob.getBytes();
	
	// check NRO signature
	if (hdr->signature.get() != nro::kAssetSig)
	{
		throw fnd::Exception(kModuleName, "NRO Asset header corrupt (unrecognised header signature)");
	}

	// check NRO format version
	if (hdr->format_version.get() != nro::kDefaultAssetFormatVersion)
	{
		throw fnd::Exception(kModuleName, "NRO Asset header corrupt (unsupported format version)");
	}

	mIconInfo.offset = hdr->icon.offset.get();
	mIconInfo.size = hdr->icon.size.get();
	mNacpInfo.offset = hdr->nacp.offset.get();
	mNacpInfo.size = hdr->nacp.size.get();
	mRomfsInfo.offset = hdr->romfs.offset.get();
	mRomfsInfo.size = hdr->romfs.size.get();
}

void nx::NroAssetHeader::clear()
{
	mBinaryBlob.clear();
	memset(&mIconInfo, 0, sizeof(mIconInfo));
	memset(&mNacpInfo, 0, sizeof(mNacpInfo));
	memset(&mRomfsInfo, 0, sizeof(mRomfsInfo));
}

const nx::NroAssetHeader::sSection& nx::NroAssetHeader::getIconInfo() const
{
	return mIconInfo;
}

void nx::NroAssetHeader::setIconInfo(const nx::NroAssetHeader::sSection& info)
{
	mIconInfo = info;
}

const nx::NroAssetHeader::sSection& nx::NroAssetHeader::getNacpInfo() const
{
	return mNacpInfo;
}

void nx::NroAssetHeader::setNacpInfo(const sSection& info)
{
	mNacpInfo = info;
}

const nx::NroAssetHeader::sSection& nx::NroAssetHeader::getRomfsInfo() const
{
	return mRomfsInfo;
}

void nx::NroAssetHeader::setRomfsInfo(const sSection& info)
{
	mRomfsInfo = info;
}

bool nx::NroAssetHeader::isEqual(const NroAssetHeader& other) const
{
	return (mIconInfo == other.mIconInfo) \
		&& (mNacpInfo == other.mNacpInfo) \
		&& (mRomfsInfo == other.mRomfsInfo);
}

void nx::NroAssetHeader::copyFrom(const NroAssetHeader& other)
{
	mIconInfo = other.mIconInfo;
	mNacpInfo = other.mNacpInfo;
	mRomfsInfo = other.mRomfsInfo;
}