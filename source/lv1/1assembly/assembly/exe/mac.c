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




//magic
#define MH_MAGIC    0xfeedface  /* the mach magic number */
#define MH_CIGAM    0xcefaedfe  /* NXSwapInt(MH_MAGIC) */
#define MH_MAGIC_64 0xfeedfacf /* the 64-bit mach magic number */
#define MH_CIGAM_64 0xcffaedfe /* NXSwapInt(MH_MAGIC_64) */
//
#define CPUTYPE_X8664 0x01000007
#define CPUTYPE_ARM64 0x0100000c

//
#define	LC_SEGMENT	0x1	/* segment of this file to be mapped */
#define	LC_SYMTAB	0x2	/* link-edit stab symbol table info */
#define	LC_SYMSEG	0x3	/* link-edit gdb symbol table info (obsolete) */
#define	LC_THREAD	0x4	/* thread */
#define	LC_UNIXTHREAD	0x5	/* unix thread (includes a stack) */
#define	LC_LOADFVMLIB	0x6	/* load a specified fixed VM shared library */
#define	LC_IDFVMLIB	0x7	/* fixed VM shared library identification */
#define	LC_IDENT	0x8	/* object identification info (obsolete) */
#define LC_FVMFILE	0x9	/* fixed VM file inclusion (internal use) */
#define LC_PREPAGE      0xa     /* prepage command (internal use) */
#define	LC_DYSYMTAB	0xb	/* dynamic link-edit symbol table info */
#define	LC_LOAD_DYLIB	0xc	/* load a dynamically linked shared library */
#define	LC_ID_DYLIB	0xd	/* dynamically linked shared lib ident */
#define LC_LOAD_DYLINKER 0xe	/* load a dynamic linker */
#define LC_ID_DYLINKER	0xf	/* dynamic linker identification */
#define	LC_PREBOUND_DYLIB 0x10	/* modules prebound for a dynamically *//*  linked shared library */
#define	LC_ROUTINES	0x11	/* image routines */
#define	LC_SUB_FRAMEWORK 0x12	/* sub framework */
#define	LC_SUB_UMBRELLA 0x13	/* sub umbrella */
#define	LC_SUB_CLIENT	0x14	/* sub client */
#define	LC_SUB_LIBRARY  0x15	/* sub library */
#define	LC_TWOLEVEL_HINTS 0x16	/* two-level namespace lookup hints */
#define	LC_PREBIND_CKSUM  0x17	/* prebind checksum */
#define	LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)  /* (all symbols are weak imported) */
#define	LC_SEGMENT_64	0x19	/* 64-bit segment of this file to be mapped */
#define	LC_ROUTINES_64	0x1a	/* 64-bit image routines */
#define LC_UUID		0x1b	/* the uuid */
#define LC_RPATH       (0x1c | LC_REQ_DYLD)    /* runpath additions */
#define LC_CODE_SIGNATURE 0x1d	/* local of code signature */
#define LC_SEGMENT_SPLIT_INFO 0x1e /* local of info to split segments */
#define LC_REEXPORT_DYLIB (0x1f | LC_REQ_DYLD) /* load and re-export dylib */
#define	LC_LAZY_LOAD_DYLIB 0x20	/* delay load of dylib until first use */
#define	LC_ENCRYPTION_INFO 0x21	/* encrypted segment information */
#define	LC_DYLD_INFO 	0x22	/* compressed dyld information */
#define	LC_DYLD_INFO_ONLY (0x22|LC_REQ_DYLD)	/* compressed dyld information only */

//
struct mach_header_64 {
u32 magic;      /* mach magic number identifier */
u32 cputype;    /* cpu specifier */
u32 cpusubtype; /* machine specifier */
u32 filetype;   /* type of file */
u32 ncmds;      /* number of load commands */
u32 sizeofcmds; /* the size of all the load commands */
u32 flags;      /* flags */
u32 reserved;   /* reserved */
};
struct command_19
{
u32 cmd;
u32 cmdsize;
char segname[16];
u64 vmaddr;
u64 vmsize;
u64 fileoff;
u64 filesize;
u32 maxprot;
u32 initprot;
u32 nsects;
u32 flags;
};
struct section_64 { /* for 64-bit architectures */
char sec_name[16];	/* name of this section */
char seg_name[16];	/* segment this section goes in */
u64 addr;		/* memory address of this section */
u64 size;		/* size in bytes of this section */
u32 offset;	/* file offset of this section */
u32 align;		/* section alignment (power of 2) */
u32 reloff;	/* file offset of relocation entries */
u32 nreloc;	/* number of relocation entries */
u32 flags;		/* flags (section type and attributes)*/
u32 reserved1;	/* reserved (for offset or index) */
u32 reserved2;	/* reserved (for count or sizeof) */
u32 reserved3;	/* reserved */
};
 



void disasm_macho64_seg19(void* buf,int len, struct command_19* cmd,int max, u32 cputype)
{
	int cnt = cmd->nsects;
	if(0 == cnt)return;

	printf("cnt=%x{\n",cnt);
	int j;
	struct section_64* pp = (void*)cmd + sizeof(struct command_19);
	for(j=0;j<cnt;j++){
		printf("addr=%16llx,offs=%8x,name=%s.%s\n", pp[j].addr, pp[j].offset, pp[j].seg_name, pp[j].sec_name);
		if(0==strncmp(pp[j].sec_name, "__text", 16)){
			if(CPUTYPE_X8664==cputype)disasm_x8664_all(buf+pp[j].offset, pp[j].size, pp[j].addr);
			if(CPUTYPE_ARM64==cputype)disasm_arm64_all(buf+pp[j].offset, pp[j].size, pp[j].addr);
		}
	}
	printf("}\n");
}
void disasm_macho64(void* buf,int len)
{
	struct mach_header_64* head = buf;
	printf(
"magic=%x\n"
"cputype_hi=%x\n"
"cputype_lo=%x\n"
"filetype=%x\n"
"cmdnum=%x\n"
"cmdsize=%x\n"
"flag=%x\n",
	head->magic,
	head->cputype,
	head->cpusubtype,
	head->filetype,
	head->ncmds,
	head->sizeofcmds,
	head->flags
	);

	if(0xfeedfacf != head->magic)return;
	switch(head->cputype){
	case CPUTYPE_X8664:
	case CPUTYPE_ARM64:
		break;
	default:
		return;
	}
	//printf("shit\n");

	int j,k;
	u32* here;

	k=0x20;;
	for(j=0;j<head->ncmds;j++){
		here = buf+k;
		printf("%08x: type=%08x, size=%08x\n", k, here[0], here[1]);

		if(0x19 == here[0])disasm_macho64_seg19(buf,len, buf+k,here[1], head->cputype);

		k += here[1];
	}
}
int check_mach(void* buf)
{
	struct mach_header_64* head = buf;
	if(0xfeedfacf != head->magic)return 0;

	switch(head->cputype){
	case CPUTYPE_X8664:
	case CPUTYPE_ARM64:
		break;
	default:
		return 0;
	}
	return 1;
}
