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
int disasm_arm64_one(u32 code, int rip);




static int* knowntable = 0;
static int knowncount = 0;
void follow_arm64_one(u8* buf, int len, int cur, int end, int ip, int sp)
{
	int j,off;
	u32 code;
begin:
	printf("@enter: depth=%d\n", sp);
	//if(isrecursive(stack))return;

	j = cur&0xfffffffc;
	while(1)
	{
		if(j >= end)break;

		code = *(u32*)(buf+j);
		disasm_arm64_one(code, ip+j);

		if(0xd65f03c0 == code){
			break;
		}
		if(0x14000000 == (code&0x7c000000)){
		off = (code&0x3ffffff)<<2;
		if(off > 0x08000000)off -= 0x10000000;
		off += j;
		//knowntable += off

		if((off>=0)&&(off<len)){
			if(off == cur){
				printf("@repeat: 0x%x\n", ip+off);
			}//dont repeat
			else if(0x14000000 == (code&0xfc000000)){
				cur = off;
				goto begin;
			}//jump
			else if(0x94000000 == (code&0xfc000000)){
				follow_arm64_one(
					buf,len,
					off,end,
					ip, sp+1
				);
			}//call
		}//valid addr
		}//jump or call

		j += 4;
	}//while
	printf("@leave: depth=%d\n", sp);
}
void follow_arm64(int argc, char** argv)
{
	u32 at = 0;
	u32 sz = 0;
	if(argc < 2)return;
	if(argc > 2)hexstr2u32(argv[2], &at);
	if(argc > 3)hexstr2u32(argv[3], &sz);
	if(0 == sz)sz = 0x1000000;

	int fd = open(argv[1] , O_RDONLY);
	if(fd <= 0){
		printf("errno=%d@open\n", errno);
		return;
	}

	u8* buf = malloc(sz);
        if(0 == buf){
		printf("errno=%d@malloc\n", errno);
		goto theend;
	}

	int ret = read(fd, buf, sz);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto release;
	}

	//knowntable += at
	follow_arm64_one(buf, ret, at, ret, 0, 0);

release:
	free(buf);
theend:
	close(fd);
}
