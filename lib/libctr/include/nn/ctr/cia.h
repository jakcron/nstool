#pragma once
#include <fnd/types.h>
#include <fnd/sha.h>
#include <fnd/rsa.h>
#include <nn/ctr/macro.h>

namespace nn
{
namespace ctr
{
	namespace cia
	{
		static const uint16_t kDefaultType = 0x0;
		static const uint16_t kDefaultFormatVersion = 0x0;
		static const size_t kContentBitfieldArrayLen = 0x2000;	
	}

#pragma pack(push, 1)
	struct sCiaHeader
	{
		le_uint32_t header_size;
		le_uint16_t type;
		le_uint16_t format_version;
		le_uint32_t cert_chain_size;
		le_uint32_t ticket_size;
		le_uint32_t tmd_size;
		le_uint32_t lotcheck_data_size;
		le_uint64_t content_size;
		byte_t included_content_bitfield[cia::kContentBitfieldArrayLen];
	};

	struct sLotCheckData
	{
		byte_t reserved_for_dependent_module_title_ids[0x180];
		byte_t reserved_00[0x180];
		le_uint32_t core_version;
		byte_t reserved_01[0xFC];
		// smdh icon data
	};
#pragma pack(pop)
}
}