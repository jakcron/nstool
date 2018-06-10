#pragma once

#include <fnd/types.h>
#include <nx/macro.h>
namespace nx
{
	namespace aset
	{
		static const uint32_t kAssetSig = _MAKE_STRUCT_SIGNATURE("ASET");

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
		le_uint32_t signature;
		le_uint32_t format_version;
		sAssetSection icon;
		sAssetSection nacp;
		sAssetSection romfs;
	};
#pragma pack(pop)
}