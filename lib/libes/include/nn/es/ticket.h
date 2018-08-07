#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/rsa.h>

namespace nn
{
namespace es
{
	namespace ticket
	{
		enum TitleKeyEncType
		{
			AES128_CBC,
			RSA2048
		};

		enum LicenseType
		{
			LICENSE_PERMANENT = 0,
			LICENSE_DEMO = 1,
			LICENSE_TRIAL = 2,
			LICENSE_RENTAL = 3,
			LICENSE_SUBSCRIPTION = 4,
			LICENSE_SERVICE = 5,
		};

		enum PropertyMaskFlags
		{
			FLAG_PRE_INSTALL,
			FLAG_SHARED_TITLE,
			FLAG_ALLOW_ALL_CONTENT
		};

		enum SectionType
		{
			SECTION_PERMANENT = 1,
			SECTION_SUBSCRIPTION = 2,
			SECTION_CONTENT = 3,
			SECTION_CONTENT_CONSUMPTION = 4,
			SECTION_ACCESS_TITLE = 5,
			SECTION_LIMITED_RESOURCE = 6,
		};

		static const size_t kIssuerSize = 0x40;
		static const byte_t kFormatVersion = 2;
		static const size_t kEncTitleKeySize = fnd::rsa::kRsa2048Size;
		static const size_t kReservedRegionSize = 8;
		static const size_t kRightsIdSize = 16;
	}
#pragma pack(push,1)
	struct sTicketBody_v2
	{
		char issuer[ticket::kIssuerSize];
		byte_t enc_title_key[ticket::kEncTitleKeySize];
		byte_t format_version;
		byte_t title_key_enc_type;
		le_uint16_t ticket_version;
		byte_t license_type;
		byte_t common_key_id;
		byte_t property_mask;
		byte_t reserved_0;
		byte_t reserved_region[ticket::kReservedRegionSize]; // explicitly reserved
		le_uint64_t ticket_id;
		le_uint64_t device_id;
		byte_t rights_id[ticket::kRightsIdSize];
		le_uint32_t account_id;
		le_uint32_t sect_total_size;
		le_uint32_t sect_header_offset;
		le_uint16_t sect_num;
		le_uint16_t sect_entry_size;
	};

	struct sSectionHeader_v2
	{
		le_uint32_t section_offset;
		le_uint32_t record_size;
		le_uint32_t section_size;
		le_uint16_t record_num;
		le_uint16_t section_type;
	};

	struct sContentRecord_v1
	{
	private:
		static const size_t kAccessMaskSize = 0x80;
		static const uint16_t kGroupMask = 0xFC00;
		static const uint16_t kAccessMaskMask = 0x3FF;

		be_uint32_t group;
		byte_t access_mask[kAccessMaskSize];
	public:
		uint32_t index_group() const { return group.get(); }
		bool is_index_enabled(uint16_t index) const
		{
			return (index_group() == get_group(index)) \
				&& ((access_mask[get_access_mask(index) / 8] & BIT(get_access_mask(index) % 8)) != 0);
		}

		void clear() { memset(this, 0, sizeof(sContentRecord_v1)); }

		void set_index_group(uint16_t index) { group = get_group(index); }
		void enable_index(uint16_t index) { access_mask[get_access_mask(index) / 8] |= BIT(get_access_mask(index) % 8); }
		void disable_index(uint16_t index) { access_mask[get_access_mask(index) / 8] &= ~BIT(get_access_mask(index) % 8); }

		inline uint16_t get_access_mask(uint16_t index) const { return index & kAccessMaskMask; }
		inline uint16_t get_group(uint16_t index) const { return index & kGroupMask; }
	};
#pragma pack(pop)
}
}