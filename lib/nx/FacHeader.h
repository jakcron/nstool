#pragma once
#include <string>
#include <fnd/memory_blob.h>
#include <nx/ISerialiseableBinary.h>

namespace nx
{
	class FacHeader :
		public ISerialiseableBinary
	{
	public:
		FacHeader();
		FacHeader(const FacHeader& other);
		FacHeader(const u8* bytes);

		bool operator==(const FacHeader& other) const;
		bool operator!=(const FacHeader& other) const;
		void operator=(const FacHeader& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes);
		void importBinary(const u8* bytes, size_t len);

		// variables
		u64 getFacSize() const;

		u64 getFsaRights() const;
		void setFsaRights(u64 flag);

		size_t getContentOwnerIdOffset() const;
		size_t getContentOwnerIdSize() const;
		void setContentOwnerIdSize(size_t size);

		size_t getSaveDataOwnerIdOffset() const;
		size_t getSaveDataOwnerIdSize() const;
		void setSaveDataOwnerIdSize(size_t size);

	private:
		const std::string kModuleName = "FAC_HEADER";
		static const u32 kFacFormatVersion = 1;

#pragma pack (push, 1)
		struct sFacHeader
		{
		private:
			u32 version_; // default 1
			u64 fac_flags_;
			struct sFacSection
			{
			private:
				u32 start_;
				u32 end_;
			public:
				u32 start() const { return le_word(start_); }
				void set_start(u32 start) { start_ = le_word(start); }

				u32 end() const { return le_word(end_); }
				void set_end(u32 end) { end_ = le_word(end); }
			} content_owner_ids_, save_data_owner_ids_; // the data for these follow later in binary. start/end relative to base of FacData instance
		public:
			u32 version() const { return le_word(version_); }
			void set_version(u32 version) { version_ = le_word(version); }

			u64 fac_flags() const { return le_dword(fac_flags_); }
			void set_fac_flags(u64 fac_flags) { fac_flags_ = le_dword(fac_flags); }

			const sFacSection& content_owner_ids() const { return content_owner_ids_; }
			sFacSection& content_owner_ids() { return content_owner_ids_; }

			const sFacSection& save_data_owner_ids() const { return save_data_owner_ids_; }
			sFacSection& save_data_owner_ids() { return save_data_owner_ids_; }
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		u64 mFsaRights;
		struct sSection
		{
			size_t offset;
			size_t size;
		} mContentOwnerIdPos, mSaveDataOwnerIdPos;

		void clearVariables();
		void calculateOffsets();
		bool isEqual(const FacHeader& other) const;
		void copyFrom(const FacHeader& other);
	};
}

