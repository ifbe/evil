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
//
void disasm_x8664_all(u8* buf, int len, u64 rip);
void disasm_arm64_all(u8* buf, int len, int rip);
void disasm_mips64_all(u8* buf, int len, int rip);
void disasm_riscv64_all(u8* buf, int len, int rip);
//
int check_elf(void*);
int disasm_elf64(void*, int, char*);
int check_mach(void*);
int disasm_macho64(void*, int, char*);
int check_pe(void*);
int disasm_pe64(void*, int, char*);
int check_object(void*);
int disasm_object(void*, int, char*);




void disasm_x8664(int argc, char** argv)
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

	disasm_x8664_all(buf, ret, 0);

release:
	free(buf);
theend:
	close(fd);
}
void disasm_arm64(int argc, char** argv)
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
	disasm_arm64_all(buf, ret, 0);

release:
	free(buf);
theend:
	close(fd);
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
void disasm_riscv64(int argc, char** argv)
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
	disasm_riscv64_all(buf, ret, 0);

release:
	free(buf);
theend:
	close(fd);
}
void disasm(int argc, char** argv)
{
	u32 at = 0;
	u32 sz = 0;
	char* section = 0;
	if(argc < 2)return;
	/*
	if(argc > 2)hexstr2u32(argv[2], &at);
	if(argc > 3)hexstr2u32(argv[3], &sz);
	*/
	for(int j=2;j<argc;j++){
		if(strncmp(argv[j], "section=", 8)==0){
			section = argv[j]+8;
		}
		else if(strncmp(argv[j], "at=", 3)==0){
			hexstr2u32(argv[j]+3, &at);
		}
		else if(strncmp(argv[j], "sz=", 3)==0){
			hexstr2u32(argv[j]+3, &sz);
		}
	}
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

	if(check_elf(buf)){
		disasm_elf64(buf, ret, section);
		goto release;
	}
	if(check_mach(buf)){
		disasm_macho64(buf, ret, section);
		goto release;
	}
	if(check_pe(buf)){
		disasm_pe64(buf, ret, section);
		goto release;
	}
	if(check_object(buf)){
		disasm_object(buf, ret, section);
		goto release;
	}

	printf("unknown filetype, bye bye!\n");

release:
	free(buf);
theend:
	close(fd);
}
