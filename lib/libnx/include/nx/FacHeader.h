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
		FacHeader(const byte_t* bytes, size_t len);

		bool operator==(const FacHeader& other) const;
		bool operator!=(const FacHeader& other) const;
		void operator=(const FacHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();
		size_t getFacSize() const;

		uint32_t getFormatVersion() const;
		void setFormatVersion(uint32_t version);

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
		static const uint32_t kFacFormatVersion = 1;

#pragma pack (push, 1)
		struct sFacHeader
		{
			le_uint32_t version; // default 1
			le_uint64_t fac_flags;
			struct sFacSection
			{
				le_uint32_t start;
				le_uint32_t end;
			} content_owner_ids, save_data_owner_ids; // the data for these follow later in binary. start/end relative to base of FacData instance
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		uint32_t mVersion;
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

