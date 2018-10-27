#pragma once
#include <string>
#include <vector>
#include <fnd/IByteModel.h>
#include <fnd/List.h>
#include <nn/hac/ServiceAccessControlEntry.h>

namespace nn
{
namespace hac
{
	class ServiceAccessControl :
		public fnd::IByteModel
	{
	public:
		ServiceAccessControl();
		ServiceAccessControl(const ServiceAccessControl& other);

		void operator=(const ServiceAccessControl& other);
		bool operator==(const ServiceAccessControl& other) const;
		bool operator!=(const ServiceAccessControl& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();
		const fnd::List<ServiceAccessControlEntry>& getServiceList() const;
		void setServiceList(const fnd::List<ServiceAccessControlEntry>& list);
	private:
		const std::string kModuleName = "SERVICE_ACCESS_CONTROL";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::List<ServiceAccessControlEntry> mServices;
	};
}
}
