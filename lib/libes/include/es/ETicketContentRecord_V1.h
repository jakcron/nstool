#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace es
{
	class ETicketContentRecord_V1 :
		public fnd::ISerialiseableBinary
	{
	public:
		ETicketContentRecord_V1();

	private:
		const std::string kModuleName = "ETICKET_CONTENT_RECORD_V1";

#pragma pack (push, 1)
		struct sContentRecord_v1
		{
		private:
			static const size_t kAccessMaskSize = 0x80;
			static const u16 kGroupMask = 0xFC00;
			static const u16 kAccessMaskMask = 0x3FF;

			u32 group_;
			u8 access_mask_[kAccessMaskSize];
		public:
			u32 index_group() const { return be_word(group_); }
			bool is_index_enabled(u16 index) const
			{
				return (index_group() == get_group(index)) \
					&& ((access_mask_[get_access_mask(index) / 8] & BIT(get_access_mask(index) % 8)) != 0);
			}

			void clear() { memset(this, 0, sizeof(sContentRecord_v1)); }

			void set_index_group(u16 index) { group_ = be_hword(get_group(index)); }
			void enable_index(u16 index) { access_mask_[get_access_mask(index) / 8] |= BIT(get_access_mask(index) % 8); }
			void disable_index(u16 index) { access_mask_[get_access_mask(index) / 8] &= ~BIT(get_access_mask(index) % 8); }

			inline u16 get_access_mask(u16 index) const { return index & kAccessMaskMask; }
			inline u16 get_group(u16 index) const { return index & kGroupMask; }
		};
#pragma pack (pop)
	};
}