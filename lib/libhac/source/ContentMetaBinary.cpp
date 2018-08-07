#include <nx/ContentMetaBinary.h>

nx::ContentMetaBinary::ContentMetaBinary()
{
	clear();
}

nx::ContentMetaBinary::ContentMetaBinary(const ContentMetaBinary & other)
{
	*this = other;
}

void nx::ContentMetaBinary::operator=(const ContentMetaBinary& other)
{
	if (other.getBytes().size() > 0)
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		mTitleId = other.mTitleId;
		mTitleVersion = other.mTitleVersion;
		mType = other.mType;
		mAttributes = other.mAttributes;
		mRequiredDownloadSystemVersion = other.mRequiredDownloadSystemVersion;
		mExtendedHeader = other.mExtendedHeader;
		mApplicationMetaExtendedHeader = other.mApplicationMetaExtendedHeader;
		mPatchMetaExtendedHeader = other.mPatchMetaExtendedHeader;
		mAddOnContentMetaExtendedHeader = other.mAddOnContentMetaExtendedHeader;
		mDeltaMetaExtendedHeader = other.mDeltaMetaExtendedHeader;
		mContentInfo = other.mContentInfo;
		mContentMetaInfo = other.mContentMetaInfo;
		mExtendedData = other.mExtendedData;
		memcpy(mDigest.data, other.mDigest.data, cnmt::kDigestLen);
	}
}

bool nx::ContentMetaBinary::operator==(const ContentMetaBinary& other) const
{
	return (mTitleId == other.mTitleId) \
		&& (mTitleVersion == other.mTitleVersion) \
		&& (mType == other.mType) \
		&& (mAttributes == other.mAttributes) \
		&& (mRequiredDownloadSystemVersion == other.mRequiredDownloadSystemVersion) \
		&& (mExtendedHeader == other.mExtendedHeader) \
		&& (mApplicationMetaExtendedHeader == other.mApplicationMetaExtendedHeader) \
		&& (mPatchMetaExtendedHeader == other.mPatchMetaExtendedHeader) \
		&& (mAddOnContentMetaExtendedHeader == other.mAddOnContentMetaExtendedHeader) \
		&& (mDeltaMetaExtendedHeader == other.mDeltaMetaExtendedHeader) \
		&& (mContentInfo == other.mContentInfo) \
		&& (mContentMetaInfo == other.mContentMetaInfo) \
		&& (mExtendedData == other.mExtendedData) \
		&& (memcmp(mDigest.data, other.mDigest.data, cnmt::kDigestLen) == 0);
}

bool nx::ContentMetaBinary::operator!=(const ContentMetaBinary& other) const
{
	return !(*this == other);
}

void nx::ContentMetaBinary::toBytes()
{
	throw fnd::Exception(kModuleName, "exportBinary() not implemented");
}

void nx::ContentMetaBinary::fromBytes(const byte_t* data, size_t len)
{
	// clear member variables
	clear();

	// validate layout
	validateBinary(data, len);

	// get pointer to header structure
	const sContentMetaHeader* hdr = (const sContentMetaHeader*)data;

	mTitleId = hdr->id.get();
	mTitleVersion = hdr->version.get();
	mType = (cnmt::ContentMetaType)hdr->type;
	mAttributes = hdr->attributes;
	mRequiredDownloadSystemVersion = hdr->required_download_system_version.get();
	size_t exdata_size = 0;

	// save exheader
	if (hdr->exhdr_size.get() > 0)
	{
		mExtendedHeader.alloc(hdr->exhdr_size.get());
		memcpy(mExtendedHeader.data(), data + getExtendedHeaderOffset(), hdr->exhdr_size.get());

		switch (mType)
		{
			case (cnmt::METATYPE_APPLICATION):
				mApplicationMetaExtendedHeader.patch_id = ((sApplicationMetaExtendedHeader*)mExtendedHeader.data())->patch_id.get();
				mApplicationMetaExtendedHeader.required_system_version = ((sApplicationMetaExtendedHeader*)mExtendedHeader.data())->required_system_version.get();
				break;
			case (cnmt::METATYPE_PATCH):
				mPatchMetaExtendedHeader.application_id = ((sPatchMetaExtendedHeader*)mExtendedHeader.data())->application_id.get();
				mPatchMetaExtendedHeader.required_system_version = ((sPatchMetaExtendedHeader*)mExtendedHeader.data())->required_system_version.get();
				break;
			case (cnmt::METATYPE_ADD_ON_CONTENT):
				mAddOnContentMetaExtendedHeader.application_id = ((sAddOnContentMetaExtendedHeader*)mExtendedHeader.data())->application_id.get();
				mAddOnContentMetaExtendedHeader.required_system_version = ((sAddOnContentMetaExtendedHeader*)mExtendedHeader.data())->required_system_version.get();
				break;
			case (cnmt::METATYPE_DELTA):
				mDeltaMetaExtendedHeader.application_id = ((sDeltaMetaExtendedHeader*)mExtendedHeader.data())->application_id.get();
				break;
			default:
				break;
		}

		exdata_size = getExtendedDataSize(mType, mExtendedHeader.data());
	}

	// save content info
	if (hdr->content_count.get() > 0)
	{
		const sContentInfo* info = (const sContentInfo*)(data + getContentInfoOffset(hdr->exhdr_size.get()));
		ContentInfo cinfo;
		for (size_t i = 0; i < hdr->content_count.get(); i++)
		{
			cinfo.hash = info[i].content_hash;
			memcpy(cinfo.nca_id, info[i].content_id, cnmt::kContentIdLen);
			cinfo.size = (uint64_t)(info[i].size_lower.get()) | (uint64_t)(info[i].size_higher.get()) << 32;
			cinfo.type = (cnmt::ContentType)info[i].content_type;
			mContentInfo.addElement(cinfo);
		}
	}

	// save content meta info
	if (hdr->content_meta_count.get() > 0)
	{
		const sContentMetaInfo* info = (const sContentMetaInfo*)(data + getContentMetaInfoOffset(hdr->exhdr_size.get(), hdr->content_count.get()));
		ContentMetaInfo cmeta;
		for (size_t i = 0; i < hdr->content_meta_count.get(); i++)
		{	
			cmeta.id = info[i].id.get();
			cmeta.version = info[i].version.get();
			cmeta.type = (cnmt::ContentMetaType)info[i].type;
			cmeta.attributes = info[i].attributes;
			mContentMetaInfo.addElement(cmeta);
		}
	}

	// save exdata
	if (exdata_size > 0)
	{
		mExtendedData.alloc(exdata_size);
		memcpy(mExtendedData.data(), data + getExtendedDataOffset(hdr->exhdr_size.get(), hdr->content_count.get(), hdr->content_meta_count.get()), exdata_size);
	}

	// save digest
	memcpy(mDigest.data, data + getDigestOffset(hdr->exhdr_size.get(), hdr->content_count.get(), hdr->content_meta_count.get(), exdata_size), cnmt::kDigestLen);
}

