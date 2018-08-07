#pragma once
#include <fnd/types.h>
#include <fnd/rsa.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace aci
	{
		static const uint32_t kAciStructMagic = _MAKE_STRUCT_MAGIC_U32("ACI0");
		static const uint32_t kAciDescStructMagic = _MAKE_STRUCT_MAGIC_U32("ACID");
		static const size_t kSectionAlignSize = 0x10;

		enum Flag
		{
			FLAG_PRODUCTION,
			FLAG_UNQUALIFIED_APPROVAL
		};
	}
#pragma pack(push,1)
	struct sAciSection
	{
		le_uint32_t offset;
		le_uint32_t size;
	};

	struct sAciHeader
	{
		le_uint32_t st_magic;
		byte_t reserved_00[0xC];
		le_uint64_t program_id;
		byte_t reserved_01[0x8];
		sAciSection fac;
		sAciSection sac;
		sAciSection kc;
	};

	struct sAciDescHeader
	{
		byte_t signature[fnd::rsa::kRsa2048Size];
		byte_t nca_rsa_signature2_modulus[fnd::rsa::kRsa2048Size];
		le_uint32_t st_magic;
		le_uint32_t signed_size;
		byte_t reserved_00[0x4];
		le_uint32_t flags;
		le_uint64_t program_id_min;
		le_uint64_t program_id_max;
		sAciSection fac;
		sAciSection sac;
		sAciSection kc;
	};
#pragma pack(pop)
}
}