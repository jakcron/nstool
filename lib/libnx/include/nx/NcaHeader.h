#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class NcaHeader : 
		public fnd::ISerialiseableBinary
	{
	public:
		enum FormatVersion
		{
			NCA2_FORMAT,
			NCA3_FORMAT
		};

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

		enum KeyBankIndex
		{
			KEY_AESXTS_0,
			KEY_AESXTS_1,
			KEY_AESCTR,
			KEY_UNUSED_3,
		};

		struct sSection
		{
			u64 offset;
			u64 size;
			crypto::sha::sSha256Hash hash;

			const sSection& operator=(const sSection& other)
			{
				offset = other.offset;
				size = other.size;
				hash = other.hash;

				return *this;
			}

			bool operator==(const sSection& other) const
			{
				return (offset == other.offset) \
					&& (size == other.size) \
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
		FormatVersion getFormatVersion() const;
		void setFormatVersion(FormatVersion ver);
		DistributionType getDistributionType() const;
		void setDistributionType(DistributionType type);
		ContentType getContentType() const;
		void setContentType(ContentType type);
		byte_t getCryptoType() const;
		void setCryptoType(byte_t type);
		byte_t getKaekIndex() const;
		void setKaekIndex(byte_t index);
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
		const std::string kNca2Sig = "NCA2";
		const std::string kNca3Sig = "NCA3";
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
			char signature[4];
			byte_t distribution_type;
			byte_t content_type;
			byte_t crypto_type; // KeyGeneration
			byte_t key_area_encryption_key_index;
			le_uint64_t nca_size;
			le_uint64_t program_id;
			le_uint32_t content_index;
			le_uint32_t sdk_addon_version;
			byte_t crypto_type_2;
			byte_t reserved_2[0xf];
			byte_t rights_id[0x10];
			struct sNcaSection
			{
				le_uint32_t start; // block units
				le_uint32_t end; // block units
				byte_t enabled;
				byte_t reserved[7];
			} section[kSectionNum];
			crypto::sha::sSha256Hash section_hash[kSectionNum];
			crypto::aes::sAes128Key enc_aes_key[kAesKeyNum];
		
		};
#pragma pack (pop)

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		FormatVersion mFormatVersion;
		DistributionType mDistributionType;
		ContentType mContentType;
		byte_t mCryptoType;
		byte_t mKaekIndex;
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