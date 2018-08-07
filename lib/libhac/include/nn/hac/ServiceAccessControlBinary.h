#pragma once
#include <string>
#include <vector>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nn/hac/ServiceAccessControlEntry.h>

namespace nn
{
namespace hac
{
	class ServiceAccessControlBinary :
		public fnd::ISerialisable
	{
	public:
		ServiceAccessControlBinary();
		ServiceAccessControlBinary(const ServiceAccessControlBinary& other);

		void operator=(const ServiceAccessControlBinary& other);
		bool operator==(const ServiceAccessControlBinary& other) const;
		bool operator!=(const ServiceAccessControlBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();
		const fnd::List<ServiceAccessControlEntry>& getServiceList() const;
		void addService(const ServiceAccessControlEntry& service);
	private:
		const std::string kModuleName = "SERVICE_ACCESS_CONTROL_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::List<ServiceAccessControlEntry> mServices;
	};
}
}
