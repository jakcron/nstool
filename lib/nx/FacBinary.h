#pragma once
#include <string>
#include <fnd/memory_blob.h>
#include <fnd/List.h>
#include <nx/ISerialiseableBinary.h>
#include <nx/FacHeader.h>

class FacBinary :
	public ISerialiseableBinary
{
public:
	enum FsAccessFlag
	{
		FSA_APPLICATION_INFO = BIT(0),
		FSA_BOOT_MODE_CONTROL = BIT(1),
		FSA_CALIBRATION = BIT(2),
		FSA_SYSTEM_SAVE_DATA = BIT(3),
		FSA_GAME_CARD = BIT(4),
		FSA_SAVE_DATA_BACKUP = BIT(5),
		FSA_SAVE_DATA_MANAGEMENT = BIT(6),
		FSA_BIS_ALL_RAW = BIT(7),
		FSA_GAME_CARD_RAW = BIT(8),
		FSA_GAME_CARD_PRIVATE = BIT(9),
		FSA_SET_TIME = BIT(10),
		FSA_CONTENT_MANAGER = BIT(11),
		FSA_IMAGE_MANAGER = BIT(12),
		FSA_CREATE_SAVE_DATA = BIT(13),
		FSA_SYSTEM_SAVE_DATA_MANAGEMENT = BIT(14),
		FSA_BIS_FILE_SYSTEM = BIT(15),
		FSA_SYSTEM_UPDATE = BIT(16),
		FSA_SAVE_DATA_META = BIT(17),
		FSA_DEVICE_SAVE_CONTROL = BIT(19),
		FSA_SETTINGS_CONTROL = BIT(20),
		FSA_DEBUG = BIT(62),
		FSA_FULL_PERMISSION = BIT(63),
	};

	FacBinary();
	FacBinary(const FacBinary& other);
	FacBinary(const u8* bytes, size_t len);

	bool operator==(const FacBinary& other) const;
	bool operator!=(const FacBinary& other) const;
	void operator=(const FacBinary& other);

	// to be used after export
	const u8* getBytes() const;
	size_t getSize() const;

	// export/import binary
	void exportBinary();
	void importBinary(const u8* bytes);
	void importBinary(const u8* bytes, size_t len);

	// variables
	bool isPermissionSet(FsAccessFlag flag) const;
	void addPermission(FsAccessFlag flag);
	void removePermission(FsAccessFlag flag);

	const fnd::List<u32>& getContentOwnerIds() const;
	void addContentOwnerId(u32 id);

	const fnd::List<u32>& getSaveDataOwnerIds() const;
	void addSaveDataOwnerId(u32 id);
private:
	const std::string kModuleName = "FAC_BINARY";

	// raw binary
	fnd::MemoryBlob mBinaryBlob;

	// variables
	FacHeader mHeader;
	u64 mFsaRights;
	fnd::List<u32> mContentOwnerIds;
	fnd::List<u32> mSaveDataOwnerIds;

	void clearVariables();
	bool isEqual(const FacBinary& other) const;
	void copyFrom(const FacBinary& other);
};

