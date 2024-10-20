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




struct offlen{
	u8 off;
	u8 len;
}__attribute__((packed));
void assembly_riscv64_mov(u8* buf, int len, struct offlen* tab, int cnt)
{
	printf("dst=%.*s, src=%.*s\n", tab[0].len, buf+tab[0].off, tab[2].len, buf+tab[2].off);
}
void assembly_riscv64_add(u8* buf, int len, struct offlen* tab, int cnt)
{
	printf("dst=%.*s, src0=%.*s, src1=%.*s\n", tab[0].len, buf+tab[0].off, tab[2].len, buf+tab[2].off, tab[4].len, buf+tab[4].off);
}
void assembly_compile_riscv64(u8* buf, int len, struct offlen* tab, int cnt)
{
	int j;
/*	for(j=0;j<cnt;j++){
		printf("%d: %.*s\n", j, tab[j].len, buf+tab[j].off);
	}
*/
	if(0 == strncmp((char*)buf+tab[0].off, "mov", 3)){
		assembly_riscv64_mov(buf, len, &tab[1], cnt-1);
	}
	if(0 == strncmp((char*)buf+tab[0].off, "add", 3)){
		assembly_riscv64_add(buf, len, &tab[1], cnt-1);
	}
}

//riscv64-elf-gcc -march=rv64i -mabi=lp64 -c test.c