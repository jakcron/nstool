#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class NpdmHeader :
		public fnd::ISerialiseableBinary
	{
	public:
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

		struct sSection
		{
			size_t offset;
			size_t size;

			void operator=(const sSection& other)
			{
				offset = other.offset;
				size = other.size;
			}

			bool operator==(const sSection& other) const
			{
				return (offset == other.offset) \
					&& (size == other.size);
			}

			bool operator!=(const sSection& other) const
			{
				return !operator==(other);
			}
		};

		NpdmHeader();
		NpdmHeader(const NpdmHeader& other);
		NpdmHeader(const byte_t* bytes, size_t len);

		bool operator==(const NpdmHeader& other) const;
		bool operator!=(const NpdmHeader& other) const;
		void operator=(const NpdmHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();
		size_t getNpdmSize() const;

		InstructionType getInstructionType() const;
		void setInstructionType(InstructionType type);

		ProcAddrSpaceType getProcAddressSpaceType() const;
		void setProcAddressSpaceType(ProcAddrSpaceType type);

		byte_t getMainThreadPriority() const;
		void setMainThreadPriority(byte_t priority);

		byte_t getMainThreadCpuId() const;
		void setMainThreadCpuId(byte_t cpu_id);

		uint32_t getVersion() const;
		void setVersion(uint32_t version);

		uint32_t getMainThreadStackSize() const;
		void setMainThreadStackSize(uint32_t size);

		const std::string& getName() const;
		void setName(const std::string& name);

		const std::string& getProductCode() const;
		void setProductCode(const std::string& product_code);

		const sSection& getAciPos() const;
		void setAciSize(size_t size);

		const sSection& getAcidPos() const;
		void setAcidSize(size_t size);
	private:
		const std::string kModuleName = "NPDM_HEADER";
		const std::string kNpdmStructSig = "META";
		static const size_t kNameMaxLen = 0x10;
		static const size_t kProductCodeMaxLen = 0x10;
		static const uint32_t kMaxPriority = BIT(6) -1 ;
		static const size_t kNpdmAlignSize = 0x10;


#pragma pack (push, 1)
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
			byte_t name_[kNameMaxLen]; // important
			byte_t product_code_[kProductCodeMaxLen]; // can be empty
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
			void set_name(const char* name) { strncpy((char*)name_, name, kNameMaxLen); }

			const char* product_code() const { return (const char*)product_code_; }
			void set_product_code(const char* product_code) { strncpy((char*)product_code_, product_code, kProductCodeMaxLen); }

			const sNpdmSection& aci() const { return aci_; }
			sNpdmSection& aci() { return aci_; }

			const sNpdmSection& acid() const { return acid_; }
			sNpdmSection& acid() { return acid_; }
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		InstructionType mInstructionType;
		ProcAddrSpaceType mProcAddressSpaceType;
		byte_t mMainThreadPriority;
		byte_t mMainThreadCpuId;
		uint32_t mVersion;
		uint32_t mMainThreadStackSize;
		std::string mName;
		std::string mProductCode;
		sSection mAciPos;
		sSection mAcidPos;

		void calculateOffsets();
		bool isEqual(const NpdmHeader& other) const;
		void copyFrom(const NpdmHeader& other);
	};
}

