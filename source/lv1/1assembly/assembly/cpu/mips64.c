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
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
int hexstr2u32(void* str, void* dat);




void disasm_mips64_one(u32 code, u64 rip)
{
	printf("%8llx:	%08x	", rip, code);
	printf("\n");
}
void disasm_mips64_all(u8* buf, int len, int rip)
{
	int j;
	u32 code;
	for(j=0;j<len;j+=4){
		code = *(u32*)(buf+j);
		disasm_mips64_one(code, rip+j);
	}
}
void disasm_mips64(int argc, char** argv)
{
	u32 at = 0;
	u32 sz = 0;
	if(argc < 2)return;
	if(argc > 2)hexstr2u32(argv[2], &at);
	if(argc > 3)hexstr2u32(argv[3], &sz);
	if(0 == sz)sz = 0x1000000;

	int fd = open(argv[1] , O_RDONLY|O_BINARY);
	if(fd <= 0){
		printf("errno=%d@open\n", errno);
		return;
	}

	u8* buf = malloc(sz);
	if(0 == buf){
		printf("errno=%d@malloc\n", errno);
		goto theend;
	}

	int ret = lseek(fd, at, SEEK_SET);
	if(ret < 0){
		printf("errno=%d@lseek\n", errno);
		goto release;
	}

	ret = read(fd, buf, sz);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto release;
	}
	disasm_mips64_all(buf, ret, 0);

release:
	free(buf);
theend:
	close(fd);
}




struct offlen{
	u8 off;
	u8 len;
}__attribute__((packed));
void assembly_mips64_mov(u8* buf, int len, struct offlen* tab, int cnt)
{
	printf("dst=%.*s, src=%.*s\n", tab[0].len, buf+tab[0].off, tab[2].len, buf+tab[2].off);
}
void assembly_mips64_add(u8* buf, int len, struct offlen* tab, int cnt)
{
	printf("dst=%.*s, src0=%.*s, src1=%.*s\n", tab[0].len, buf+tab[0].off, tab[2].len, buf+tab[2].off, tab[4].len, buf+tab[4].off);
}
void assembly_compile_mips64(u8* buf, int len, struct offlen* tab, int cnt)
{
	int j;
/*	for(j=0;j<cnt;j++){
		printf("%d: %.*s\n", j, tab[j].len, buf+tab[j].off);
	}
*/
	if(0 == strncmp((char*)buf+tab[0].off, "mov", 3)){
		assembly_mips64_mov(buf, len, &tab[1], cnt-1);
	}
	if(0 == strncmp((char*)buf+tab[0].off, "add", 3)){
		assembly_mips64_add(buf, len, &tab[1], cnt-1);
	}
}