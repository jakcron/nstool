#include <nx/AssetHeader.h>

nx::AssetHeader::AssetHeader()
{
	clear();
}

nx::AssetHeader::AssetHeader(const AssetHeader& other)
{
	copyFrom(other);
}

nx::AssetHeader::AssetHeader(const byte_t* bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::AssetHeader::operator==(const AssetHeader& other) const
{
	return isEqual(other);
}

bool nx::AssetHeader::operator!=(const AssetHeader& other) const
{
	return !(*this == other);
}

void nx::AssetHeader::operator=(const AssetHeader& other)
{
	copyFrom(other);
}

const byte_t* nx::AssetHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::AssetHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::AssetHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sAssetHeader));
	nx::sAssetHeader* hdr = (nx::sAssetHeader*)mBinaryBlob.getBytes();

	// set header identifers
	hdr->signature = aset::kAssetSig;
	hdr->format_version = aset::kDefaultAssetFormatVersion;

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

void nx::AssetHeader::importBinary(const byte_t* bytes, size_t len)
{
	// check input data size
	if (len < sizeof(sAssetHeader))
	{
		throw fnd::Exception(kModuleName, "ASET header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mBinaryBlob.alloc(sizeof(sAssetHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	// get sAssetHeader ptr
	const nx::sAssetHeader* hdr = (const nx::sAssetHeader*)mBinaryBlob.getBytes();
	
	// check NRO signature
	if (hdr->signature.get() != aset::kAssetSig)
	{
		throw fnd::Exception(kModuleName, "ASET header corrupt (unrecognised header signature)");
	}

	// check NRO format version
	if (hdr->format_version.get() != aset::kDefaultAssetFormatVersion)
	{
		throw fnd::Exception(kModuleName, "ASET header corrupt (unsupported format version)");
	}

	mIconInfo.offset = hdr->icon.offset.get();
	mIconInfo.size = hdr->icon.size.get();
	mNacpInfo.offset = hdr->nacp.offset.get();
	mNacpInfo.size = hdr->nacp.size.get();
	mRomfsInfo.offset = hdr->romfs.offset.get();
	mRomfsInfo.size = hdr->romfs.size.get();
}

void nx::AssetHeader::clear()
{
	mBinaryBlob.clear();
	memset(&mIconInfo, 0, sizeof(mIconInfo));
	memset(&mNacpInfo, 0, sizeof(mNacpInfo));
	memset(&mRomfsInfo, 0, sizeof(mRomfsInfo));
}

const nx::AssetHeader::sSection& nx::AssetHeader::getIconInfo() const
{
	return mIconInfo;
}

void nx::AssetHeader::setIconInfo(const nx::AssetHeader::sSection& info)
{
	mIconInfo = info;
}

const nx::AssetHeader::sSection& nx::AssetHeader::getNacpInfo() const
{
	return mNacpInfo;
}

void nx::AssetHeader::setNacpInfo(const sSection& info)
{
	mNacpInfo = info;
}

const nx::AssetHeader::sSection& nx::AssetHeader::getRomfsInfo() const
{
	return mRomfsInfo;
}

void nx::AssetHeader::setRomfsInfo(const sSection& info)
{
	mRomfsInfo = info;
}

bool nx::AssetHeader::isEqual(const AssetHeader& other) const
{
	return (mIconInfo == other.mIconInfo) \
		&& (mNacpInfo == other.mNacpInfo) \
		&& (mRomfsInfo == other.mRomfsInfo);
}

void nx::AssetHeader::copyFrom(const AssetHeader& other)
{
	mIconInfo = other.mIconInfo;
	mNacpInfo = other.mNacpInfo;
	mRomfsInfo = other.mRomfsInfo;
}