#pragma once
#include <nn/hac/aset.h>
#include <fnd/List.h>
#include <fnd/ISerialisable.h>

namespace nn
{
namespace hac
{
	class AssetHeader : 
		public fnd::ISerialisable
	{
	public:
		struct sSection
		{
			uint64_t offset;
			uint64_t size;

			void operator=(const sSection& other)
			{
				offset = other.offset;
				size = other.size;
			}

			bool operator==(const sSection& other) const
			{
				return (offset == other.offset) \
					&& (size == other.size);
			}

			bool operator!=(const sSection& other) const
			{
				return !operator==(other);
			}
		};

		AssetHeader();
		AssetHeader(const AssetHeader& other);

		void operator=(const AssetHeader& other);
		bool operator==(const AssetHeader& other) const;
		bool operator!=(const AssetHeader& other) const;

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
}