#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nx/fac.h>

namespace nx
{
	class FacBinary :
		public fnd::ISerialisable
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

		const fnd::List<fac::FsAccessFlag>& getFsaRightsList() const;
		void setFsaRightsList(const fnd::List<fac::FsAccessFlag>& list);

		const fnd::List<uint32_t>& getContentOwnerIdList() const;
		void setContentOwnerIdList(const fnd::List<uint32_t>& list);

		const fnd::List<uint32_t>& getSaveDataOwnerIdList() const;
		void setSaveDataOwnerIdList(const fnd::List<uint32_t>& list);

	private:
		const std::string kModuleName = "FAC_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::List<fac::FsAccessFlag> mFsaRights;
		fnd::List<uint32_t> mContentOwnerIdList;
		fnd::List<uint32_t> mSaveDataOwnerIdList;
	};
}

