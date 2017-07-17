#pragma once
#include <string>
#include <fnd/memory_blob.h>
#include <fnd/List.h>
#include <nx/ISerialiseableBinary.h>
#include <nx/FacHeader.h>


namespace nx
{
	class FacBinary :
		public FacHeader
	{
	public:
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
		void importBinary(const u8* bytes, size_t len);

		// variables
		void clear();

		const fnd::List<u32>& getContentOwnerIdList() const;
		void setContentOwnerIdList(const fnd::List<u32>& list);

		const fnd::List<u32>& getSaveDataOwnerIdList() const;
		void setSaveDataOwnerIdList(const fnd::List<u32>& list);

	private:
		const std::string kModuleName = "FAC_BINARY";
		static const u32 kFacFormatVersion = 1;

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		fnd::List<u32> mContentOwnerIdList;
		fnd::List<u32> mSaveDataOwnerIdList;

		bool isEqual(const FacBinary& other) const;
		void copyFrom(const FacBinary& other);
	};
}

