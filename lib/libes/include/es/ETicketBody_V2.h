#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <crypto/rsa.h>

namespace es
{
	class ETicketBody_V2 :
		public fnd::ISerialiseableBinary
	{
	public:
		enum TitleKeyEncType
		{
			AES128_CBC,
			RSA2048
		};

		enum LicenseType
		{
			ES_LICENSE_PERMANENT = 0,
			ES_LICENSE_DEMO = 1,
			ES_LICENSE_TRIAL = 2,
			ES_LICENSE_RENTAL = 3,
			ES_LICENSE_SUBSCRIPTION = 4,
			ES_LICENSE_SERVICE = 5,
		};

		ETicketBody_V2();
		ETicketBody_V2(const ETicketBody_V2& other);
		ETicketBody_V2(const byte_t* bytes, size_t len);

		bool operator==(const ETicketBody_V2& other) const;
		bool operator!=(const ETicketBody_V2& other) const;
		void operator=(const ETicketBody_V2& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		virtual void importBinary(const byte_t* bytes, size_t len);

		// variables
		virtual void clear();
		
		const std::string& getIssuer() const;
		void setIssuer(const std::string& issuer);

		const byte_t* getEncTitleKey() const;
		void setEncTitleKey(const byte_t* data, size_t len);

		TitleKeyEncType getTitleKeyEncType() const;
		void setTitleKeyEncType(TitleKeyEncType type);

		uint16_t getTicketVersion() const;
		void setTicketVersion(uint16_t version);

		LicenseType getLicenseType() const;
		void setLicenseType(LicenseType type);

		byte_t getCommonKeyId() const;
		void setCommonKeyId(byte_t id);

		bool isPreInstall() const;
		void setIsPreInstall(bool isPreInstall);

		bool isSharedTitle() const;
		void setIsSharedTitle(bool isSharedTitle);

		bool allowAllContent() const;
		void setAllowAllContent(bool allowAllContent);

		const byte_t* getReservedRegion() const;
		void setReservedRegion(const byte_t* data, size_t len);

		uint64_t getTicketId() const;
		void setTicketId(uint64_t id);

		uint64_t getDeviceId() const;
		void setDeviceId(uint64_t id);

		const byte_t* getRightsId() const;
		void setRightsId(const byte_t* id);

		uint32_t getAccountId() const;
		void setAccountId(uint32_t id);

		uint32_t getSectionTotalSize() const;
		void setSectionTotalSize(uint32_t size);

		uint32_t getSectionHeaderOffset() const;
		void setSectionHeaderOffset(uint32_t offset);

		uint16_t getSectionNum() const;
		void setSectionNum(uint16_t num);

		uint16_t getSectionEntrySize() const;
		void setSectionEntrySize(uint16_t size);

	private:
		const std::string kModuleName = "ES_ETICKET_BODY_V2";
		static const size_t kIssuerLen = 0x40;
		static const byte_t kFormatVersion = 2;
		static const size_t kEncTitleKeyLen = crypto::rsa::kRsa2048Size;
		static const size_t kReservedRegionLen = 8;
		static const size_t kRightsIdLen = 16;

		enum PropertyMaskFlags
		{
			FLAG_PRE_INSTALL,
			FLAG_SHARED_TITLE,
			FLAG_ALLOW_ALL_CONTENT
		};

#pragma pack (push, 1)
		struct sTicketBody_v2
		{
			char issuer[kIssuerLen];
			byte_t enc_title_key[kEncTitleKeyLen];
			byte_t format_version;
			byte_t title_key_enc_type;
			le_uint16_t ticket_version;
			byte_t license_type;
			byte_t common_key_id;
			byte_t property_mask;
			byte_t reserved_0;
			byte_t reserved_region[kReservedRegionLen]; // explicitly reserved
			le_uint64_t ticket_id;
			le_uint64_t device_id;
			byte_t rights_id[kRightsIdLen];
			le_uint32_t account_id;
			le_uint32_t sect_total_size;
			le_uint32_t sect_header_offset;
			le_uint16_t sect_num;
			le_uint16_t sect_entry_size;
		
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		std::string mIssuer;
		byte_t mEncTitleKey[kEncTitleKeyLen];
		TitleKeyEncType mEncType; // 0 = aes-cbc, 1 = rsa2048
		uint16_t mTicketVersion;
		LicenseType mLicenseType;
		byte_t mCommonKeyId;
		bool mPreInstall;
		bool mSharedTitle;
		bool mAllowAllContent;
		byte_t mReservedRegion[kReservedRegionLen]; // explicitly reserved
		uint64_t mTicketId;
		uint64_t mDeviceId;
		byte_t mRightsId[kRightsIdLen];
		uint32_t mAccountId;
		uint32_t mSectTotalSize;
		uint32_t mSectHeaderOffset;
		uint16_t mSectNum;
		uint16_t mSectEntrySize;

		// helpers
		bool isEqual(const ETicketBody_V2& other) const;
		void copyFrom(const ETicketBody_V2& other);
	};
}


