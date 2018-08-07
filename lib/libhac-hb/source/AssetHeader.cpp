#include <cstring>
#include <nn/hac/AssetHeader.h>

nn::hac::AssetHeader::AssetHeader()
{
	clear();
}

nn::hac::AssetHeader::AssetHeader(const AssetHeader& other)
{
	*this = other;
}

void nn::hac::AssetHeader::operator=(const AssetHeader& other)
{
	mRawBinary = other.mRawBinary;
	mIconInfo = other.mIconInfo;
	mNacpInfo = other.mNacpInfo;
	mRomfsInfo = other.mRomfsInfo;
}

bool nn::hac::AssetHeader::operator==(const AssetHeader& other) const
{
	return (mIconInfo == other.mIconInfo) \
		&& (mNacpInfo == other.mNacpInfo) \
		&& (mRomfsInfo == other.mRomfsInfo);
}

bool nn::hac::AssetHeader::operator!=(const AssetHeader& other) const
{
	return !(*this == other);
}

void nn::hac::AssetHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sAssetHeader));
	nn::hac::sAssetHeader* hdr = (nn::hac::sAssetHeader*)mRawBinary.data();

	// set header identifers
	hdr->st_magic = aset::kAssetStructMagic;
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

void nn::hac::AssetHeader::fromBytes(const byte_t* bytes, size_t len)
{
	// check input data size
	if (len < sizeof(sAssetHeader))
	{
		throw fnd::Exception(kModuleName, "ASET header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sAssetHeader));
	memcpy(mRawBinary.data(), bytes, mRawBinary.size());

	// get sAssetHeader ptr
	const nn::hac::sAssetHeader* hdr = (const nn::hac::sAssetHeader*)mRawBinary.data();
	
	// check NRO signature
	if (hdr->st_magic.get() != aset::kAssetStructMagic)
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

const fnd::Vec<byte_t>& nn::hac::AssetHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::AssetHeader::clear()
{
	mRawBinary.clear();
	memset(&mIconInfo, 0, sizeof(mIconInfo));
	memset(&mNacpInfo, 0, sizeof(mNacpInfo));
	memset(&mRomfsInfo, 0, sizeof(mRomfsInfo));
}

const nn::hac::AssetHeader::sSection& nn::hac::AssetHeader::getIconInfo() const
{
	return mIconInfo;
}

void nn::hac::AssetHeader::setIconInfo(const nn::hac::AssetHeader::sSection& info)
{
	mIconInfo = info;
}

const nn::hac::AssetHeader::sSection& nn::hac::AssetHeader::getNacpInfo() const
{
	return mNacpInfo;
}

void nn::hac::AssetHeader::setNacpInfo(const sSection& info)
{
	mNacpInfo = info;
}

const nn::hac::AssetHeader::sSection& nn::hac::AssetHeader::getRomfsInfo() const
{
	return mRomfsInfo;
}

void nn::hac::AssetHeader::setRomfsInfo(const sSection& info)
{
	mRomfsInfo = info;
}