#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <nx/FacHeader.h>


namespace nx
{
	class FacBinary :
		public FacHeader
	{
	public:
		FacBinary();
		FacBinary(const FacBinary& other);
		FacBinary(const byte_t* bytes, size_t len);

		bool operator==(const FacBinary& other) const;
		bool operator!=(const FacBinary& other) const;
		void operator=(const FacBinary& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();

		const fnd::List<uint32_t>& getContentOwnerIdList() const;
		void setContentOwnerIdList(const fnd::List<uint32_t>& list);

		const fnd::List<uint32_t>& getSaveDataOwnerIdList() const;
		void setSaveDataOwnerIdList(const fnd::List<uint32_t>& list);

	private:
		const std::string kModuleName = "FAC_BINARY";
		static const uint32_t kFacFormatVersion = 1;

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		fnd::List<uint32_t> mContentOwnerIdList;
		fnd::List<uint32_t> mSaveDataOwnerIdList;

		bool isEqual(const FacBinary& other) const;
		void copyFrom(const FacBinary& other);
	};
}

