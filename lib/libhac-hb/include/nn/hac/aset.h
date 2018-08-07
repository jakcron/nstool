#pragma once
#include <fnd/types.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace aset
	{
		static const uint32_t kAssetStructMagic = _MAKE_STRUCT_MAGIC_U32("ASET");

		static const uint32_t kDefaultAssetFormatVersion = 0;
	}

#pragma pack(push,1)
	struct sAssetSection
	{
		le_uint64_t offset;
		le_uint64_t size;
	};

	struct sAssetHeader
	{
		le_uint32_t st_magic;
		le_uint32_t format_version;
		sAssetSection icon;
		sAssetSection nacp;
		sAssetSection romfs;
	};
#pragma pack(pop)
}
}