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
		ETicketBody_V2(const u8* bytes, size_t len);

		bool operator==(const ETicketBody_V2& other) const;
		bool operator!=(const ETicketBody_V2& other) const;
		void operator=(const ETicketBody_V2& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		virtual void importBinary(const u8* bytes, size_t len);

		// variables
		virtual void clear();
		
		const std::string& getIssuer() const;
		void setIssuer(const std::string& issuer);

		const u8* getEncTitleKey() const;
		void setEncTitleKey(const u8* data, size_t len);

		TitleKeyEncType getTitleKeyEncType() const;
		void setTitleKeyEncType(TitleKeyEncType type);

		u16 getTicketVersion() const;
		void setTicketVersion(u16 version);

		LicenseType getLicenseType() const;
		void setLicenseType(LicenseType type);

		u8 getCommonKeyId() const;
		void setCommonKeyId(u8 id);

		bool isPreInstall() const;
		void setIsPreInstall(bool isPreInstall);

		bool isSharedTitle() const;
		void setIsSharedTitle(bool isSharedTitle);

		bool allowAllContent() const;
		void setAllowAllContent(bool allowAllContent);

		const u8* getReservedRegion() const;
		void setReservedRegion(const u8* data, size_t len);

		u64 getTicketId() const;
		void setTicketId(u64 id);

		u64 getDeviceId() const;
		void setDeviceId(u64 id);

		const u8* getRightsId() const;
		void setRightsId(const u8* id);

		u32 getAccountId() const;
		void setAccountId(u32 id);

		u32 getSectionTotalSize() const;
		void setSectionTotalSize(u32 size);

		u32 getSectionHeaderOffset() const;
		void setSectionHeaderOffset(u32 offset);

		u16 getSectionNum() const;
		void setSectionNum(u16 num);

		u16 getSectionEntrySize() const;
		void setSectionEntrySize(u16 size);

	private:
		const std::string kModuleName = "ES_ETICKET_BODY_V2";
		static const size_t kIssuerLen = 0x40;
		static const u8 kFormatVersion = 2;
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
			u8 enc_title_key_[kEncTitleKeyLen];
			u8 format_version_;
			u8 title_key_enc_type_;
			u16 ticket_version_;
			u8 license_type_;
			u8 common_key_id_;
			u8 property_mask_;
			u8 reserved_0_;
			u8 reserved_region_[kReservedRegionLen]; // explicitly reserved
			u64 ticket_id_;
			u64 device_id_;
			u8 rights_id_[kRightsIdLen];
			u32 account_id_;
			u32 sect_total_size_;
			u32 sect_header_offset_;
			u16 sect_num_;
			u16 sect_entry_size_;
		public:
			const char* issuer() const { return issuer_; }
			void set_issuer(const char issuer[kIssuerLen]) { strncpy(issuer_, issuer, kIssuerLen); }

			const u8* enc_title_key() const { return enc_title_key_; }
			void set_enc_title_key(const u8* enc_title_key, size_t len) { memset(enc_title_key_, 0, kEncTitleKeyLen); memcpy(enc_title_key_, enc_title_key, MIN(len, kEncTitleKeyLen)); }

			u8 format_version() const { return format_version_; }
			void set_format_version(u8 version) { format_version_ = version; }

			u8 title_key_enc_type() const { return title_key_enc_type_; }
			void set_title_key_enc_type(u8 type) { title_key_enc_type_ = type; }

			u16 ticket_version() const { return le_hword(ticket_version_); }
			void set_ticket_version(u16 version) { ticket_version_ = le_hword(version); }

			u8 license_type() const { return license_type_; }
			void set_license_type(u8 license_type) { license_type_ = license_type; }

			u8 common_key_id() const { return common_key_id_; }
			void set_common_key_id(u8 common_key_id) { common_key_id_ = common_key_id; }

			u8 property_mask() const { return property_mask_; }
			void set_property_mask(u8 mask) { property_mask_ = mask; }

			const u8* reserved_region() const { return reserved_region_; }
			void set_reserved_region(const u8* reserved_region, size_t len) { memcpy(reserved_region_, reserved_region, MIN(len, kReservedRegionLen)); }

			u64 ticket_id() const { return le_dword(ticket_id_); }
			void set_ticket_id(u64 ticket_id) { ticket_id_ = le_dword(ticket_id); }

			u64 device_id() const { return le_dword(device_id_); }
			void set_device_id(u64 device_id) { device_id_ = le_dword(device_id); }

			const u8* rights_id() const { return rights_id_; }
			void set_rights_id(const u8 rights_id[kRightsIdLen]) { memcpy(rights_id_, rights_id, kRightsIdLen); }

			u32 account_id() const { return le_word(account_id_); }
			void set_account_id(u32 id) { account_id_ = le_word(id); }

			u32 sect_total_size() const { return le_word(sect_total_size_); }
			void set_sect_total_size(u32 size) { sect_total_size_ = le_word(size); }

			u32 sect_header_offset() const { return le_word(sect_header_offset_); }
			void set_sect_header_offset(u32 offset) { sect_header_offset_ = le_word(offset); }

			u16 sect_num() const { return le_hword(sect_num_); }
			void set_sect_num(u16 num) { sect_num_ = num; }

			u16 sect_entry_size() const { return le_hword(sect_entry_size_); }
			void set_sect_entry_size(u16 size) { sect_entry_size_ = le_hword(size); }
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		std::string mIssuer;
		u8 mEncTitleKey[kEncTitleKeyLen];
		TitleKeyEncType mEncType; // 0 = aes-cbc, 1 = rsa2048
		u16 mTicketVersion;
		LicenseType mLicenseType;
		u8 mCommonKeyId;
		bool mPreInstall;
		bool mSharedTitle;
		bool mAllowAllContent;
		u8 mReservedRegion[kReservedRegionLen]; // explicitly reserved
		u64 mTicketId;
		u64 mDeviceId;
		u8 mRightsId[kRightsIdLen];
		u32 mAccountId;
		u32 mSectTotalSize;
		u32 mSectHeaderOffset;
		u16 mSectNum;
		u16 mSectEntrySize;

		// helpers
		bool isEqual(const ETicketBody_V2& other) const;
		void copyFrom(const ETicketBody_V2& other);
	};
}


