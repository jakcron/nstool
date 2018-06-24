#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/ISerialisable.h>

namespace nx
{
	class SacEntry : 
		public fnd::ISerialisable
	{
	public:
		SacEntry();
		SacEntry(const std::string& name, bool isServer);
		SacEntry(const SacEntry& other);

		void operator=(const SacEntry& other);
		bool operator==(const SacEntry& other) const;
		bool operator!=(const SacEntry& other) const;

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
		const std::string kModuleName = "SAC_ENTRY";
		static const size_t kMaxServiceNameLen = 8;

		enum SacEntryFlag
		{
			SAC_IS_SERVER = _BIT(7),
			SAC_NAME_LEN_MASK = _BIT(7) - 1
		};

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		bool mIsServer;
		std::string mName;

		bool isEqual(const SacEntry& other) const;
		void copyFrom(const SacEntry& other);
	};
}