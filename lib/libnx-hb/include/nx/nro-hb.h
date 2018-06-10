#pragma once

#include <nx/nro.h>
#include <nx/macro.h>
namespace nx
{
	namespace nro
	{
		static const uint64_t kNroHomebrewSig = _MAKE_STRUCT_SIGNATURE_U64("HOMEBREW");
		static const uint32_t kAssetSig = _MAKE_STRUCT_SIGNATURE("ASET");

		static const uint32_t kDefaultAssetFormatVersion = 0;
	}

#pragma pack(push,1)
	struct sNroAssetSection
	{
		le_uint64_t offset;
		le_uint64_t size;
	};

	struct sNroAssetHeader
	{
		le_uint32_t signature;
		le_uint32_t format_version;
		sNroAssetSection icon;
		sNroAssetSection nacp;
		sNroAssetSection romfs;
	};
#pragma pack(pop)
}