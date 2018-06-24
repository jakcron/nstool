#pragma once
#include <string>
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

		void operator=(const FacBinary& other);
		bool operator==(const FacBinary& other) const;
		bool operator!=(const FacBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

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
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::List<uint32_t> mContentOwnerIdList;
		fnd::List<uint32_t> mSaveDataOwnerIdList;
	};
}

