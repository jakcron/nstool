#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class FacHeader :
		public fnd::ISerialiseableBinary
	{
	public:
		enum FsAccessFlag
		{
			FSA_APPLICATION_INFO,
			FSA_BOOT_MODE_CONTROL,
			FSA_CALIBRATION,
			FSA_SYSTEM_SAVE_DATA,
			FSA_GAME_CARD,
			FSA_SAVE_DATA_BACKUP,
			FSA_SAVE_DATA_MANAGEMENT,
			FSA_BIS_ALL_RAW,
			FSA_GAME_CARD_RAW,
			FSA_GAME_CARD_PRIVATE,
			FSA_SET_TIME,
			FSA_CONTENT_MANAGER,
			FSA_IMAGE_MANAGER,
			FSA_CREATE_SAVE_DATA,
			FSA_SYSTEM_SAVE_DATA_MANAGEMENT,
			FSA_BIS_FILE_SYSTEM,
			FSA_SYSTEM_UPDATE,
			FSA_SAVE_DATA_META,
			FSA_DEVICE_SAVE_CONTROL,
			FSA_SETTINGS_CONTROL,
			FSA_DEBUG = 62,
			FSA_FULL_PERMISSION = 63,
		};

		FacHeader();
		FacHeader(const FacHeader& other);
		FacHeader(const u8* bytes, size_t len);

		bool operator==(const FacHeader& other) const;
		bool operator!=(const FacHeader& other) const;
		void operator=(const FacHeader& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes, size_t len);

		// variables
		void clear();
		size_t getFacSize() const;

		u32 getFormatVersion() const;
		void setFormatVersion(u32 version);

		const fnd::List<FsAccessFlag>& getFsaRightsList() const;
		void setFsaRightsList(const fnd::List<FsAccessFlag>& list);

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
		u32 mVersion;
		fnd::List<FsAccessFlag> mFsaRights;
		struct sSection
		{
			size_t offset;
			size_t size;
		} mContentOwnerIdPos, mSaveDataOwnerIdPos;

		void calculateOffsets();
		bool isEqual(const FacHeader& other) const;
		void copyFrom(const FacHeader& other);
	};
}

