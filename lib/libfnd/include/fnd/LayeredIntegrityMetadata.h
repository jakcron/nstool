#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/sha.h>

namespace fnd
{
	class LayeredIntegrityMetadata
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

		LayeredIntegrityMetadata();

		void operator=(const LayeredIntegrityMetadata& other);
		bool operator==(const LayeredIntegrityMetadata& other) const;
		bool operator!=(const LayeredIntegrityMetadata& other) const;

		const fnd::List<sLayer>& getHashLayerInfo() const;
		void setHashLayerInfo(const fnd::List<sLayer>& layer_info);

		const sLayer& getDataLayer() const;
		void setDataLayerInfo(const sLayer& data_info);

		const fnd::List<fnd::sha::sSha256Hash>& getMasterHashList() const;
		void setMasterHashList(const fnd::List<fnd::sha::sSha256Hash>& master_hash_list);

		bool getAlignHashToBlock() const;
		void setAlignHashToBlock(bool doAlign);
	private:
		// data
		fnd::List<sLayer> mLayerInfo;
		sLayer mDataLayer;
		fnd::List<fnd::sha::sSha256Hash> mMasterHashList;
		bool mDoAlignHashToBlock;
	};
}