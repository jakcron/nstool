#pragma once
#include <string>
#include <vector>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nx/SacEntry.h>

namespace nx
{
	class SacBinary :
		public fnd::ISerialisable
	{
	public:
		SacBinary();
		SacBinary(const SacBinary& other);

		void operator=(const SacBinary& other);
		bool operator==(const SacBinary& other) const;
		bool operator!=(const SacBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();
		const fnd::List<SacEntry>& getServiceList() const;
		void addService(const SacEntry& service);
	private:
		const std::string kModuleName = "SAC_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::List<SacEntry> mServices;
	};

}
