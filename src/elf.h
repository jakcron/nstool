#pragma once
#include "types.h"

namespace nstool
{
	namespace elf
	{
		/* These constants are for the segment types stored in the image headers */
		enum SegmentType
		{
			PT_NULL = 0,
			PT_LOAD = 1,
			PT_DYNAMIC = 2,
			PT_INTERP = 3,
			PT_NOTE = 4,
			PT_SHLIB = 5,
			PT_PHDR = 6,
			PT_TLS = 7,               /* Thread local storage segment */
			PT_LOOS = 0x60000000,      /* OS-specific */
			PT_HIOS = 0x6fffffff,      /* OS-specific */
			PT_LOPROC = 0x70000000,
			PT_HIPROC = 0x7fffffff
		};

		/* These constants define the different elf file types */
		enum ElfType
		{
			ET_NONE = 0,
			ET_REL = 1,
			ET_EXEC = 2,
			ET_DYN = 3,
			ET_CORE = 4,
			ET_LOPROC = 0xff00,
			ET_HIPROC = 0xffff
		};

		/* This is the info that is needed to parse the dynamic section of the file */
		enum DynamicSectionType
		{
			DT_NULL = 0,
			DT_NEEDED = 1,
			DT_PLTRELSZ = 2,
			DT_PLTGOT = 3,
			DT_HASH = 4,
			DT_STRTAB = 5,
			DT_SYMTAB = 6,
			DT_RELA = 7,
			DT_RELASZ = 8,
			DT_RELAENT = 9,
			DT_STRSZ = 10,
			DT_SYMENT = 11,
			DT_INIT = 12,
			DT_FINI = 13,
			DT_SONAME = 14,
			DT_RPATH = 15,
			DT_SYMBOLIC = 16,
			DT_REL = 17,
			DT_RELSZ = 18,
			DT_RELENT = 19,
			DT_PLTREL = 20,
			DT_DEBUG = 21,
			DT_TEXTREL = 22,
			DT_JMPREL = 23,
			DT_ENCODING = 32,
			OLD_DT_LOOS = 0x60000000,
			DT_LOOS = 0x6000000d,
			DT_HIOS = 0x6ffff000,
			DT_VALRNGLO = 0x6ffffd00,
			DT_VALRNGHI = 0x6ffffdff,
			DT_ADDRRNGLO = 0x6ffffe00,
			DT_ADDRRNGHI = 0x6ffffeff,
			DT_VERSYM = 0x6ffffff0,
			DT_RELACOUNT = 0x6ffffff9,
			DT_RELCOUNT = 0x6ffffffa,
			DT_FLAGS_1 = 0x6ffffffb,
			DT_VERDEF = 0x6ffffffc,
			DT_VERDEFNUM = 0x6ffffffd,
			DT_VERNEED = 0x6ffffffe,
			DT_VERNEEDNUM = 0x6fffffff,
			OLD_DT_HIOS = 0x6fffffff,
			DT_LOPROC = 0x70000000,
			DT_HIPROC = 0x7fffffff
		};

		/* This info is needed when parsing the symbol table */
		enum SymbolBinding
		{
			STB_LOCAL = 0,
			STB_GLOBAL = 1,
			STB_WEAK = 2,
			STB_LOOS = 10,
			STB_HIOS = 12,
			STB_LOPROC,
			STB_HIPROC = 0xf
		};

		enum SymbolType
		{
			STT_NOTYPE = 0,
			STT_OBJECT = 1,
			STT_FUNC = 2,
			STT_SECTION = 3,
			STT_FILE = 4,
			STT_COMMON = 5,
			STT_TLS = 6,
			STT_LOOS = 10,
			STT_HIOS = 12,
			STT_LOPROC,
			STT_HIPROC = 0xf
		};

		/* These constants define the permissions on sections in the program
		header, p_flags. */
		enum PermissionFlag
		{
			PF_R = 0x4,
			PF_W = 0x2,
			PF_X = 0x1
		};

		/* sh_type */
		enum SectionHeaderType
		{
			SHT_NULL = 0,
			SHT_PROGBITS = 1,
			SHT_SYMTAB = 2,
			SHT_STRTAB = 3,
			SHT_RELA = 4,
			SHT_HASH = 5,
			SHT_DYNAMIC = 6,
			SHT_NOTE = 7,
			SHT_NOBITS = 8,
			SHT_REL	 = 9,
			SHT_SHLIB = 10,
			SHT_DYNSYM = 11,
			SHT_NUM	 = 12,
			SHT_LOPROC = 0x70000000,
			SHT_HIPROC = 0x7fffffff,
			SHT_LOUSER = 0x80000000,
			SHT_HIUSER = 0xffffffff
		};
		
