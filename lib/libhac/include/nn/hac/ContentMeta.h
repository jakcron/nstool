#pragma once
#include <string>
#include <cstring>
#include <fnd/IByteModel.h>
#include <fnd/List.h>
#include <nn/hac/define/cnmt.h>
#include <nn/hac/ContentInfo.h>
#include <nn/hac/ContentMetaInfo.h>
#include <nn/hac/ApplicationMetaExtendedHeader.h>
#include <nn/hac/PatchMetaExtendedHeader.h>
#include <nn/hac/AddOnContentMetaExtendedHeader.h>
#include <nn/hac/DeltaMetaExtendedHeader.h>

namespace nn
{
namespace hac
{
	class ContentMeta :
		public fnd::IByteModel
	{
	public:
		ContentMeta();
		ContentMeta(const ContentMeta& other);

		void operator=(const ContentMeta& other);
		bool operator==(const ContentMeta& other) const;
		bool operator!=(const ContentMeta& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getTitleId() const;
		void setTitleId(uint64_t title_id);

		uint32_t getTitleVersion() const;
		void setTitleVersion(uint32_t version);

		cnmt::ContentMetaType getContentMetaType() const;
		void setContentMetaType(cnmt::ContentMetaType type);

		byte_t getAttributes() const;
		void setAttributes(byte_t attributes);

		uint32_t getRequiredDownloadSystemVersion() const;
		void setRequiredDownloadSystemVersion(uint32_t version);

		const ApplicationMetaExtendedHeader& getApplicationMetaExtendedHeader() const;
		void setApplicationMetaExtendedHeader(const ApplicationMetaExtendedHeader& exhdr);

		const PatchMetaExtendedHeader& getPatchMetaExtendedHeader() const;
		void setPatchMetaExtendedHeader(const PatchMetaExtendedHeader& exhdr);

		const AddOnContentMetaExtendedHeader& getAddOnContentMetaExtendedHeader() const;
		void setAddOnContentMetaExtendedHeader(const AddOnContentMetaExtendedHeader& exhdr);

		const DeltaMetaExtendedHeader& getDeltaMetaExtendedHeader() const;
		void setDeltaMetaExtendedHeader(const DeltaMetaExtendedHeader& exhdr);

		const fnd::List<ContentInfo>& getContentInfo() const;
		void setContentInfo(const fnd::List<ContentInfo>& info);

		const fnd::List<ContentMetaInfo>& getContentMetaInfo() const;
		void setContentMetaInfo(const fnd::List<ContentMetaInfo>& info);

		const fnd::Vec<byte_t>& getExtendedData() const;
		void setExtendedData(const fnd::Vec<byte_t>& data);

		const cnmt::sDigest& getDigest() const;
		void setDigest(const cnmt::sDigest& digest);


	private:
		const std::string kModuleName = "CONTENT_META";

		// binary blob
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mTitleId;
		uint32_t mTitleVersion;
		cnmt::ContentMetaType mType;
		byte_t mAttributes;
		uint32_t mRequiredDownloadSystemVersion;
		fnd::Vec<byte_t> mExtendedHeader;

		ApplicationMetaExtendedHeader mApplicationMetaExtendedHeader;
		PatchMetaExtendedHeader mPatchMetaExtendedHeader;
		AddOnContentMetaExtendedHeader mAddOnContentMetaExtendedHeader;
		DeltaMetaExtendedHeader mDeltaMetaExtendedHeader;

		fnd::List<ContentInfo> mContentInfo;
		fnd::List<ContentMetaInfo> mContentMetaInfo;
		fnd::Vec<byte_t> mExtendedData;
		cnmt::sDigest mDigest;

		inline size_t getExtendedHeaderOffset() const { return sizeof(sContentMetaHeader); }
		inline size_t getContentInfoOffset(size_t exhdrSize) const { return getExtendedHeaderOffset() + exhdrSize; }
		inline size_t getContentMetaInfoOffset(size_t exhdrSize, size_t contentInfoNum) const { return getContentInfoOffset(exhdrSize) + contentInfoNum * sizeof(sContentInfo); }
		inline size_t getExtendedDataOffset(size_t exhdrSize, size_t contentInfoNum, size_t contentMetaNum) const { return getContentMetaInfoOffset(exhdrSize, contentInfoNum) + contentMetaNum * sizeof(sContentMetaInfo); }
		inline size_t getDigestOffset(size_t exhdrSize, size_t contentInfoNum, size_t contentMetaNum, size_t exdataSize) const { return getExtendedDataOffset(exhdrSize, contentInfoNum, contentMetaNum) + exdataSize; }
		inline size_t getTotalSize(size_t exhdrSize, size_t contentInfoNum, size_t contentMetaNum, size_t exdataSize) const { return getDigestOffset(exhdrSize, contentInfoNum, contentMetaNum, exdataSize) + cnmt::kDigestLen; }

		bool validateExtendedHeaderSize(cnmt::ContentMetaType type, size_t exhdrSize) const;
		size_t getExtendedDataSize(cnmt::ContentMetaType type, const byte_t* data) const;
		void validateBinary(const byte_t* bytes, size_t len) const;
	};
}
}