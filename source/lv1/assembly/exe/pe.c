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
int disasm_x8664_all(void* buf, int len, u64 rip);




struct mz
{
	u16 magic;		//'M','Z'
	u16 cblp;		//文件最后页字节数
	u16 fcp;		//文件页数
	u16 crlc;		//重定义元素个数
	u16 hdrcp;		//头部尺寸
	u16 min;		//最小附加段
	u16 max;		//最大附加段
	u16 ss;
	u16 sp;
	u16 cksum;		//校验和
	u16 ip;
	u16 cs;
	u16 farlc;		//重分配表地址
	u16 ovno;		//覆盖号
	u16 res[4];		//保留字
	u16 oemid;
	u16 oeminfo;
	u16 res2[10];	//保留字
	u32 addr;		//pe段的地址
};
struct pe
{
	u32 magic;
	u16 machine;
	u16 numsec;
	u32 timedate;
	u32 symtab;
	u32 symnum;
	u16 szopthdr;
	u16 character;
};
struct opthdr
{
	u16 magic;
	u8 majorlinker;
	u8 minorlinker;
	u32 codesize;
	u32 datasize;
	u32 bsssize;
	u32 entry;
	u32 codebase;
};
struct opt32
{
	u16 magic;
	u8 majorlinker;
	u8 minorlinker;
	u32 codesize;
	u32 datasize;
	u32 bsssize;
	u32 entry;
	u32 codebase;
	u32 database;	//only in 32
	u32 imagebase;
	u32 sectionalign;
	u32 filealign;
	u16 majoropsys;
	u16 minoropsys;
	u16 majorversion;
	u16 minorversion;
	u16 majorsubsys;
	u16 minorsubsys;
	u16 win32version;
	u32 imagesize;
	u32 headersize;
	u32 checksum;
	u16 subsys;
	u16 dllcharacter;
	u32 stackreserve;
	u32 stackcommit;
	u32 heapreserve;
	u32 heapcommit;
	u32 loaderflag;
	u32 numrvasize;
};
struct opt64
{
	u16 magic;
	u8 majorlinker;
	u8 minorlinker;
	u32 codesize;
	u32 datasize;
	u32 bsssize;
	u32 entry;
	u32 codebase;
	u64 imagebase;
	u32 sectionalign;
	u32 filealign;
	u16 majoropsys;
	u16 minoropsys;
	u16 majorversion;
	u16 minorversion;
	u16 majorsubsys;
	u16 minorsubsys;
	u16 win32version;
	u32 imagesize;
	u32 headersize;
	u32 checksum;
	u16 subsys;
	u16 dllcharacter;
	u64 stackreserve;
	u64 stackcommit;
	u64 heapreserve;
	u64 heapcommit;
	u32 loaderflag;
	u32 numrvasize;
};
struct dirtab
{
	u32 exportaddr;
	u32 exportsize;
	u32 importaddr;
	u32 importsize;
	u32 resourceaddr;
	u32 resourcesize;
	u32 exceptionaddr;
	u32 exceptionsize;
	u32 certificateaddr;
	u32 certificatesize;
	u32 relocationaddr;
	u32 relocationsize;
	u32 debugaddr;
	u32 debugsize;
	u32 architectureaddr;
	u32 architecturesize;
	u32 globalptraddr;
	u32 globalptrsize;
	u32 tlstableaddr;
	u32 tlstablesize;
	u32 loadconfigaddr;
	u32 loadconfigsize;
	u32 boundimportaddr;
	u32 boundimportsize;
	u32 iataddr;
	u32 iatsize;
	u32 delayimportaddr;
	u32 delayimportsize;
	u64 clrruntime;
	u64 mustzero;
};
struct sectab{
	char Name[8];
	u32 VirtualSize;
	u32 VirtualAddress;
	u32 SizeOfRawData;
	u32 PointerToRawData;
	u32 PointerToRelocations;
	u32 PointerToLinenumbers;
	u16 NumberOfRelocations;
	u16 NumberOfLinenumbers;
	u32 Characteristics;
};