		/* sh_flags */
		enum SectionHeaderFlag
		{
			SHF_WRITE = 0x1,
			SHF_ALLOC = 0x2,
			SHF_EXECINSTR = 0x4,
			SHF_RELA_LIVEPATCH = 0x00100000,
			SHF_RO_AFTER_INIT = 0x00200000,
			SHF_MASKPROC = 0xf0000000
		};
		
		/* special section indexes */
		enum SpecialSectionIndex
		{
			SHN_UNDEF = 0,
			SHN_LORESERVE = 0xff00,
			SHN_LOPROC = 0xff00,
			SHN_HIPROC = 0xff1f,
			SHN_LOOS = 0xff20,
			SHN_HIOS = 0xff3f,
			SHN_ABS = 0xfff1,
			SHN_COMMON = 0xfff2,
			SHN_HIRESERVE = 0xffff
		};

		enum ElfIdentIndex
		{
			EI_MAG0 = 0,		/* e_ident[] indexes */
			EI_MAG1 = 1,
			EI_MAG2 = 2,
			EI_MAG3 = 3,
			EI_CLASS = 4,
			EI_DATA = 5,
			EI_VERSION = 6,
			EI_OSABI = 7,
			EI_PAD = 8
		};

		enum ElfClass
		{
			ELFCLASSNONE = 0,		/* EI_CLASS */
			ELFCLASS32 = 1,
			ELFCLASS64 = 2,
			ELFCLASSNUM = 3
		};
		
		enum ElfData
		{
			ELFDATANONE = 0,		/* e_ident[EI_DATA] */
			ELFDATA2LSB = 1,
			ELFDATA2MSB = 2
		};
		
		enum ElfVersion
		{
			EV_NONE = 0,		/* e_version, EI_VERSION */
			EV_CURRENT = 1,
			EV_NUM = 2,
		};
		
		enum ElfOsAbi
		{
			ELFOSABI_NONE = 0,
			ELFOSABI_LINUX =3
		};
		

