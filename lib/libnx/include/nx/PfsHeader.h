#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>


namespace nx
{
	class PfsHeader :
		public fnd::ISerialiseableBinary
	{
	public:
		struct sFile
		{
			std::string name;
			size_t offset;
			size_t size;

			sFile& operator=(const sFile& other)
			{
				name = other.name;
				offset = other.offset;
				size = other.size;
				return *this;
			}

			bool operator==(const sFile& other) const
			{
				return (name == other.name) \
					&& (offset == other.offset) \
					&& (size == other.size);
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

		PfsHeader();
		PfsHeader(const PfsHeader& other);
		PfsHeader(const byte_t* bytes, size_t len);

		bool operator==(const PfsHeader& other) const;
		bool operator!=(const PfsHeader& other) const;
		void operator=(const PfsHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();

		const fnd::List<sFile>& getFileList() const;
		void addFile(const std::string& name, size_t size);

	private:
		const std::string kModuleName = "PFS_HEADER";
		const std::string kPfsStructSig = "PFS0";
		static const size_t kPfsAlign = 0x40;

#pragma pack (push, 1)
		struct sPfsFile
		{
			le_uint64_t data_offset;
			le_uint64_t size;
			le_uint64_t name_offset;
		};

		struct sPfsHeader
		{
			char signature[4];
			le_uint32_t file_num;
			le_uint64_t name_table_size;
		};
#pragma pack (pop)

		// binary blob
		fnd::MemoryBlob mBinaryBlob;

		// variables
		fnd::List<sFile> mFileList;

		void calculateOffsets(size_t data_offset);
		bool isEqual(const PfsHeader& other) const;
		void copyFrom(const PfsHeader& other);
	};
}

