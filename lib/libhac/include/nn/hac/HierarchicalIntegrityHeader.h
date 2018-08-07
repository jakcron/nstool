#pragma once
#include <nn/hac/hierarchicalintegrity.h>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <fnd/sha.h>

namespace nn
{
namespace hac
{
	class HierarchicalIntegrityHeader :
		public fnd::ISerialisable
	{
	public:
		struct sLayer
		{
			size_t offset;
			size_t size;
			size_t block_size;

			void operator=(const sLayer& other)
			{
				offset = other.offset;
				size = other.size;
				block_size = other.block_size;
			}

			bool operator==(const sLayer& other) const
			{
				return (offset == other.offset && size == other.size && block_size == other.block_size);
			}

			bool operator!=(const sLayer& other) const
			{
				return !(*this == other);
			}
		};

		HierarchicalIntegrityHeader();
		HierarchicalIntegrityHeader(const HierarchicalIntegrityHeader& other);

		void operator=(const HierarchicalIntegrityHeader& other);
		bool operator==(const HierarchicalIntegrityHeader& other) const;
		bool operator!=(const HierarchicalIntegrityHeader& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const fnd::List<sLayer>& getLayerInfo() const;
		void setLayerInfo(const fnd::List<sLayer>& layer_info);

		const fnd::List<fnd::sha::sSha256Hash>& getMasterHashList() const;
		void setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list);
	private:
		const std::string kModuleName = "HIERARCHICAL_INTEGRITY_HEADER";

		// binary
		fnd::Vec<byte_t> mRawBinary;

		// data
		fnd::List<sLayer> mLayerInfo;
		fnd::List<fnd::sha::sSha256Hash> mMasterHashList;
	};
}
}