const fnd::Vec<byte_t>& nx::ContentMetaBinary::getBytes() const
{
	return mRawBinary;
}

void nx::ContentMetaBinary::clear()
{
	mRawBinary.clear();
	mTitleId = 0;
	mTitleVersion = 0;
	mType = cnmt::METATYPE_SYSTEM_PROGRAM;
	mAttributes = 0;
	mRequiredDownloadSystemVersion = 0;
	mExtendedHeader.clear();
	memset(&mApplicationMetaExtendedHeader, 0, sizeof(mApplicationMetaExtendedHeader));
	memset(&mPatchMetaExtendedHeader, 0, sizeof(mPatchMetaExtendedHeader));
	memset(&mAddOnContentMetaExtendedHeader, 0, sizeof(mAddOnContentMetaExtendedHeader));
	memset(&mDeltaMetaExtendedHeader, 0, sizeof(mDeltaMetaExtendedHeader));
	mContentInfo.clear();
	mContentMetaInfo.clear();
	mExtendedData.clear();
	memset(mDigest.data, 0, cnmt::kDigestLen);
}

uint64_t nx::ContentMetaBinary::getTitleId() const
{
	return mTitleId;
}

void nx::ContentMetaBinary::setTitleId(uint64_t title_id)
{
	mTitleId = title_id;
}

uint32_t nx::ContentMetaBinary::getTitleVersion() const
{
	return mTitleVersion;
}

void nx::ContentMetaBinary::setTitleVersion(uint32_t version)
{
	mTitleVersion = version;
}

nx::cnmt::ContentMetaType nx::ContentMetaBinary::getType() const
{
	return mType;
}

void nx::ContentMetaBinary::setType(cnmt::ContentMetaType type)
{
	mType = type;
}

byte_t nx::ContentMetaBinary::getAttributes() const
{
	return mAttributes;
}

void nx::ContentMetaBinary::setAttributes(byte_t attributes)
{
	mAttributes = attributes;
}

uint32_t nx::ContentMetaBinary::getRequiredDownloadSystemVersion() const
{
	return mRequiredDownloadSystemVersion;
}

void nx::ContentMetaBinary::setRequiredDownloadSystemVersion(uint32_t version)
{
	mRequiredDownloadSystemVersion = version;
}

const nx::ContentMetaBinary::ApplicationMetaExtendedHeader& nx::ContentMetaBinary::getApplicationMetaExtendedHeader() const
{
	return mApplicationMetaExtendedHeader;
}

void nx::ContentMetaBinary::setApplicationMetaExtendedHeader(const ApplicationMetaExtendedHeader& exhdr)
{
	mApplicationMetaExtendedHeader = exhdr;
}

const nx::ContentMetaBinary::PatchMetaExtendedHeader& nx::ContentMetaBinary::getPatchMetaExtendedHeader() const
{
	return mPatchMetaExtendedHeader;
}

