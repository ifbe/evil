#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define s8 signed char
#define s16 signed short
#define s32 signed int
#define s64 signed long long
int hexstr2u32(void* str, void* dat);
int disasm_arm64_all(void* buf, int len, u64 rip);
int disasm_x8664_all(void* buf, int len, u64 rip);


#define SHT_NULL                    0    // Unused
#define SHT_PROGBITS                1    // Program data
#define SHT_SYMTAB                  2    // Symbol table
#define SHT_STRTAB                  3    // String table
#define SHT_RELA                    4    // Relocation entries with addends
#define SHT_HASH                    5    // Symbol hash table
#define SHT_DYNAMIC                 6    // Dynamic linking information
#define SHT_NOTE                    7    // Notes
#define SHT_NOBITS                  8    // bss
#define SHT_REL                     9    // Relocation entries, no addends
#define SHT_SHLIB                  10    // Reserved
#define SHT_DYNSYM                 11    // Dynamic linker symbol table
#define SHT_INIT_ARRAY             14    // Array of constructors
#define SHT_FINI_ARRAY             15    // Array of destructors
#define SHT_PREINIT_ARRAY          16    // Array of pre-constructors
#define SHT_GROUP                  17    // Section group
#define SHT_SYMTAB_SHNDX           18    // Extended section indeces
#define SHT_NUM                    19    // Number of defined types. 
#define SHT_LOOS           0x60000000    // Start OS-specific. 
#define SHT_GNU_ATTRIBUTES 0x6ffffff5    // Object attributes. 
#define SHT_GNU_HASH       0x6ffffff6    // GNU-style hash table. 
#define SHT_GNU_LIBLIST    0x6ffffff7    // Prelink library list
#define SHT_CHECKSUM       0x6ffffff8    // Checksum for DSO content. 
#define SHT_LOSUNW         0x6ffffffa    // Sun-specific low bound. 
#define SHT_SUNW_move      0x6ffffffa
#define SHT_SUNW_COMDAT    0x6ffffffb
#define SHT_SUNW_syminfo   0x6ffffffc
#define SHT_GNU_verdef     0x6ffffffd    // Version definition section. 
#define SHT_GNU_verneed    0x6ffffffe    // Version needs section. 
#define SHT_GNU_versym     0x6fffffff    // Version symbol table. 
#define SHT_HISUNW         0x6fffffff    // Sun-specific high bound. 
#define SHT_HIOS           0x6fffffff    // End OS-specific type
#define SHT_LOPROC         0x70000000    // Start of processor-specific
#define SHT_HIPROC         0x7fffffff    // End of processor-specific
#define SHT_LOUSER         0x80000000    // Start of application-specific
#define SHT_HIUSER         0x8fffffff    // End of application-specific


#define EI_NIDENT	16
struct elf32_hdr{
unsigned char e_ident[EI_NIDENT];
u16 e_type;
u16 e_machine;
u32 e_version;
u32 e_entry;  /* Entry point */
u32 e_phoff;
u32 e_shoff;
u32 e_flags;
u16 e_ehsize;
u16 e_phentsize;
u16 e_phnum;
u16 e_shentsize;
u16 e_shnum;
u16 e_shstrndx;
};
struct elf64_hdr{
unsigned char e_ident[EI_NIDENT];	/* ELF "magic number" */
u16 e_type;
u16 e_machine;
u32 e_version;
u64 e_entry;		/* Entry point virtual address */
u64 e_phoff;		/* Program header table file offset */
u64 e_shoff;		/* Section header table file offset */
u32 e_flags;
u16 e_ehsize;
u16 e_phentsize;
u16 e_phnum;
u16 e_shentsize;
u16 e_shnum;
u16 e_shstrndx;
};
struct elf32_phdr{
u32 p_type;
u32 p_offset;
u32 p_vaddr;
u32 p_paddr;
u32 p_filesz;
u32 p_memsz;
u32 p_flags;
u32 p_align;
};
struct elf64_phdr {
u32 p_type;
u32 p_flags;
u64 p_offset;		/* Segment file offset */
u64 p_vaddr;		/* Segment virtual address */
u64 p_paddr;		/* Segment physical address */
u64 p_filesz;		/* Segment size in file */
u64 p_memsz;		/* Segment size in memory */
u64 p_align;		/* Segment alignment, file & memory */
};
struct elf32_shdr {
u32 sh_name;
u32 sh_type;
u32 sh_flags;
u32 sh_addr;
u32 sh_offset;
u32 sh_size;
u32 sh_link;
u32 sh_info;
u32 sh_addralign;
u32 sh_entsize;
};
struct elf64_shdr {
u32 sh_name;		/* Section name, index in string tbl */
u32 sh_type;		/* Type of section */
u64 sh_flags;		/* Miscellaneous section attributes */
u64 sh_addr;		/* Section virtual addr at execution */
u64 sh_offset;		/* Section file offset */
u64 sh_size;		/* Size of section in bytes */
u32 sh_link;		/* Index of another section */
u32 sh_info;		/* Additional section information */
u64 sh_addralign;	/* Section alignment */
u64 sh_entsize;		/* Entry size if section holds table */
};




