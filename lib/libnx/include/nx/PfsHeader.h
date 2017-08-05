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
		PfsHeader(const u8* bytes, size_t len);

		bool operator==(const PfsHeader& other) const;
		bool operator!=(const PfsHeader& other) const;
		void operator=(const PfsHeader& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes, size_t len);

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
		private:
			u64 data_offset_;
			u64 size_;
			u64 name_offset_;
		public:
			u64 offset() const { return le_dword(data_offset_); }
			void set_offset(u64 offset) { data_offset_ = le_dword(offset); }

			u64 size() const { return le_dword(size_); }
			void set_size(u64 size) { size_ = le_dword(size); }

			u64 name_offset() const { return le_dword(name_offset_); }
			void set_name_offset(u64 offset) { name_offset_ = le_dword(offset); }
		};

		struct sPfsHeader
		{
		private:
			u8 signature_[4];
			u32 file_num_;
			u64 name_table_size_;
		public:
			const char* signature() const { return (const char*)signature_; }
			void set_signature(const char* signature) { memcpy(signature_, signature, 4); }

			u32 file_num() const { return le_word(file_num_); }
			void set_file_num(u32 file_num) { file_num_ = le_word(file_num); }

			u64 name_table_size() const { return le_dword(name_table_size_); }
			void set_name_table_size(u64 size) { name_table_size_ = le_dword(size); }
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

