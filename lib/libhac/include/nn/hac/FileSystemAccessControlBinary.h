#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nn/hac/fac.h>

namespace nn
{
namespace hac
{
	class FileSystemAccessControlBinary : public fnd::ISerialisable
	{
	public:
		struct sSaveDataOwnerId
		{
			nn::hac::fac::SaveDataOwnerIdAccessType access_type;
			uint64_t id;

			void operator=(const sSaveDataOwnerId& other)
			{
				access_type = other.access_type;
				id = other.id;
			}

			bool operator==(const sSaveDataOwnerId& other) const
			{
				return (access_type == other.access_type) \
					&& (id == other.id);
			}

			bool operator!=(const sSaveDataOwnerId& other) const
			{
				return !(*this == other);
			}
		}; 

		FileSystemAccessControlBinary();
		FileSystemAccessControlBinary(const FileSystemAccessControlBinary& other);

		void operator=(const FileSystemAccessControlBinary& other);
		bool operator==(const FileSystemAccessControlBinary& other) const;
		bool operator!=(const FileSystemAccessControlBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint32_t getFormatVersion() const;
		void setFormatVersion(uint32_t version);

		const fnd::List<fac::FsAccessFlag>& getFsaRightsList() const;
		void setFsaRightsList(const fnd::List<fac::FsAccessFlag>& list);

		const fnd::List<uint64_t>& getContentOwnerIdList() const;
		void setContentOwnerIdList(const fnd::List<uint64_t>& list);

		const fnd::List<sSaveDataOwnerId>& getSaveDataOwnerIdList() const;
		void setSaveDataOwnerIdList(const fnd::List<sSaveDataOwnerId>& list);
	private:
		const std::string kModuleName = "FILE_SYSTEM_ACCESS_CONTROL_BINARY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint32_t mVersion;
		fnd::List<fac::FsAccessFlag> mFsaRights;
		fnd::List<uint64_t> mContentOwnerIdList;
		fnd::List<sSaveDataOwnerId> mSaveDataOwnerIdList;
	};
}
}