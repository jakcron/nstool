#pragma once
#include <nx/hierarchicalsha256.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class HierarchicalIntegrityHeader :
		public fnd::ISerialiseableBinary
	{
	public:
		struct sLayer
		{
			size_t offset;
			size_t size;
			size_t hash_block_size;

			void operator=(const sLayer& other)
			{
				offset = other.offset;
				size = other.size;
				hash_block_size = other.hash_block_size;
			}

			bool operator==(const sLayer& other)
			{
				return (offset == other.offset && size == other.size && hash_block_size == other.hash_block_size);
			}

			bool operator!=(const sLayer& other)
			{
				return !(*this == other);
			}
		};

		HierarchicalIntegrityHeader();
		HierarchicalIntegrityHeader(const HierarchicalIntegrityHeader& other);
		HierarchicalIntegrityHeader(const byte_t* bytes, size_t len);

		bool operator==(const HierarchicalIntegrityHeader& other) const;
		bool operator!=(const HierarchicalIntegrityHeader& other) const;
		void operator=(const HierarchicalIntegrityHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();

		const fnd::List<sLayer>& getLayerInfo() const;
		void setLayerInfo(const fnd::List<sLayer>& layer_info);

		const fnd::List<crypto::sha::sSha256Hash>& getMasterHashList() const;
		void setMasterHashList(const fnd::List<crypto::sha::sSha256Hash>& master_hash_list);
	private:
		const std::string kModuleName = "HIERARCHICAL_INTEGRITY_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		fnd::List<sLayer> mLayerInfo;
		fnd::List<crypto::sha::sSha256Hash> mMasterHashList;

		bool isEqual(const HierarchicalIntegrityHeader& other) const;
		void copyFrom(const HierarchicalIntegrityHeader& other);
	};

}