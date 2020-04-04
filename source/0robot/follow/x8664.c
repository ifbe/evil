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
int disasm_x8664_one(u8* buf, int rip);




static int* knowntable = 0;
static int knowncount = 0;
void follow_x8664_one(u8* buf, int len, int cur, int end, int ip, int sp)
{
	int j,k;
begin:
	printf("@enter: depth=%d\n", sp);

	while(1)
	{
		if(cur >= end)break;
//printf("cur=%x,end=%x,%x\n",cur,end,buf[cur]);
		j = disasm_x8664_one(buf+cur, ip+cur);

		if(0xc3 == buf[cur]){
			break;
		}
		if(0xe8 == buf[cur]){
			k = *(int*)(buf+cur+1);
			k += cur+5;

			if((k>=0)&&(k<len)){
				//if(known)skip this
				//else known++, follow this
				follow_x8664_one(buf, len, k, len, ip, sp+1);
			}//valid addr
		}
		if(0xe9 == buf[cur]){
			k = *(int*)(buf+cur+1);
			k += cur+5;
			if((k>=0)&&(k<len)){
				cur = k;
				goto begin;
			}//valid addr
		}

		cur += j;
	}//while
	printf("@leave: depth=%d\n", sp);
}
void follow_x8664(int argc, char** argv)
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

	follow_x8664_one(buf, ret, at, ret, 0x10000, 0);

release:
	free(buf);
theend:
	close(fd);
}
