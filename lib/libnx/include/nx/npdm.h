#pragma once
#include <string>
#include <fnd/types.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	namespace npdm
	{
		const std::string kNpdmStructSig = "META";
		static const size_t kNameMaxLen = 0x10;
		static const size_t kProductCodeMaxLen = 0x10;
		static const uint32_t kMaxPriority = BIT(6) -1 ;
		static const size_t kNpdmAlignSize = 0x10;

		enum InstructionType
		{
			INSTR_32BIT,
			INSTR_64BIT,
		};

		enum ProcAddrSpaceType
		{
			ADDR_SPACE_64BIT = 1,
			ADDR_SPACE_32BIT,
			ADDR_SPACE_32BIT_NO_RESERVED,
		};
	}
#pragma pack(push,1)

	struct sNpdmHeader
	{
	private:
		byte_t signature_[4]; // be"META" 
		byte_t reserved_0[8];
		byte_t flags_;
		byte_t reserved_1;
		byte_t main_thread_priority_; // 0-63 inclusive
		byte_t main_thread_cpu_id_;
		byte_t reserved_2[8];
		uint32_t version_;
		uint32_t main_thread_stack_size_; // default 4096
		byte_t name_[npdm::kNameMaxLen]; // important
		byte_t product_code_[npdm::kProductCodeMaxLen]; // can be empty
		byte_t reserved_3[48];
		// Access Control Info
		struct sNpdmSection
		{
		private:
			uint32_t offset_;
			uint32_t size_;
		public:
			uint32_t offset() const { return le_word(offset_); }
			void set_offset(uint32_t offset) { offset_ = le_word(offset); }

			uint32_t size() const { return le_word(size_); }
			void set_size(uint32_t size) { size_ = le_word(size); }
		} aci_, acid_;
	public:
		const char* signature() const { return (const char*)signature_; }
		void set_signature(const char* signature) { memcpy(signature_, signature, 4); }

		byte_t flags() const { return flags_; }
		void set_flags(byte_t flags) { flags_ = flags; }

		byte_t main_thread_priority() const { return main_thread_priority_; }
		void set_main_thread_priority(byte_t priority) { main_thread_priority_ = priority; }

		byte_t main_thread_cpu_id() const { return main_thread_cpu_id_; }
		void set_main_thread_cpu_id(byte_t cpu_id) { main_thread_cpu_id_ = cpu_id; }

		uint32_t version() const { return le_word(version_); }
		void set_version(uint32_t version) { version_ = le_word(version); }

		uint32_t main_thread_stack_size() const { return le_word(main_thread_stack_size_); }
		void set_main_thread_stack_size(uint32_t size) { main_thread_stack_size_ = le_word(size); }

		const char* name() const { return (const char*)name_; }
		void set_name(const char* name) { strncpy((char*)name_, name, npdm::kNameMaxLen); }

		const char* product_code() const { return (const char*)product_code_; }
		void set_product_code(const char* product_code) { strncpy((char*)product_code_, product_code, npdm::kProductCodeMaxLen); }

		const sNpdmSection& aci() const { return aci_; }
		sNpdmSection& aci() { return aci_; }

		const sNpdmSection& acid() const { return acid_; }
		sNpdmSection& acid() { return acid_; }
	};

#pragma pack(pop)
}