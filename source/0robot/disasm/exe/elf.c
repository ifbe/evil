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




void disasm_elf64(void* buf,int len)
{
	printf("elf64\n");
}