void disasm_elf64_program(void* buf)
{
	struct elf64_hdr* h = buf;
	struct elf64_phdr* ph = buf + h->e_phoff;
	printf("ph@[%llx,?):\n", h->e_phoff);
	printf(
	"type=%x\n"
	"flags=%x\n"
	"offset=%llx\n"
	"vaddr=%llx\n"
	"paddr=%llx\n"
	"filesz=%llx\n"
	"memsz=%llx\n"
	"align=%llx\n"
	"\n",
	ph->p_type,
	ph->p_flags,
	ph->p_offset,
	ph->p_vaddr,
	ph->p_paddr,
	ph->p_filesz,
	ph->p_memsz,
	ph->p_align
	);

	switch(h->e_machine){
	case 0x3e:disasm_x8664_all(buf+ph->p_offset, ph->p_filesz, ph->p_vaddr);break;
	case 0xb7:disasm_arm64_all(buf+ph->p_offset, ph->p_filesz, ph->p_vaddr);break;
	}
}
void disasm_elf64_section(void* buf)
{
	int j;
	struct elf64_hdr* h = buf;
	struct elf64_shdr* sh = buf + h->e_shoff;
	void* str = buf+sh[h->e_shstrndx].sh_offset;

	printf("sh@[%llx,?),str@[%x,?):\n", h->e_shoff, sh[h->e_shstrndx].sh_offset);
	printf("name  type  flag  addr  offs  size  link  info  align entsz\n");
	for(j=0;j<h->e_shnum;j++){
		printf("%-6x%-6x%-6llx%-6llx%-6llx%-6llx%-6x%-6x%-6llx%-6llx%.16s\n",
		sh[j].sh_name,
		sh[j].sh_type,
		sh[j].sh_flags,
		sh[j].sh_addr,
		sh[j].sh_offset,
		sh[j].sh_size,
		sh[j].sh_link,
		sh[j].sh_info,
		sh[j].sh_addralign,
		sh[j].sh_entsize,
		str + sh[j].sh_name
		);
		if(0 != strncmp(str + sh[j].sh_name, ".text", 5))continue;

		switch(h->e_machine){
		case 0x3e:disasm_x8664_all(buf+sh[j].sh_offset, sh[j].sh_size, 0);break;
		case 0xb7:disasm_arm64_all(buf+sh[j].sh_offset, sh[j].sh_size, 0);break;
		}
	}//for
}
void disasm_elf64(void* buf,int len)
{
//----------------0.header----------------
	struct elf64_hdr* h = buf;
	if(2 != h->e_ident[4])return;	//not 64bit

	printf(
	"head@[0,?):\n"
	"type=%x\n"
	"machine=%x\n"
	"version=%x\n"
	"entry=%llx\n"
	"phoff=%llx\n"
	"shoff=%llx\n"
	"flag=%x\n"
	"ehsize=%x\n"
	"phentsize=%x\n"
	"phnum=%x\n"
	"shentsize=%x\n"
	"shnum=%x\n"
	"shstrndx=%x\n"
	"\n",
	h->e_type,
	h->e_machine,
	h->e_version,
	h->e_entry,
	h->e_phoff,
	h->e_shoff,
	h->e_flags,
	h->e_ehsize,
	h->e_phentsize,
	h->e_phnum,
	h->e_shentsize,
	h->e_shnum,
	h->e_shstrndx
	);

	if(h->e_phoff)disasm_elf64_program(buf);
	if(h->e_shoff)disasm_elf64_section(buf);
}
int check_elf(u8* addr)
{
	u32 temp=*(u32*)addr;
	if(temp==0x464c457f)return 0x666c65;
	return 0;
}
