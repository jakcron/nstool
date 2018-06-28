#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialisable.h>
#include <nx/aci.h>
#include <nx/FileSystemAccessControlBinary.h>
#include <nx/SacBinary.h>
#include <nx/KcBinary.h>

namespace nx
{
	class AccessControlInfoBinary : public fnd::ISerialisable
	{
	public:
		AccessControlInfoBinary();
		AccessControlInfoBinary(const AccessControlInfoBinary& other);

		void operator=(const AccessControlInfoBinary& other);
		bool operator==(const AccessControlInfoBinary& other) const;
		bool operator!=(const AccessControlInfoBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getProgramId() const;
		void setProgramId(uint64_t program_id);

		const nx::FileSystemAccessControlBinary& getFileSystemAccessControl() const;
		void setFileSystemAccessControl(const FileSystemAccessControlBinary& fac);

		const nx::SacBinary& getServiceAccessControl() const;
		void setServiceAccessControl(const SacBinary& sac);

		const nx::KcBinary& getKernelCapabilities() const;
		void setKernelCapabilities(const KcBinary& kc);
	private:
		const std::string kModuleName = "ACCESS_CONTROL_INFO_BINARY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mProgramId;
		nx::FileSystemAccessControlBinary mFileSystemAccessControl;
		nx::SacBinary mServiceAccessControl;
		nx::KcBinary mKernelCapabilities;
	};
}