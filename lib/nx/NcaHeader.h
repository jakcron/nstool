#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/memory_blob.h>
#include <fnd/List.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <nx/ISerialiseableBinary.h>

namespace nx
{
	class NcaHeader : public ISerialiseableBinary
	{
	public:
		enum DistributionType
		{
			DIST_DOWNLOAD,
			DIST_GAME_CARD
		};

		enum ContentType
		{
			TYPE_PROGRAM,
			TYPE_META,
			TYPE_CONTROL,
			TYPE_MANUAL,
			TYPE_DATA,
		};

		enum EncryptionType
		{
			CRYPT_AUTO,
			CRYPT_NONE,
			CRYPT_AESCTR = 3
		};

		enum EncryptionKeyIndex
		{
			KEY_UNUSED_0,
			KEY_UNUSED_1,
			KEY_DEFAULT,
			KEY_UNUSED_3,
		};

		struct sSection
		{
			u64 offset;
			u64 size;
			EncryptionType enc_type;
			crypto::sha::sSha256Hash hash;

			const sSection& operator=(const sSection& other)
			{
				offset = other.offset;
				size = other.size;
				enc_type = other.enc_type;
				hash = other.hash;

				return *this;
			}

			bool operator==(const sSection& other) const
			{
				return (offset == other.offset) \
					&& (size == other.size) \
					&& (enc_type == other.enc_type) \
					&& (hash == other.hash);
			}

			bool operator!=(const sSection& other) const
			{
				return operator==(other);
			}
		};

		static const size_t kBlockSize = 0x200;

		NcaHeader();
		NcaHeader(const NcaHeader& other);
		NcaHeader(const u8* bytes, size_t len);

		bool operator==(const NcaHeader& other) const;
		bool operator!=(const NcaHeader& other) const;
		void operator=(const NcaHeader& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes, size_t len);

		// variables
		void clear();
		DistributionType getDistributionType() const;
		void setDistributionType(DistributionType type);
		ContentType getContentType() const;
		void setContentType(ContentType type);
		EncryptionType getEncryptionType() const;
		void setEncryptionType(EncryptionType type);
		EncryptionKeyIndex getKeyIndex() const;
		void setKeyIndex(EncryptionKeyIndex index);
		u64 getNcaSize() const;
		void setNcaSize(u64 size);
		u64 getProgramId() const;
		void setProgramId(u64 program_id);
		u32 getContentIndex() const;
		void setContentIndex(u32 index);
		u32 getSdkAddonVersion() const;
		void setSdkAddonVersion(u32 version);
		const fnd::List<sSection>& getSections() const;
		void addSection(const sSection& section);
		const fnd::List<crypto::aes::sAes128Key>& getEncAesKeys() const;
		void addEncAesKey(const crypto::aes::sAes128Key& key);

	private:
		const std::string kModuleName = "NCA_HEADER";
		const std::string kNcaSig = "NCA2";
		static const size_t kSectionNum = 4;
		static const size_t kAesKeyNum = 4;
		static const u32 kDefaultSdkAddonVersion = 721920;

		enum ProgramPartitionId
		{
			SECTION_CODE,
			SECTION_DATA,
			SECTION_LOGO,
		};

#pragma pack (push, 1)

		struct sNcaHeader
		{
		private:
			u8 signature_[4];
			u8 distribution_type_;
			u8 content_type_;
			u8 key_generation_;
			u8 key_area_encryption_key_index_;
			u64 nca_size_;
			u64 program_id_;
			u32 content_index_;
			u32 sdk_addon_version_;
			u8 reserved_2[0x20];
			struct sNcaSection
			{
			private:
				u32 start_; // block units
				u32 end_; // block units
				u8 enabled_;
				u8 reserved[7];
			public:
				u32 start() const { return le_word(start_); }
				void set_start(u32 offset) { start_ = le_word(offset); }

				u32 end() const { return le_word(end_); }
				void set_end(u32 offset) { end_ = le_word(offset); }

				u8 enabled() const { return enabled_; }
				void set_enabled(u8 is_enabled) { enabled_ = is_enabled; }
			} section_[kSectionNum];
			crypto::sha::sSha256Hash section_hash_[kSectionNum];
			crypto::aes::sAes128Key enc_aes_keys_[kAesKeyNum];
		public:
			const char* signature() const { return (const char*)signature_; }
			void set_signature(const char* signature) { memcpy(signature_, signature, 4); }

			u8 distribution_type() const { return distribution_type_; }
			void set_distribution_type(u8 type) { distribution_type_ = type; }

			u8 content_type() const { return content_type_; }
			void set_content_type(u8 type) { content_type_ = type; }

			u8 key_generation() const { return key_generation_; }
			void set_key_generation(u8 type) { key_generation_ = type; }

			u8 key_area_encryption_key_index() const { return key_area_encryption_key_index_; }
			void set_key_area_encryption_key_index(u8 index) { key_area_encryption_key_index_ = index; }

			u64 nca_size() const { return le_dword(nca_size_); }
			void set_nca_size(u64 nca_size) { nca_size_ = le_dword(nca_size); }

			u64 program_id() const { return le_dword(program_id_); }
			void set_program_id(u64 program_id) { program_id_ = le_dword(program_id); }

			u32 content_index() const { return le_word(content_index_); }
			void set_content_index(u32 index) { content_index_ = le_word(index); }

			u32 sdk_addon_version() const { return le_word(sdk_addon_version_); }
			void set_sdk_addon_version(u32 version) { sdk_addon_version_ = le_word(version); }

			const sNcaSection& section(u8 index) const { return section_[index%kSectionNum]; }
			sNcaSection& section(u8 index) { return section_[index%kSectionNum]; }

			const crypto::sha::sSha256Hash& section_hash(u8 index) const { return section_hash_[index%kSectionNum]; }
			crypto::sha::sSha256Hash& section_hash(u8 index) { return section_hash_[index%kSectionNum]; }

			const crypto::aes::sAes128Key& enc_aes_key(u8 index) const { return enc_aes_keys_[index%kAesKeyNum]; }
			crypto::aes::sAes128Key& enc_aes_key(u8 index) { return enc_aes_keys_[index%kAesKeyNum]; }
		};
#pragma pack (pop)

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		DistributionType mDistributionType;
		ContentType mContentType;
		EncryptionType mEncryptionType;
		EncryptionKeyIndex mKeyIndex;
		u64 mNcaSize;
		u64 mProgramId;
		u32 mContentIndex;
		u32 mSdkAddonVersion;
		fnd::List<sSection> mSections;
		fnd::List<crypto::aes::sAes128Key> mEncAesKeys;

		u64 blockNumToSize(u32 block_num) const;
		u32 sizeToBlockNum(u64 real_size) const;
		bool isEqual(const NcaHeader& other) const;
		void copyFrom(const NcaHeader& other);
	};

}