#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/IByteModel.h>
#include <nn/hac/define/aci.h>
#include <nn/hac/FileSystemAccessControl.h>
#include <nn/hac/ServiceAccessControl.h>
#include <nn/hac/KernelCapabilityControl.h>

namespace nn
{
namespace hac
{
	class AccessControlInfoDesc :
		public fnd::IByteModel
	{
	public:
		struct sProgramIdRestrict
		{
			uint64_t min;
			uint64_t max;

			void operator=(const sProgramIdRestrict& other)
			{
				min = other.min;
				max = other.max;
			}

			bool operator==(const sProgramIdRestrict& other) const
			{
				return (min == other.min) \
					&& (max == other.max);
			}

			bool operator!=(const sProgramIdRestrict& other) const
			{
				return !(*this == other);
			}
		};

		AccessControlInfoDesc();
		AccessControlInfoDesc(const AccessControlInfoDesc& other);

		void operator=(const AccessControlInfoDesc& other);
		bool operator==(const AccessControlInfoDesc& other) const;
		bool operator!=(const AccessControlInfoDesc& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		void generateSignature(const fnd::rsa::sRsa2048Key& key);
		void validateSignature(const fnd::rsa::sRsa2048Key& key) const;

		// variables
		void clear();

		const fnd::rsa::sRsa2048Key& getContentArchiveHeaderSignature2Key() const;
		void setContentArchiveHeaderSignature2Key(const fnd::rsa::sRsa2048Key& key);

		const fnd::List<aci::Flag>& getFlagList() const;
		void setFlagList(const fnd::List<aci::Flag>& flags);

		const sProgramIdRestrict& getProgramIdRestrict() const;
		void setProgramIdRestrict(const sProgramIdRestrict& pid_restrict);

		const nn::hac::FileSystemAccessControl& getFileSystemAccessControl() const;
		void setFileSystemAccessControl(const FileSystemAccessControl& fac);

		const nn::hac::ServiceAccessControl& getServiceAccessControl() const;
		void setServiceAccessControl(const ServiceAccessControl& sac);

		const nn::hac::KernelCapabilityControl& getKernelCapabilities() const;
		void setKernelCapabilities(const KernelCapabilityControl& kc);
	private:
		const std::string kModuleName = "ACCESS_CONTROL_INFO_DESC_BINARY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::rsa::sRsa2048Key mContentArchiveHeaderSignature2Key;
		fnd::List<aci::Flag> mFlags;
		sProgramIdRestrict mProgramIdRestrict;
		nn::hac::FileSystemAccessControl mFileSystemAccessControl;
		nn::hac::ServiceAccessControl mServiceAccessControl;
		nn::hac::KernelCapabilityControl mKernelCapabilities;
	};
}
}