void nx::ContentMetaBinary::setPatchMetaExtendedHeader(const PatchMetaExtendedHeader& exhdr)
{
	mPatchMetaExtendedHeader = exhdr;
}

const nx::ContentMetaBinary::AddOnContentMetaExtendedHeader& nx::ContentMetaBinary::getAddOnContentMetaExtendedHeader() const
{
	return mAddOnContentMetaExtendedHeader;
}

void nx::ContentMetaBinary::setAddOnContentMetaExtendedHeader(const AddOnContentMetaExtendedHeader& exhdr)
{
	mAddOnContentMetaExtendedHeader = exhdr;
}

const nx::ContentMetaBinary::DeltaMetaExtendedHeader& nx::ContentMetaBinary::getDeltaMetaExtendedHeader() const
{
	return mDeltaMetaExtendedHeader;
}

void nx::ContentMetaBinary::setDeltaMetaExtendedHeader(const DeltaMetaExtendedHeader& exhdr)
{
	mDeltaMetaExtendedHeader = exhdr;
}

const fnd::List<nx::ContentMetaBinary::ContentInfo>& nx::ContentMetaBinary::getContentInfo() const
{
	return mContentInfo;
}

void nx::ContentMetaBinary::setContentInfo(const fnd::List<nx::ContentMetaBinary::ContentInfo>& info)
{
	mContentInfo = info;
}

const fnd::List<nx::ContentMetaBinary::ContentMetaInfo>& nx::ContentMetaBinary::getContentMetaInfo() const
{
	return mContentMetaInfo;
}

void nx::ContentMetaBinary::setContentMetaInfo(const fnd::List<nx::ContentMetaBinary::ContentMetaInfo>& info)
{
	mContentMetaInfo = info;
}

const fnd::Vec<byte_t> & nx::ContentMetaBinary::getExtendedData() const
{
	return mExtendedData;
}

void nx::ContentMetaBinary::setExtendedData(const fnd::Vec<byte_t> & data)
{
	mExtendedData = data;
}

const nx::sDigest & nx::ContentMetaBinary::getDigest() const
{
	return mDigest;
}

void nx::ContentMetaBinary::setDigest(const nx::sDigest & digest)
{

	memcpy(mDigest.data, digest.data, cnmt::kDigestLen);
}

bool nx::ContentMetaBinary::validateExtendedHeaderSize(cnmt::ContentMetaType type, size_t exhdrSize) const
{
	bool validSize = false;

	switch (type)
	{
		case (cnmt::METATYPE_APPLICATION):
			validSize = (exhdrSize == sizeof(sApplicationMetaExtendedHeader));
			break;
		case (cnmt::METATYPE_PATCH):
			validSize = (exhdrSize == sizeof(sPatchMetaExtendedHeader));
			break;
		case (cnmt::METATYPE_ADD_ON_CONTENT):
			validSize = (exhdrSize == sizeof(sAddOnContentMetaExtendedHeader));
			break;
		case (cnmt::METATYPE_DELTA):
			validSize = (exhdrSize == sizeof(sDeltaMetaExtendedHeader));
			break;
		default:
			validSize = (exhdrSize == 0);
	}

	return validSize;
}

size_t nx::ContentMetaBinary::getExtendedDataSize(cnmt::ContentMetaType type, const byte_t * data) const
{
	size_t exdata_len = 0;
	if (type == cnmt::METATYPE_PATCH)
	{
		const sPatchMetaExtendedHeader* exhdr = (const sPatchMetaExtendedHeader*)(data);
		exdata_len = exhdr->extended_data_size.get();
	}
	else if (type == cnmt::METATYPE_DELTA)
	{
		const sDeltaMetaExtendedHeader* exhdr = (const sDeltaMetaExtendedHeader*)(data);
		exdata_len = exhdr->extended_data_size.get();
	}
	return exdata_len;
}

void nx::ContentMetaBinary::validateBinary(const byte_t * data, size_t len) const
{
	// check if it is large enough to read the header
	if (len < sizeof(sContentMetaHeader))
	{
		throw fnd::Exception(kModuleName, "Binary too small");
	}

	// get pointer to header structure
	const sContentMetaHeader* hdr = (const sContentMetaHeader*)data;

	// validate extended header size
	if (validateExtendedHeaderSize((cnmt::ContentMetaType)hdr->type, hdr->exhdr_size.get()) == false)
	{
		throw fnd::Exception(kModuleName, "Invalid extended header size");
	}

	// check binary size again for new minimum size
	if (len < getTotalSize(hdr->exhdr_size.get(), hdr->content_count.get(), hdr->content_meta_count.get(), 0))
	{
		throw fnd::Exception(kModuleName, "Binary too small");
	}

	// check binary size again with extended data size
	if (len < getTotalSize(hdr->exhdr_size.get(), hdr->content_count.get(), hdr->content_meta_count.get(), getExtendedDataSize((cnmt::ContentMetaType)hdr->type, data + getExtendedHeaderOffset())))
	{
		throw fnd::Exception(kModuleName, "Binary too small");
	}
}