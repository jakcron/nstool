#pragma once
#include <nx/aset.h>
#include <fnd/List.h>
#include <fnd/ISerialisable.h>

namespace nx
{
	class AssetHeader : 
		public fnd::ISerialisable
	{
	public:
		struct sSection
		{
			uint64_t offset;
			uint64_t size;
		};

		AssetHeader();
		AssetHeader(const AssetHeader& other);

		bool operator==(const AssetHeader& other) const;
		bool operator!=(const AssetHeader& other) const;
		void operator=(const AssetHeader& other);

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const sSection& getIconInfo() const;
		void setIconInfo(const sSection& info);

		const sSection& getNacpInfo() const;
		void setNacpInfo(const sSection& info);

		const sSection& getRomfsInfo() const;
		void setRomfsInfo(const sSection& info);
	private:
		const std::string kModuleName = "NRO_ASSET_HEADER";

		// binary
		fnd::Vec<byte_t> mRawBinary;

		// data
		sSection mIconInfo;
		sSection mNacpInfo;
		sSection mRomfsInfo;
	};

}