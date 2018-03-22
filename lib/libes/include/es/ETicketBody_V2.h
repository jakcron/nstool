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
		private:
			char issuer_[kIssuerLen];
			byte_t enc_title_key_[kEncTitleKeyLen];
			byte_t format_version_;
			byte_t title_key_enc_type_;
			uint16_t ticket_version_;
			byte_t license_type_;
			byte_t common_key_id_;
			byte_t property_mask_;
			byte_t reserved_0_;
			byte_t reserved_region_[kReservedRegionLen]; // explicitly reserved
			uint64_t ticket_id_;
			uint64_t device_id_;
			byte_t rights_id_[kRightsIdLen];
			uint32_t account_id_;
			uint32_t sect_total_size_;
			uint32_t sect_header_offset_;
			uint16_t sect_num_;
			uint16_t sect_entry_size_;
		public:
			const char* issuer() const { return issuer_; }
			void set_issuer(const char issuer[kIssuerLen]) { strncpy(issuer_, issuer, kIssuerLen); }

			const byte_t* enc_title_key() const { return enc_title_key_; }
			void set_enc_title_key(const byte_t* enc_title_key, size_t len) { memset(enc_title_key_, 0, kEncTitleKeyLen); memcpy(enc_title_key_, enc_title_key, MIN(len, kEncTitleKeyLen)); }

			byte_t format_version() const { return format_version_; }
			void set_format_version(byte_t version) { format_version_ = version; }

			byte_t title_key_enc_type() const { return title_key_enc_type_; }
			void set_title_key_enc_type(byte_t type) { title_key_enc_type_ = type; }

			uint16_t ticket_version() const { return le_hword(ticket_version_); }
			void set_ticket_version(uint16_t version) { ticket_version_ = le_hword(version); }

			byte_t license_type() const { return license_type_; }
			void set_license_type(byte_t license_type) { license_type_ = license_type; }

			byte_t common_key_id() const { return common_key_id_; }
			void set_common_key_id(byte_t common_key_id) { common_key_id_ = common_key_id; }

			byte_t property_mask() const { return property_mask_; }
			void set_property_mask(byte_t mask) { property_mask_ = mask; }

			const byte_t* reserved_region() const { return reserved_region_; }
			void set_reserved_region(const byte_t* reserved_region, size_t len) { memcpy(reserved_region_, reserved_region, MIN(len, kReservedRegionLen)); }

			uint64_t ticket_id() const { return le_dword(ticket_id_); }
			void set_ticket_id(uint64_t ticket_id) { ticket_id_ = le_dword(ticket_id); }

			uint64_t device_id() const { return le_dword(device_id_); }
			void set_device_id(uint64_t device_id) { device_id_ = le_dword(device_id); }

			const byte_t* rights_id() const { return rights_id_; }
			void set_rights_id(const byte_t rights_id[kRightsIdLen]) { memcpy(rights_id_, rights_id, kRightsIdLen); }

			uint32_t account_id() const { return le_word(account_id_); }
			void set_account_id(uint32_t id) { account_id_ = le_word(id); }

			uint32_t sect_total_size() const { return le_word(sect_total_size_); }
			void set_sect_total_size(uint32_t size) { sect_total_size_ = le_word(size); }

			uint32_t sect_header_offset() const { return le_word(sect_header_offset_); }
			void set_sect_header_offset(uint32_t offset) { sect_header_offset_ = le_word(offset); }

			uint16_t sect_num() const { return le_hword(sect_num_); }
			void set_sect_num(uint16_t num) { sect_num_ = num; }

			uint16_t sect_entry_size() const { return le_hword(sect_entry_size_); }
			void set_sect_entry_size(uint16_t size) { sect_entry_size_ = le_hword(size); }
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


