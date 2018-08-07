#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialisable.h>

namespace nn
{
namespace hac
{
	class ServiceAccessControlEntry : 
		public fnd::ISerialisable
	{
	public:
		ServiceAccessControlEntry();
		ServiceAccessControlEntry(const std::string& name, bool isServer);
		ServiceAccessControlEntry(const ServiceAccessControlEntry& other);

		void operator=(const ServiceAccessControlEntry& other);
		bool operator==(const ServiceAccessControlEntry& other) const;
		bool operator!=(const ServiceAccessControlEntry& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();
		bool isServer() const;
		void setIsServer(bool isServer);
		const std::string& getName() const;
		void setName(const std::string& name);
	private:
		const std::string kModuleName = "SERVICE_ACCESS_CONTROL_ENTRY";
		static const size_t kMaxServiceNameLen = 8;

		enum ServiceAccessControlEntryFlag
		{
			SAC_IS_SERVER = _BIT(7),
			SAC_NAME_LEN_MASK = _BIT(7) - 1
		};

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		bool mIsServer;
		std::string mName;
	};
}
}