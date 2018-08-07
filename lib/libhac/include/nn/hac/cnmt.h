#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>

namespace nn
{
namespace hac
{
	namespace cnmt
	{
		enum ContentType
		{
			TYPE_META = 0,
			TYPE_PROGRAM,
			TYPE_DATA,
			TYPE_CONTROL,
			TYPE_HTML_DOCUMENT,
			TYPE_LEGAL_INFORMATION,
			TYPE_DELTA_FRAGMENT
		};

		enum ContentMetaType
		{
			METATYPE_SYSTEM_PROGRAM = 1,
			METATYPE_SYSTEM_DATA,
			METATYPE_SYSTEM_UPDATE,
			METATYPE_BOOT_IMAGE_PACKAGE,
			METATYPE_BOOT_IMAGE_PACKAGE_SAFE,

			METATYPE_APPLICATION = 0x80,
			METATYPE_PATCH, // can have extended data
			METATYPE_ADD_ON_CONTENT,
			METATYPE_DELTA // can have extended data
		};

		enum UpdateType
		{
			UPDATETYPE_APPLY_AS_DELTA,
			UPDATETYPE_OVERWRITE,
			UPDATETYPE_CREATE
		};

		enum ContentMetaAttribute
		{
			ATTRIBUTE_INCLUDES_EX_FAT_DRIVER,
			ATTRIBUTE_REBOOTLESS
		};

		static const uint32_t kRequiredSystemVersion = 335544320;
		static const uint32_t kDefaultVersion = 335545344;
		static const size_t kContentIdLen = 0x10;
		static const size_t kDigestLen = 0x20;
	}


#pragma pack(push,1)
	/*
	struct sContentMeta
	{
		sContentMetaHeader hdr;
		byte_t exhdr[]; // optional
		sContentInfo info[];
		sContentMetaInfo meta[];
		byte_t extdata[];
		byte_t digest[32]
	};
	*/

	struct sContentMetaHeader
	{
		le_uint64_t id;
		le_uint32_t version;
		byte_t type;
		byte_t reserved_0;
		le_uint16_t exhdr_size;
		le_uint16_t content_count;
		le_uint16_t content_meta_count;
		byte_t attributes;
		byte_t reserved_1[3];
		le_uint32_t required_download_system_version;
		byte_t reserved_2[4];
	};

	struct sContentInfo
	{
		fnd::sha::sSha256Hash content_hash;
		byte_t content_id[cnmt::kContentIdLen];
		le_uint32_t size_lower;
		le_uint16_t size_higher;
		byte_t content_type;
		byte_t id_offset;
	};

	struct sContentMetaInfo
	{
		le_uint64_t id;
		le_uint32_t version;
		byte_t type;
		byte_t attributes;
		byte_t reserved[2];
	};

	struct sApplicationMetaExtendedHeader
	{
		le_uint64_t patch_id;
		le_uint32_t required_system_version;
		byte_t reserved[4];
	};

	struct sPatchMetaExtendedHeader
	{
		le_uint64_t application_id;
		le_uint32_t required_system_version;
		le_uint32_t extended_data_size;
		byte_t reserved[8];
	};

	struct sAddOnContentMetaExtendedHeader
	{
		le_uint64_t application_id;
		le_uint32_t required_system_version;
		byte_t reserved[4];
	};

	struct sDeltaMetaExtendedHeader
	{
		le_uint64_t application_id;
		le_uint32_t extended_data_size;
		byte_t reserved[4];
	};

	struct sDigest
	{
		byte_t data[cnmt::kDigestLen];
	};
#pragma pack(pop)
}
}