#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IByteModel.h>
#include <nn/hac/define/aci.h>
#include <nn/hac/FileSystemAccessControl.h>
#include <nn/hac/ServiceAccessControl.h>
#include <nn/hac/KernelCapabilityControl.h>

namespace nn
{
namespace hac
{
	class AccessControlInfo :
		public fnd::IByteModel
	{
	public:
		AccessControlInfo();
		AccessControlInfo(const AccessControlInfo& other);

		void operator=(const AccessControlInfo& other);
		bool operator==(const AccessControlInfo& other) const;
		bool operator!=(const AccessControlInfo& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getProgramId() const;
		void setProgramId(uint64_t program_id);

		const nn::hac::FileSystemAccessControl& getFileSystemAccessControl() const;
		void setFileSystemAccessControl(const FileSystemAccessControl& fac);

		const nn::hac::ServiceAccessControl& getServiceAccessControl() const;
		void setServiceAccessControl(const ServiceAccessControl& sac);

		const nn::hac::KernelCapabilityControl& getKernelCapabilities() const;
		void setKernelCapabilities(const KernelCapabilityControl& kc);
	private:
		const std::string kModuleName = "ACCESS_CONTROL_INFO_BINARY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mProgramId;
		nn::hac::FileSystemAccessControl mFileSystemAccessControl;
		nn::hac::ServiceAccessControl mServiceAccessControl;
		nn::hac::KernelCapabilityControl mKernelCapabilities;
	};
}
}