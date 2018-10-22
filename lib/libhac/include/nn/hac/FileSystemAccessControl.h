#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IByteModel.h>
#include <fnd/List.h>
#include <nn/hac/define/fac.h>

namespace nn
{
namespace hac
{
	class FileSystemAccessControl :
		public fnd::IByteModel
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

		FileSystemAccessControl();
		FileSystemAccessControl(const FileSystemAccessControl& other);

		void operator=(const FileSystemAccessControl& other);
		bool operator==(const FileSystemAccessControl& other) const;
		bool operator!=(const FileSystemAccessControl& other) const;

		// IByteModel
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
		const std::string kModuleName = "FILE_SYSTEM_ACCESS_CONTROL";

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