		/*
		* Notes used in ET_CORE. Architectures export some of the arch register sets
		* using the corresponding note types via the PTRACE_GETREGSET and
		* PTRACE_SETREGSET requests.
		*/
		enum NoteType
		{
			NT_PRSTATUS = 1,
			NT_PRFPREG = 2,
			NT_PRPSINFO = 3,
			NT_TASKSTRUCT = 4,
			NT_AUXV = 6,
		/*
		* Note to userspace developers: size of NT_SIGINFO note may increase
		* in the future to accomodate more fields, don't assume it is fixed!
		*/
			NT_SIGINFO = 0x53494749,
			NT_FILE = 0x46494c45,
			NT_PRXFPREG = 0x46e62b7f,      /* copied from gdb5.1/include/elf/common.h */
			NT_PPC_VMX = 0x100,		/* PowerPC Altivec/VMX registers */
			NT_PPC_SPE = 0x101,		/* PowerPC SPE/EVR registers */
			NT_PPC_VSX = 0x102,		/* PowerPC VSX registers */
			NT_PPC_TAR = 0x103,		/* Target Address Register */
			NT_PPC_PPR = 0x104,		/* Program Priority Register */
			NT_PPC_DSCR = 0x105,		/* Data Stream Control Register */
			NT_PPC_EBB = 0x106,		/* Event Based Branch Registers */
			NT_PPC_PMU = 0x107,		/* Performance Monitor Registers */
			NT_PPC_TM_CGPR = 0x108,		/* TM checkpointed GPR Registers */
			NT_PPC_TM_CFPR = 0x109,		/* TM checkpointed FPR Registers */
			NT_PPC_TM_CVMX = 0x10a,		/* TM checkpointed VMX Registers */
			NT_PPC_TM_CVSX = 0x10b,		/* TM checkpointed VSX Registers */
			NT_PPC_TM_SPR = 0x10c,		/* TM Special Purpose Registers */
			NT_PPC_TM_CTAR = 0x10d,		/* TM checkpointed Target Address Register */
			NT_PPC_TM_CPPR = 0x10e,		/* TM checkpointed Program Priority Register */
			NT_PPC_TM_CDSCR = 0x10f,		/* TM checkpointed Data Stream Control Register */
			NT_PPC_PKEY = 0x110,		/* Memory Protection Keys registers */
			NT_386_TLS = 0x200,		/* i386 TLS slots (struct user_desc) */
			NT_386_IOPERM = 0x201,		/* x86 io permission bitmap (1=deny) */
			NT_X86_XSTATE = 0x202,		/* x86 extended state using xsave */
			NT_S390_HIGH_GPRS = 0x300,	/* s390 upper register halves */
			NT_S390_TIMER = 0x301,		/* s390 timer register */
			NT_S390_TODCMP = 0x302,		/* s390 TOD clock comparator register */
			NT_S390_TODPREG = 0x303,		/* s390 TOD programmable register */
			NT_S390_CTRS = 0x304,		/* s390 control registers */
			NT_S390_PREFIX = 0x305,		/* s390 prefix register */
			NT_S390_LAST_BREAK = 0x306,	/* s390 breaking event address */
			NT_S390_SYSTEM_CALL = 0x307,	/* s390 system call restart data */
			NT_S390_TDB = 0x308,		/* s390 transaction diagnostic block */
			NT_S390_VXRS_LOW = 0x309,	/* s390 vector registers 0-15 upper half */
			NT_S390_VXRS_HIGH = 0x30a,	/* s390 vector registers 16-31 */
			NT_S390_GS_CB = 0x30b,		/* s390 guarded storage registers */
			NT_S390_GS_BC = 0x30c,		/* s390 guarded storage broadcast control block */
			NT_S390_RI_CB = 0x30d,		/* s390 runtime instrumentation */
			NT_ARM_VFP = 0x400,		/* ARM VFP/NEON registers */
			NT_ARM_TLS = 0x401,		/* ARM TLS register */
			NT_ARM_HW_BREAK = 0x402,		/* ARM hardware breakpoint registers */
			NT_ARM_HW_WATCH = 0x403,		/* ARM hardware watchpoint registers */
			NT_ARM_SYSTEM_CALL = 0x404,	/* ARM system call number */
			NT_ARM_SVE = 0x405,		/* ARM Scalable Vector Extension registers */
			NT_ARC_V2 = 0x600,		/* ARCv2 accumulator/extra registers */
			NT_VMCOREDD = 0x700,		/* Vmcore Device Dump Note */
			NT_MIPS_DSP = 0x800,		/* MIPS DSP ASE registers */
			NT_MIPS_FP_MODE = 0x801,		/* MIPS floating-point mode */
		};
		
		static const size_t kEIdentSize = 0x10;
		static const byte_t kElfMagic[sizeof(uint32_t)] = {0x7f, 'E', 'L', 'F'};


		inline byte_t get_elf_st_bind(byte_t st_info) { return st_info >> 4; }
		inline byte_t get_elf_st_type(byte_t st_info) { return st_info & 0xf; }
		inline byte_t get_elf_st_info(byte_t st_bind, byte_t st_type) { return (st_type & 0xf) | ((st_bind & 0xf) << 4);}

		/* The following are used with relocations */
		#define ELF32_R_SYM(x) ((x) >> 8)
		#define ELF32_R_TYPE(x) ((x) & 0xff)

		#define ELF64_R_SYM(i)			((i) >> 32)
		#define ELF64_R_TYPE(i)			((i) & 0xffffffff)
	}

	struct Elf32_Dyn
	{
		int32_t d_tag;
		union{
			int32_t	d_val;
			uint32_t	d_ptr;
		} d_un;
	};

	struct Elf64_Dyn
	{
		int64_t d_tag;		/* entry tag value */
		union {
			uint64_t d_val;
			uint64_t d_ptr;
		} d_un;
	};

	struct Elf32_Rel
	{
		uint32_t	r_offset;
		uint32_t	r_info;
	};

	struct Elf64_Rel
	{
		uint64_t r_offset;	/* Location at which to apply the action */
		uint64_t r_info;	/* index and type of relocation */
	};

	struct Elf32_Rela
	{
		uint32_t	r_offset;
		uint32_t	r_info;
		int32_t	r_addend;
	};

