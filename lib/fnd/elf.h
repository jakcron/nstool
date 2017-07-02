#pragma once
#include "types.h"

typedef byte_t Elf_Byte;
typedef word_t Elf32_Addr;
typedef word_t Elf32_Off;
typedef long_t Elf32_Sword; // lol "sword"
typedef word_t Elf32_Word;
typedef hword_t Elf32_Half;

enum
{
	EI_MAG0    = 0, // 0x7F
	EI_MAG1    = 1, // 'E'
	EI_MAG2    = 2, // 'L'
	EI_MAG3    = 3, // 'F'
	EI_CLASS   = 4, // File class
	EI_DATA    = 5, // Data encoding
	EI_VERSION = 6, // File version
	EI_PAD     = 7, // Start of padding bytes
	EI_NIDENT  = 16 // Size of e_ident[]
};

typedef struct
{
	unsigned char e_ident[EI_NIDENT]; // Identification bytes
	Elf32_Half    e_type;             // Object file type
	Elf32_Half    e_machine;          // Object architecture
	Elf32_Word    e_version;          // Object file version
	Elf32_Addr    e_entry;            // Object entry point
	Elf32_Off     e_phoff;            // Program header file offset
	Elf32_Off     e_shoff;            // Section header file offset
	Elf32_Word    e_flags;            // Processor-specific flags
	Elf32_Half    e_ehsize;           // ELF header size
	Elf32_Half    e_phentsize;        // Program header entry size
	Elf32_Half    e_phnum;            // Program header entries
	Elf32_Half    e_shentsize;        // Section header entry size
	Elf32_Half    e_shnum;            // Section header entries
	Elf32_Half    e_shstrndx;         // String table index
} Elf32_Ehdr;

typedef struct
{
	Elf32_Word p_type;   // Segment type
	Elf32_Off  p_offset; // File offset
	Elf32_Addr p_vaddr;  // Virtual address
	Elf32_Addr p_paddr;  // Physical address
	Elf32_Word p_filesz; // File image size
	Elf32_Word p_memsz;  // Memory image size
	Elf32_Word p_flags;  // Segment flags
	Elf32_Word p_align;  // Alignment value
} Elf32_Phdr;

typedef struct
{
	Elf32_Word sh_name;      // Name (index into section header string table section)
	Elf32_Word sh_type;      // Type
	Elf32_Word sh_flags;     // Flags
	Elf32_Addr sh_addr;      // Address
	Elf32_Off  sh_offset;    // File offset
	Elf32_Word sh_size;      // Section size
	Elf32_Word sh_link;      // Section header table index link
	Elf32_Word sh_info;      // Extra information
	Elf32_Word sh_addralign; // Address alignment
	Elf32_Word sh_entsize;   // Section entry size
} Elf32_Shdr;

typedef struct
{
	Elf32_Addr r_offset; // Offset of relocation
	Elf32_Word r_info;   // Symbol table index and type
} Elf32_Rel;

typedef struct
{
	Elf32_Word    st_name;  // Name - index into string table
	Elf32_Addr    st_value; // Symbol value
	Elf32_Word    st_size;  // Symbol size
	unsigned char st_info;  // Type and binding
	unsigned char st_other; // Visibility
	Elf32_Half    st_shndx; // Section header index
} Elf32_Sym;

enum
{
	ET_NONE = 0, // No file type
	ET_REL  = 1, // Relocatable file
	ET_EXEC = 2, // Executable file
	ET_DYN  = 3, // Shared object file
	ET_CORE = 4, // Core file
};

enum
{
	ET_ARM = 40 // ARM architecture
};

enum
{
	EV_NONE    = 0, // Invalid version
	EV_CURRENT = 1  // Current version
};

#define ELF_MAGIC "\177ELF"

enum
{
	ELFDATANONE = 0, // Invalid data encoding
	ELFDATA2LSB = 1, // Little endian
	ELFDATA2MSB = 2, // Big endian
};

enum
{
	PT_NULL    = 0, // Unused
	PT_LOAD    = 1, // Loadable segment
	PT_DYNAMIC = 2, // Dynamic linking information
	PT_INTERP  = 3, // Interpreter
	PT_NOTE    = 4, // Auxiliary information
	PT_SHLIB   = 5, // Reserved
	PT_PHDR    = 6  // Program header table
};

enum
{
	PF_R = 4, // Read flag
	PF_W = 2, // Write flag
	PF_X = 1, // Execute flag
	PF_OS_SHARED = 0x100000, // OS-specific
	PF_CTRSDK = 0x80000000, // Set in CTRSDK ELF Text segments
};

enum
{
	SHN_LORESERVE = 0xFF00,
	SHN_HIRESERVE = 0xFFFF
};

enum
{
	SHT_NULL = 0,            // Inactive
	SHT_PROGBITS = 1,        // Program defined information
	SHT_SYMTAB = 2,          // Symbol table section
	SHT_STRTAB = 3,          // String table section
	SHT_RELA = 4,            // Relocation section with addends
	SHT_HASH = 5,            // Symbol hash table section
	SHT_DYNAMIC = 6,         // Dynamic section
	SHT_NOTE = 7,            // Note section
	SHT_NOBITS = 8,          // No space section
	SHT_REL = 9,             // Relation section without addends
	SHT_SHLIB = 10,          // Reserved
	SHT_DYNSYM = 11,         // Dynamic symbol table section
	SHT_NUM = 12,            // Number of section types
	SHT_LOPROC = 0x70000000, // Reserved range for processor
	SHT_ARM_EXIDX = 0x70000001, // ARM exception index table
	SHT_HIPROC = 0x7fffffff, // Specific section header types
	SHT_LOUSER = 0x80000000, // Reserved range for application
	SHT_HIUSER = 0xffffffff  // Specific indexes
};

enum
{
	SHF_WRITE = 1,             // Writable section
	SHF_ALLOC = 2,             // Loadable section
	SHF_EXECINSTR = 4,         // Executable section
	SHF_MASKPROC = 0xf0000000, // Processor-specific
};

#define ELF32_R_SYM(i)    ((i) >> 8)
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s) << 8) + (unsigned char)(t))

enum
{
	R_ARM_NONE = 0,
	R_ARM_PC24 = 1,
	R_ARM_ABS32 = 2,
	R_ARM_REL32 = 3,
	R_ARM_THM_CALL = 10,
	R_ARM_PLT32 = 27,
	R_ARM_CALL = 28,
	R_ARM_JUMP24 = 29,
	R_ARM_TARGET1 = 38,
	R_ARM_TARGET2 = 41,
	R_ARM_PREL31 = 42,
	R_ARM_THM_JUMP11 = 102,
	R_ARM_THM_JUMP8 = 103
};

// Symbol scope
enum
{
	STB_LOCAL = 0,
	STB_GLOBAL = 1,
	STB_WEAK = 2
};

#define ELF32_ST_BIND(i)   (((unsigned char)(i)) >> 4)
#define ELF32_ST_TYPE(val) ((val) & 0xf)

// Symbol type
enum
{
	STT_NOTYPE = 0,
	STT_OBJECT = 1,
	STT_FUNC = 2
};

// Symbol visibility
enum
{
	STV_DEFAULT = 0,
	STV_INTERNAL = 1,
	STV_HIDDEN = 2,
	STV_PROTECTED = 3
};
