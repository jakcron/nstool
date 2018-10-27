#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IByteModel.h>
#include <fnd/List.h>
#include <nn/hac/define/pfs.h>

namespace nn
{
namespace hac
{
	class PartitionFsHeader :
		public fnd::IByteModel
	{
	public:
		enum FsType
		{
			TYPE_PFS0,
			TYPE_HFS0
		};

		struct sFile
		{
			std::string name;
			size_t offset;
			size_t size;
			size_t hash_protected_size;
			fnd::sha::sSha256Hash hash;

			sFile& operator=(const sFile& other)
			{
				name = other.name;
				offset = other.offset;
				size = other.size;
				hash_protected_size = other.hash_protected_size;
				hash = other.hash;
				return *this;
			}

			bool operator==(const sFile& other) const
			{
				return (name == other.name) \
					&& (offset == other.offset) \
					&& (size == other.size) \
					&& (hash_protected_size == other.hash_protected_size) \
					&& (hash == other.hash);
			}

			bool operator!=(const sFile& other) const
			{
				return !operator==(other);
			}

			bool operator==(const std::string& other) const
			{
				return (name == other);
			}

			bool operator!=(const std::string& other) const
			{
				return !operator==(other);
			}
		};

		PartitionFsHeader();
		PartitionFsHeader(const PartitionFsHeader& other);

		void operator=(const PartitionFsHeader& other);
		bool operator==(const PartitionFsHeader& other) const;
		bool operator!=(const PartitionFsHeader& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		FsType getFsType() const;
		void setFsType(FsType type);
		const fnd::List<sFile>& getFileList() const;
		void addFile(const std::string& name, size_t size);
		void addFile(const std::string& name, size_t size, size_t hash_protected_size, const fnd::sha::sSha256Hash& hash);

	private:
		const std::string kModuleName = "PARTITIONFS_HEADER";

		// binary blob
		fnd::Vec<byte_t> mRawBinary;

		// variables
		FsType mFsType;
		fnd::List<sFile> mFileList;

		size_t getFileEntrySize(FsType fs_type);
		void calculateOffsets(size_t data_offset);
	};
}
}