#pragma once
#include <nx/nro-hb.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class NroAssetHeader : 
		public fnd::ISerialiseableBinary
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
				return !(*this == other);
			}
		};

		NroAssetHeader();
		NroAssetHeader(const NroAssetHeader& other);
		NroAssetHeader(const byte_t* bytes, size_t len);

		bool operator==(const NroAssetHeader& other) const;
		bool operator!=(const NroAssetHeader& other) const;
		void operator=(const NroAssetHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

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
		fnd::MemoryBlob mBinaryBlob;

		// data
		sSection mIconInfo;
		sSection mNacpInfo;
		sSection mRomfsInfo;
		
		// helpers
		bool isEqual(const NroAssetHeader& other) const;
		void copyFrom(const NroAssetHeader& other);
	};

}