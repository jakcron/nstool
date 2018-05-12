#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <nx/cnmt.h>


namespace nx
{
	class ContentMetaBinary :
		public fnd::ISerialiseableBinary
	{
	public:
		struct ContentInfo
		{
			crypto::sha::sSha256Hash hash;
			byte_t nca_id[cnmt::kContentIdLen];
			size_t size;
			cnmt::ContentType type;

			ContentInfo& operator=(const ContentInfo& other)
			{
				hash = other.hash;
				memcpy(nca_id, other.nca_id, cnmt::kContentIdLen);
				size = other.size;
				type = other.type;
				return *this;
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

			ContentMetaInfo& operator=(const ContentMetaInfo& other)
			{
				id = other.id;
				version = other.version;
				type = other.type;
				attributes = other.attributes;
				return *this;
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

			ApplicationMetaExtendedHeader& operator=(const ApplicationMetaExtendedHeader& other)
			{
				patch_id = other.patch_id;
				required_system_version = other.required_system_version;
				return *this;
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

			PatchMetaExtendedHeader& operator=(const PatchMetaExtendedHeader& other)
			{
				application_id = other.application_id;
				required_system_version = other.required_system_version;
				return *this;
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
			uint32_t required_system_version;

			AddOnContentMetaExtendedHeader& operator=(const AddOnContentMetaExtendedHeader& other)
			{
				application_id = other.application_id;
				required_system_version = other.required_system_version;
				return *this;
			}

			bool operator==(const AddOnContentMetaExtendedHeader& other) const
			{
				return (application_id == other.application_id) \
					&& (required_system_version == other.required_system_version);
			}

			bool operator!=(const AddOnContentMetaExtendedHeader& other) const
			{
				return !operator==(other);
			}
		};

		struct DeltaMetaExtendedHeader
		{
			uint64_t application_id;

			DeltaMetaExtendedHeader& operator=(const DeltaMetaExtendedHeader& other)
			{
				application_id = other.application_id;
				return *this;
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
		ContentMetaBinary(const byte_t* bytes, size_t len);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

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

		const fnd::List<nx::ContentMetaBinary::ContentInfo>& getContentInfo() const;
		void setContentInfo(const fnd::List<nx::ContentMetaBinary::ContentInfo>& info);

		const fnd::List<nx::ContentMetaBinary::ContentMetaInfo>& getContentMetaInfo() const;
		void setContentMetaInfo(const fnd::List<nx::ContentMetaBinary::ContentMetaInfo>& info);

		const fnd::MemoryBlob& getExtendedData() const;
		void setExtendedData(const fnd::MemoryBlob& data);

		const nx::sDigest& getDigest() const;
		void setDigest(const nx::sDigest& digest);


	private:
		const std::string kModuleName = "CONTENT_META_BINARY";

		// binary blob
		fnd::MemoryBlob mBinaryBlob;

		// variables
		uint64_t mTitleId;
		uint32_t mTitleVersion;
		cnmt::ContentMetaType mType;
		byte_t mAttributes;
		uint32_t mRequiredDownloadSystemVersion;
		fnd::MemoryBlob mExtendedHeader;

		ApplicationMetaExtendedHeader mApplicationMetaExtendedHeader;
		PatchMetaExtendedHeader mPatchMetaExtendedHeader;
		AddOnContentMetaExtendedHeader mAddOnContentMetaExtendedHeader;
		DeltaMetaExtendedHeader mDeltaMetaExtendedHeader;

		fnd::List<nx::ContentMetaBinary::ContentInfo> mContentInfo;
		fnd::List<nx::ContentMetaBinary::ContentMetaInfo> mContentMetaInfo;
		fnd::MemoryBlob mExtendedData;
		nx::sDigest mDigest;

		inline size_t getExtendedHeaderOffset() const { return sizeof(sContentMetaHeader); }
		inline size_t getContentInfoOffset(size_t exhdrSize) const { return getExtendedHeaderOffset() + exhdrSize; }
		inline size_t getContentMetaInfoOffset(size_t exhdrSize, size_t contentInfoNum) const { return getContentInfoOffset(exhdrSize) + contentInfoNum * sizeof(sContentInfo); }
		inline size_t getExtendedDataOffset(size_t exhdrSize, size_t contentInfoNum, size_t contentMetaNum) const { return getContentMetaInfoOffset(exhdrSize, contentInfoNum) + contentMetaNum * sizeof(sContentMetaInfo); }
		inline size_t getDigestOffset(size_t exhdrSize, size_t contentInfoNum, size_t contentMetaNum, size_t exdataSize) const { return getExtendedDataOffset(exhdrSize, contentInfoNum, contentMetaNum) + exdataSize; }
		inline size_t getTotalSize(size_t exhdrSize, size_t contentInfoNum, size_t contentMetaNum, size_t exdataSize) const { return getDigestOffset(exhdrSize, contentInfoNum, contentMetaNum, exdataSize) + cnmt::kDigestLen; }

		bool validateExtendedHeaderSize(cnmt::ContentMetaType type, size_t exhdrSize) const;
		size_t getExtendedDataSize(cnmt::ContentMetaType type, const byte_t* data) const;
		void validateBinary(const byte_t* bytes, size_t len) const;

		bool isEqual(const ContentMetaBinary& other) const;
		void copyFrom(const ContentMetaBinary& other);
	};
}