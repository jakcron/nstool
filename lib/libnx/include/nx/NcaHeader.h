#pragma once
#include <nx/nca.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
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

		struct sPartition
		{
			byte_t index;
			uint64_t offset;
			uint64_t size;
			crypto::sha::sSha256Hash hash;

			const sPartition& operator=(const sPartition& other)
			{
				index = other.index;
				offset = other.offset;
				size = other.size;
				hash = other.hash;

				return *this;
			}

			bool operator==(const sPartition& other) const
			{
				return (index == other.index) \
					&& (offset == other.offset) \
					&& (size == other.size) \
					&& (hash == other.hash);
			}

			bool operator!=(const sPartition& other) const
			{
				return !operator==(other);
			}
		};

		NcaHeader();
		NcaHeader(const NcaHeader& other);
		NcaHeader(const byte_t* bytes, size_t len);

		bool operator==(const NcaHeader& other) const;
		bool operator!=(const NcaHeader& other) const;
		void operator=(const NcaHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();
		FormatVersion getFormatVersion() const;
		void setFormatVersion(FormatVersion ver);
		nca::DistributionType getDistributionType() const;
		void setDistributionType(nca::DistributionType type);
		nca::ContentType getContentType() const;
		void setContentType(nca::ContentType type);
		byte_t getKeyGeneration() const;
		void setKeyGeneration(byte_t gen);
		byte_t getKaekIndex() const;
		void setKaekIndex(byte_t index);
		uint64_t getContentSize() const;
		void setContentSize(uint64_t size);
		uint64_t getProgramId() const;
		void setProgramId(uint64_t program_id);
		uint32_t getContentIndex() const;
		void setContentIndex(uint32_t index);
		uint32_t getSdkAddonVersion() const;
		void setSdkAddonVersion(uint32_t version);
		const byte_t* getRightsId() const;
		void setRightsId(const byte_t* rights_id);
		const fnd::List<sPartition>& getPartitions() const;
		void setPartitions(const fnd::List<sPartition>& partitions);
		const fnd::List<crypto::aes::sAes128Key>& getEncAesKeys() const;
		void setEncAesKeys(const fnd::List<crypto::aes::sAes128Key>& keys);

	private:
		const std::string kModuleName = "NCA_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		FormatVersion mFormatVersion;
		nca::DistributionType mDistributionType;
		nca::ContentType mContentType;
		byte_t mKeyGeneration;
		byte_t mKaekIndex;
		uint64_t mContentSize;
		uint64_t mProgramId;
		uint32_t mContentIndex;
		uint32_t mSdkAddonVersion;
		byte_t mRightsId[nca::kRightsIdLen];
		fnd::List<sPartition> mPartitions;
		fnd::List<crypto::aes::sAes128Key> mEncAesKeys;

		uint64_t blockNumToSize(uint32_t block_num) const;
		uint32_t sizeToBlockNum(uint64_t real_size) const;
		bool isEqual(const NcaHeader& other) const;
		void copyFrom(const NcaHeader& other);
	};

}