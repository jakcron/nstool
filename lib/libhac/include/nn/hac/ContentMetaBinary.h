#pragma once
#include <string>
#include <cstring>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nn/hac/cnmt.h>

namespace nn
{
namespace hac
{
	class ContentMetaBinary :
		public fnd::ISerialisable
	{
	public:
		struct ContentInfo
		{
			fnd::sha::sSha256Hash hash;
			byte_t nca_id[cnmt::kContentIdLen];
			size_t size;
			cnmt::ContentType type;

			void operator=(const ContentInfo& other)
			{
				hash = other.hash;
				memcpy(nca_id, other.nca_id, cnmt::kContentIdLen);
				size = other.size;
				type = other.type;
			}

			bool operator==(const ContentInfo& other) const
			{
				return (hash == other.hash) \
					&& (memcmp(nca_id, other.nca_id, cnmt::kContentIdLen) == 0) \
					&& (size == other.size) \
					&& (type == other.type);
			}

			bool operator!=(const ContentInfo& other) const
			{
				return !operator==(other);
			}
		};

		struct ContentMetaInfo
		{
			uint64_t id;
			uint32_t version;
			cnmt::ContentMetaType type;
			byte_t attributes;

			void operator=(const ContentMetaInfo& other)
			{
				id = other.id;
				version = other.version;
				type = other.type;
				attributes = other.attributes;
			}

			bool operator==(const ContentMetaInfo& other) const
			{
				return (id == other.id) \
					&& (version == other.version) \
					&& (type == other.type) \
					&& (attributes == other.attributes);
			}

			bool operator!=(const ContentMetaInfo& other) const
			{
				return !operator==(other);
			}
		};

		struct ApplicationMetaExtendedHeader
		{
			uint64_t patch_id;
			uint32_t required_system_version;

			void operator=(const ApplicationMetaExtendedHeader& other)
			{
				patch_id = other.patch_id;
				required_system_version = other.required_system_version;
			}

			bool operator==(const ApplicationMetaExtendedHeader& other) const
			{
				return (patch_id == other.patch_id) \
					&& (required_system_version == other.required_system_version);
			}

			bool operator!=(const ApplicationMetaExtendedHeader& other) const
			{
				return !operator==(other);
			}
		};

		struct PatchMetaExtendedHeader
		{
			uint64_t application_id;
			uint32_t required_system_version;

			void operator=(const PatchMetaExtendedHeader& other)
			{
				application_id = other.application_id;
				required_system_version = other.required_system_version;
			}

			bool operator==(const PatchMetaExtendedHeader& other) const
			{
				return (application_id == other.application_id) \
					&& (required_system_version == other.required_system_version);
			}

			bool operator!=(const PatchMetaExtendedHeader& other) const
			{
				return !operator==(other);
			}
		};

		struct AddOnContentMetaExtendedHeader
		{
			uint64_t application_id;
			uint32_t required_application_version;

			void operator=(const AddOnContentMetaExtendedHeader& other)
			{
				application_id = other.application_id;
				required_application_version = other.required_application_version;
			}

			bool operator==(const AddOnContentMetaExtendedHeader& other) const
			{
				return (application_id == other.application_id) \
					&& (required_application_version == other.required_application_version);
			}

			bool operator!=(const AddOnContentMetaExtendedHeader& other) const
			{
				return !operator==(other);
			}
		};

		struct DeltaMetaExtendedHeader
		{
			uint64_t application_id;

			void operator=(const DeltaMetaExtendedHeader& other)
			{
				application_id = other.application_id;
			}

			bool operator==(const DeltaMetaExtendedHeader& other) const
			{
				return (application_id == other.application_id);
			}

			bool operator!=(const DeltaMetaExtendedHeader& other) const
			{
				return !operator==(other);
			}
		};

		ContentMetaBinary();
		ContentMetaBinary(const ContentMetaBinary& other);

		void operator=(const ContentMetaBinary& other);
		bool operator==(const ContentMetaBinary& other) const;
		bool operator!=(const ContentMetaBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getTitleId() const;
		void setTitleId(uint64_t title_id);

		uint32_t getTitleVersion() const;
		void setTitleVersion(uint32_t version);

		cnmt::ContentMetaType getType() const;
		void setType(cnmt::ContentMetaType type);

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

		const fnd::List<nn::hac::ContentMetaBinary::ContentInfo>& getContentInfo() const;
		void setContentInfo(const fnd::List<nn::hac::ContentMetaBinary::ContentInfo>& info);

		const fnd::List<nn::hac::ContentMetaBinary::ContentMetaInfo>& getContentMetaInfo() const;
		void setContentMetaInfo(const fnd::List<nn::hac::ContentMetaBinary::ContentMetaInfo>& info);

		const fnd::Vec<byte_t>& getExtendedData() const;
		void setExtendedData(const fnd::Vec<byte_t>& data);

		const nn::hac::sDigest& getDigest() const;
		void setDigest(const nn::hac::sDigest& digest);


	private:
		const std::string kModuleName = "CONTENT_META_BINARY";

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

		fnd::List<nn::hac::ContentMetaBinary::ContentInfo> mContentInfo;
		fnd::List<nn::hac::ContentMetaBinary::ContentMetaInfo> mContentMetaInfo;
		fnd::Vec<byte_t> mExtendedData;
		nn::hac::sDigest mDigest;

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