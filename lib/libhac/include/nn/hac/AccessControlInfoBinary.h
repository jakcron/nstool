#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialisable.h>
#include <nn/hac/aci.h>
#include <nn/hac/FileSystemAccessControlBinary.h>
#include <nn/hac/ServiceAccessControlBinary.h>
#include <nn/hac/KernelCapabilityBinary.h>

namespace nn
{
namespace hac
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

		const nn::hac::FileSystemAccessControlBinary& getFileSystemAccessControl() const;
		void setFileSystemAccessControl(const FileSystemAccessControlBinary& fac);

		const nn::hac::ServiceAccessControlBinary& getServiceAccessControl() const;
		void setServiceAccessControl(const ServiceAccessControlBinary& sac);

		const nn::hac::KernelCapabilityBinary& getKernelCapabilities() const;
		void setKernelCapabilities(const KernelCapabilityBinary& kc);
	private:
		const std::string kModuleName = "ACCESS_CONTROL_INFO_BINARY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mProgramId;
		nn::hac::FileSystemAccessControlBinary mFileSystemAccessControl;
		nn::hac::ServiceAccessControlBinary mServiceAccessControl;
		nn::hac::KernelCapabilityBinary mKernelCapabilities;
	};
}
}