#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/macro.h>

namespace nx
{
	namespace aci
	{
		static const uint32_t kAciStructSig = _MAKE_STRUCT_SIGNATURE("ACI0");
		static const uint32_t kAciDescStructSig = _MAKE_STRUCT_SIGNATURE("ACID");
		static const size_t kAciAlignSize = 0x10;

		enum Flags
		{
			FLAG_PRODUCTION,
			FLAG_UNQUALIFIED_APPROVAL
		};
	}
#pragma pack(push,1)
	struct sAciHeader
	{
		le_uint32_t signature;
		le_uint32_t size; // includes prefacing signature, set only in ACID made by SDK (it enables easy resigning)
		byte_t reserved_0[4];
		le_uint32_t flags; // set in ACID only
		union uProgramIdInfo
		{
			struct sRestrictProgramId
			{
				le_uint64_t min;
				le_uint64_t max;
			} program_id_restrict;
			le_uint64_t program_id;
		} program_id_info;
		struct sAciSection
		{
			le_uint32_t offset; // aligned by 0x10 from the last one
			le_uint32_t size;
		} fac, sac, kc;
	};
#pragma pack(pop)
}