	struct Elf64_Rela
	{
		uint64_t r_offset;	/* Location at which to apply the action */
		uint64_t r_info;	/* index and type of relocation */
		int64_t r_addend;	/* Constant addend used to compute value */
	};

	struct Elf32_Sym
	{
		uint32_t	st_name;
		uint32_t	st_value;
		uint32_t	st_size;
		byte_t	st_info;
		byte_t	st_other;
		uint16_t	st_shndx;
	};

	struct Elf64_Sym
	{
		uint32_t st_name;		/* Symbol name, index in string tbl */
		byte_t	st_info;	/* Type and binding attributes */
		byte_t	st_other;	/* No defined meaning, 0 */
		uint16_t st_shndx;		/* Associated section index */
		uint64_t st_value;		/* Value of the symbol */
		uint64_t st_size;		/* Associated symbol size */
	};

	struct Elf32_Ehdr
	{
		byte_t	e_ident[elf::kEIdentSize];
		uint16_t	e_type;
		uint16_t	e_machine;
		uint32_t	e_version;
		uint32_t	e_entry;  /* Entry point */
		uint32_t	e_phoff;
		uint32_t	e_shoff;
		uint32_t	e_flags;
		uint16_t	e_ehsize;
		uint16_t	e_phentsize;
		uint16_t	e_phnum;
		uint16_t	e_shentsize;
		uint16_t	e_shnum;
		uint16_t	e_shstrndx;
	};

	struct Elf64_Ehdr
	{
		byte_t	e_ident[elf::kEIdentSize];	/* ELF "magic number" */
		uint16_t e_type;
		uint16_t e_machine;
		uint32_t e_version;
		uint64_t e_entry;		/* Entry point virtual address */
		uint64_t e_phoff;		/* Program header table file offset */
		uint64_t e_shoff;		/* Section header table file offset */
		uint32_t e_flags;
		uint16_t e_ehsize;
		uint16_t e_phentsize;
		uint16_t e_phnum;
		uint16_t e_shentsize;
		uint16_t e_shnum;
		uint16_t e_shstrndx;
	};

	struct Elf32_Phdr
	{
		uint32_t	p_type;
		uint32_t	p_offset;
		uint32_t	p_vaddr;
		uint32_t	p_paddr;
		uint32_t	p_filesz;
		uint32_t	p_memsz;
		uint32_t	p_flags;
		uint32_t	p_align;
	};

	struct Elf64_Phdr
	{
		uint32_t p_type;
		uint32_t p_flags;
		uint64_t p_offset;		/* Segment file offset */
		uint64_t p_vaddr;		/* Segment virtual address */
		uint64_t p_paddr;		/* Segment physical address */
		uint64_t p_filesz;		/* Segment size in file */
		uint64_t p_memsz;		/* Segment size in memory */
		uint64_t p_align;		/* Segment alignment, file & memory */
	};

	struct Elf32_Shdr
	{
		uint32_t	sh_name;
		uint32_t	sh_type;
		uint32_t	sh_flags;
		uint32_t	sh_addr;
		uint32_t	sh_offset;
		uint32_t	sh_size;
		uint32_t	sh_link;
		uint32_t	sh_info;
		uint32_t	sh_addralign;
		uint32_t	sh_entsize;
	};

	struct Elf64_Shdr
	{
		uint32_t sh_name;		/* Section name, index in string tbl */
		uint32_t sh_type;		/* Type of section */
		uint64_t sh_flags;		/* Miscellaneous section attributes */
		uint64_t sh_addr;		/* Section virtual addr at execution */
		uint64_t sh_offset;		/* Section file offset */
		uint64_t sh_size;		/* Size of section in bytes */
		uint32_t sh_link;		/* Index of another section */
		uint32_t sh_info;		/* Additional section information */
		uint64_t sh_addralign;	/* Section alignment */
		uint64_t sh_entsize;	/* Entry size if section holds table */
	};

	/* Note header in a PT_NOTE section */
	struct Elf32_Nhdr
	{
		uint32_t	n_namesz;	/* Name size */
		uint32_t	n_descsz;	/* Content size */
		uint32_t	n_type;		/* Content type */
	};

	/* Note header in a PT_NOTE section */
	struct Elf64_Nhdr
	{
		uint32_t n_namesz;	/* Name size */
		uint32_t n_descsz;	/* Content size */
		uint32_t n_type;	/* Content type */
	};
}