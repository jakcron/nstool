#pragma once
#include <nx/hierarchicalsha256.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class HierarchicalSha256Header :
		public fnd::ISerialiseableBinary
	{
	public:
		struct sLayer
		{
			size_t offset;
			size_t size;

			void operator=(const sLayer& other)
			{
				offset = other.offset;
				size = other.size;
			}

			bool operator==(const sLayer& other)
			{
				return (offset == other.offset && size == other.size);
			}

			bool operator!=(const sLayer& other)
			{
				return !(*this == other);
			}
		};

		HierarchicalSha256Header();
		HierarchicalSha256Header(const HierarchicalSha256Header& other);
		HierarchicalSha256Header(const byte_t* bytes, size_t len);

		bool operator==(const HierarchicalSha256Header& other) const;
		bool operator!=(const HierarchicalSha256Header& other) const;
		void operator=(const HierarchicalSha256Header& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();

		const crypto::sha::sSha256Hash& getMasterHash() const;
		void setMasterHash(const crypto::sha::sSha256Hash& master_hash);

		size_t getHashBlockSize() const;
		void setHashBlockSize(size_t hash_block_size);

		const fnd::List<sLayer>& getLayerInfo() const;
		void setLayerInfo(const fnd::List<sLayer>& layer_info);
	private:
		const std::string kModuleName = "HIERARCHICAL_SHA256_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		crypto::sha::sSha256Hash mMasterHash;
		size_t mHashBlockSize;
		fnd::List<sLayer> mLayerInfo;

		bool isEqual(const HierarchicalSha256Header& other) const;
		void copyFrom(const HierarchicalSha256Header& other);
	};

}