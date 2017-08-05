#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>

namespace es
{
	class ETicketSectionHeader_V2 :
		public fnd::ISerialiseableBinary
	{
	public:
		enum SectionType
		{
			PERMANENT = 1,
			SUBSCRIPTION = 2,
			CONTENT = 3,
			CONTENT_CONSUMPTION = 4,
			ACCESS_TITLE = 5,
			LIMITED_RESOURCE = 6,
		};

		ETicketSectionHeader_V2();
		ETicketSectionHeader_V2(const ETicketSectionHeader_V2& other);
		ETicketSectionHeader_V2(const u8* bytes, size_t len);

		bool operator==(const ETicketSectionHeader_V2& other) const;
		bool operator!=(const ETicketSectionHeader_V2& other) const;
		void operator=(const ETicketSectionHeader_V2& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		virtual void importBinary(const u8* bytes, size_t len);

		// variables
		virtual void clear();

		u32 getSectionOffset() const;
		void setSectionOffset(u32 offset);

		u32 getRecordSize() const;
		void setRecordSize(u32 size);

		u32 getSectionSize() const;
		void getSectionSize(u32 size);

		u16 getRecordNum() const;
		void setRecordNum(u16 record_num);

		SectionType getSectionType() const;
		void setSectionType(SectionType type);

	private:
		const std::string kModuleName = "ETICKET_SECTION_HEADER_V2";
#pragma pack (push, 1)
		struct sSectionHeader_v2
		{
		private:
			u32 section_offset_;
			u32 record_size_;
			u32 section_size_;
			u16 record_num_;
			u16 section_type_;
		public:
			u32 section_offset() const { return le_word(section_offset_); }
			void set_section_offset(u32 offset) { section_offset_ = le_word(offset); }

			u32 record_size() const { return le_word(record_size_); }
			void set_record_size(u32 size) { record_size_ = le_word(size); }

			u32 section_size() const { return le_word(section_size_); }
			void set_section_size(u32 size) { section_size_ = le_word(size); }

			u16 record_num() const { return le_hword(record_num_); }
			void set_record_num(u16 num) { record_num_ = le_hword(num); }

			u16 section_type() const { return le_hword(section_type_); }
			void set_section_type(u16 type) { section_type_ = le_hword(type); }
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		u32 mSectionOffset;
		u32 mRecordSize;
		u32 mSectionSize;
		u16 mRecordNum;
		SectionType mSectionType;

		// helpers
		bool isEqual(const ETicketSectionHeader_V2& other) const;
		void copyFrom(const ETicketSectionHeader_V2& other);

	};

}
