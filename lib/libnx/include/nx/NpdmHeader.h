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
		NpdmHeader(const u8* bytes, size_t len);

		bool operator==(const NpdmHeader& other) const;
		bool operator!=(const NpdmHeader& other) const;
		void operator=(const NpdmHeader& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes, size_t len);

		// variables
		void clear();
		size_t getNpdmSize() const;

		InstructionType getInstructionType() const;
		void setInstructionType(InstructionType type);

		ProcAddrSpaceType getProcAddressSpaceType() const;
		void setProcAddressSpaceType(ProcAddrSpaceType type);

		u8 getMainThreadPriority() const;
		void setMainThreadPriority(u8 priority);

		u8 getMainThreadCpuId() const;
		void setMainThreadCpuId(u8 cpu_id);

		u32 getVersion() const;
		void setVersion(u32 version);

		u32 getMainThreadStackSize() const;
		void setMainThreadStackSize(u32 size);

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
		static const u32 kMaxPriority = BIT(6) -1 ;
		static const size_t kNpdmAlignSize = 0x10;


#pragma pack (push, 1)
		struct sNpdmHeader
		{
		private:
			u8 signature_[4]; // be"META" 
			u8 reserved_0[8];
			u8 flags_;
			u8 reserved_1;
			u8 main_thread_priority_; // 0-63 inclusive
			u8 main_thread_cpu_id_;
			u8 reserved_2[8];
			u32 version_;
			u32 main_thread_stack_size_; // default 4096
			u8 name_[kNameMaxLen]; // important
			u8 product_code_[kProductCodeMaxLen]; // can be empty
			u8 reserved_3[48];
			// Access Control Info
			struct sNpdmSection
			{
			private:
				u32 offset_;
				u32 size_;
			public:
				u32 offset() const { return le_word(offset_); }
				void set_offset(u32 offset) { offset_ = le_word(offset); }

				u32 size() const { return le_word(size_); }
				void set_size(u32 size) { size_ = le_word(size); }
			} aci_, acid_;
		public:
			const char* signature() const { return (const char*)signature_; }
			void set_signature(const char* signature) { memcpy(signature_, signature, 4); }

			u8 flags() const { return flags_; }
			void set_flags(u8 flags) { flags_ = flags; }

			u8 main_thread_priority() const { return main_thread_priority_; }
			void set_main_thread_priority(u8 priority) { main_thread_priority_ = priority; }

			u8 main_thread_cpu_id() const { return main_thread_cpu_id_; }
			void set_main_thread_cpu_id(u8 cpu_id) { main_thread_cpu_id_ = cpu_id; }

			u32 version() const { return le_word(version_); }
			void set_version(u32 version) { version_ = le_word(version); }

			u32 main_thread_stack_size() const { return le_word(main_thread_stack_size_); }
			void set_main_thread_stack_size(u32 size) { main_thread_stack_size_ = le_word(size); }

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
		u8 mMainThreadPriority;
		u8 mMainThreadCpuId;
		u32 mVersion;
		u32 mMainThreadStackSize;
		std::string mName;
		std::string mProductCode;
		sSection mAciPos;
		sSection mAcidPos;

		void calculateOffsets();
		bool isEqual(const NpdmHeader& other) const;
		void copyFrom(const NpdmHeader& other);
	};
}

