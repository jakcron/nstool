#pragma once
#include <nn/hac/nca.h>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>

namespace nn
{
namespace hac
{
	class NcaHeader : 
		public fnd::ISerialisable
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
			fnd::sha::sSha256Hash hash;

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

		void operator=(const NcaHeader& other);
		bool operator==(const NcaHeader& other) const;
		bool operator!=(const NcaHeader& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

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
		bool hasRightsId() const;
		const byte_t* getRightsId() const;
		void setRightsId(const byte_t* rights_id);
		const fnd::List<sPartition>& getPartitions() const;
		void setPartitions(const fnd::List<sPartition>& partitions);
		const fnd::List<fnd::aes::sAes128Key>& getEncAesKeys() const;
		void setEncAesKeys(const fnd::List<fnd::aes::sAes128Key>& keys);

	private:
		const std::string kModuleName = "NCA_HEADER";

		// binary
		fnd::Vec<byte_t> mRawBinary;

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
		fnd::List<fnd::aes::sAes128Key> mEncAesKeys;

		uint64_t blockNumToSize(uint32_t block_num) const;
		uint32_t sizeToBlockNum(uint64_t real_size) const;
	};
}
}