u64 disasm_pe64_nt(struct pe* pe,int len)
{
	u64 imagebase;
	struct opthdr* opt;
	struct opt32* opt32;
	struct opt64* opt64;
	struct dirtab* dir;

	//------------pe.part0-----------
	printf(
"machine=%x\n"
"symtab=%x\n"
"symnum=%x\n"
"numsec=%x\n"
"szopthdr=%x\n"
"\n",
pe->machine,
pe->symtab,
pe->symnum,
pe->numsec,
pe->szopthdr
	);

	//------------pe.part1-----------
	opt = (void*)pe+24;
	printf(
"codesize=%x\n"
"datasize=%x\n"
"bsssize=%x\n"
"entry=%x\n"
"codebase=%x\n"
"\n",
opt->codesize,
opt->datasize,
opt->bsssize,
opt->entry,
opt->codebase
	);

	//32 or 64
	if(opt->magic == 0x10b)
	{
		opt32 = (void*)opt;
		dir = (void*)opt + 96;

		imagebase = opt32->imagebase;
		printf(
"database=%x\n"
"imagebase=%x\n"
"imagesize=%x\n"
"headersize=%x\n"
"stackreserve=%x\n"
"stackcommit=%x\n"
"reserve=%x\n"
"heapcommit=%x\n"
"\n",
opt32->database,
opt32->imagebase,
opt32->imagesize,
opt32->headersize,
opt32->stackreserve,
opt32->stackcommit,
opt32->heapreserve,
opt32->heapcommit
		);
	}
	else if(opt->magic == 0x20b)
	{
		opt64 = (void*)opt;
		dir = (void*)opt + 108;

		imagebase = opt64->imagebase;
		printf(
"imagebase=%llx\n"
"imagesize=%x\n"
"headersize=%x\n"
"stackreserve=%llx\n"
"stackcommit=%llx\n"
"heapreserve=%llx\n"
"heapcommit=%llx\n"
"\n",
opt64->imagebase,
opt64->imagesize,
opt64->headersize,
opt64->stackreserve,
opt64->stackcommit,
opt64->heapreserve,
opt64->heapcommit
		);
	}

	//---------------pe.part2-------------
	printf(
"export@%x$%x\n"
"import@%x$%x\n"
"resource@%x$%x\n"
"exception@%x$%x\n"
"certificate@%x$%x\n"
"relocation@%x$%x\n"
"debug@%x$%x\n"
"architecture@%x$%x\n"
"globalptr@%x$%x\n"
"tlstable@%x$%x\n"
"loadconfig@%x$%x\n"
"boundimport@%x$%x\n"
"iat@%x$%x\n"
"delayimport@%x$%x\n"
"\n",
dir->exportaddr, dir->exportsize,
dir->importaddr, dir->importsize,
dir->resourceaddr, dir->resourcesize,
dir->exceptionaddr, dir->exceptionsize,
dir->certificateaddr, dir->certificatesize,
dir->relocationaddr, dir->relocationsize,
dir->debugaddr, dir->debugsize,
dir->architectureaddr, dir->architecturesize,
dir->globalptraddr, dir->globalptrsize,
dir->tlstableaddr, dir->tlstablesize,
dir->loadconfigaddr, dir->loadconfigsize,
dir->boundimportaddr, dir->boundimportsize,
dir->iataddr, dir->iatsize,
dir->delayimportaddr, dir->delayimportsize
	);

	return imagebase;
}
static int shoulddisasm(char* curr, char* want)
{
	if(want){
		if(strcmp(curr, want)==0)return 1;
	}
	else{
		if(strncmp(curr, ".text", 5)==0)return 1;
	}
	return 0;
}
int disasm_pe64(void* pe, int len, char* section)
{
	u64 base;
	int j,num;
	int mzsz,ntsz;
	struct mz* mz;
	struct pe* nt;
	struct sectab* sec;

//------------mz head-------------
	mz = pe;
	mzsz = mz->addr;
	printf("mz@[0,%x)\n", mzsz);


//------------pe head-------------
	nt = (void*)(pe+mzsz);
	ntsz = 24 + nt->szopthdr;
	printf("nt@[%x,%x)\n", mzsz, mzsz+ntsz);

	base = disasm_pe64_nt(nt, ntsz);


//-----------section table--------
	sec = (void*)nt + ntsz;
	num = nt->numsec;
	printf("section@[%x,%x)\n", mzsz+ntsz, mzsz+ntsz + num*0x28);

	for(j=0;j<num;j++){
		printf("%8x,%8x,%8x,%8x:	%.8s\n",
        		sec[j].PointerToRawData,
        		sec[j].SizeOfRawData,
			sec[j].VirtualAddress,
			sec[j].VirtualSize,
			sec[j].Name
		);
		int dis = shoulddisasm(sec[j].Name, section);
		if(!dis)continue;

		disasm_x8664_all(
			pe+sec[j].PointerToRawData, sec[j].SizeOfRawData,
			base + sec[j].VirtualAddress
		);
	}
	return 0;
}
int check_pe(u8* addr)
{
	u32 temp;

	//[0,1]:'M','Z'
	temp=*(u16*)addr;
	if(temp!=0x5a4d)return 0;

	//[0x3c,0x3f]:	=0x40,=0xf0,=0x?
	temp=*(u32*)(addr+0x3c);
	if(temp>=0xffc)return 0;

	temp=*(u32*)(addr+temp);
	if(temp!=0x4550)return 0;

	return 64;
}
