#pragma once
#include <nn/hac/hierarchicalsha256.h>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>

namespace nn
{
namespace hac
{
	class HierarchicalSha256Header :
		public fnd::ISerialisable
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

			bool operator==(const sLayer& other) const
			{
				return (offset == other.offset && size == other.size);
			}

			bool operator!=(const sLayer& other) const
			{
				return !(*this == other);
			}
		};

		HierarchicalSha256Header();
		HierarchicalSha256Header(const HierarchicalSha256Header& other);

		void operator=(const HierarchicalSha256Header& other);
		bool operator==(const HierarchicalSha256Header& other) const;
		bool operator!=(const HierarchicalSha256Header& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>&  getBytes() const;

		// variables
		void clear();

		const fnd::sha::sSha256Hash& getMasterHash() const;
		void setMasterHash(const fnd::sha::sSha256Hash& master_hash);

		size_t getHashBlockSize() const;
		void setHashBlockSize(size_t hash_block_size);

		const fnd::List<sLayer>& getLayerInfo() const;
		void setLayerInfo(const fnd::List<sLayer>& layer_info);
	private:
		const std::string kModuleName = "HIERARCHICAL_SHA256_HEADER";

		// binary
		fnd::Vec<byte_t> mRawBinary;

		// data
		fnd::sha::sSha256Hash mMasterHash;
		size_t mHashBlockSize;
		fnd::List<sLayer> mLayerInfo;
	};
}
}