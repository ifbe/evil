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
int disasm_x8664_one(u8* buf, int rip);




static int* knowntable = 0;
static int knowncount = 0;
static int* stacktable = 0;
static int stackcount = 0;
static int knowncheck(int* tab, int cnt, int val)
{
	int j;
	for(j=0;j<cnt;j++){
		if(val == tab[j])return 1;
	}
	return 0;
}
static int stackcheck(int* tab, int cnt, int val)
{
	int j;
	for(j=0;j<cnt;j++){
		if(val == tab[j])return 1;
	}
	return 0;
}
void follow_x8664_one(u8* buf, int len, int cur, int ip, int* stack, int sp)
{
	int j,isz;
	int off,far;
	printf("@enter: depth=%d\n", sp);

begin:
	j = cur;
	far = cur;
	while(1)
	{
		if(j >= len)break;

		isz = disasm_x8664_one(buf+j, ip+j);

		if(0xc3 == buf[j]){
			//still more to follow
			if((far > j+isz)&&(far<j+isz+128)){
				printf("@ret(still more: cur=%x,far=%x)\n",j,far);
				j += isz;
				continue;
			}

			break;
		}

		if(0x70 == (buf[j]&0xf0)){
			off = (char)buf[j+1];
			off += j+2;

			printf("@j08: off=%x,far=%x\n", off, far);
			if(far < off)far = off;

			j += isz;
			continue;
		}
		if(0x0f == buf[j]){
		if(0x80 == (buf[j+1]&0xf0)){
			off = *(int*)(buf+j+2);
			off += j+6;

			printf("@j32: off=%x,far=%x\n", off, far);
			if(far < off)far = off;

			j += isz;
			continue;
		}//8*
		}//0f

		if(0xe8 == buf[j]){
			//skip if address wrong
			off = *(int*)(buf+j+1);
			off += j+5;
			if((off<0)|(off>=len)){
				printf("@call %x(wrong address)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if stack recursive
			if(stackcheck(stack, sp, off)){
				printf("@call %x(stack recursive)\n", off);
				j += isz;
				continue;
			}
			stack[sp] = off;

			//skip if address repeat
			if(knowncheck(knowntable, knowncount, off)){
				printf("@call %x(known already)\n", off);
				j += isz;
				continue;
			}
			knowntable[knowncount] = off;
			knowncount += 1;

			follow_x8664_one(
				buf, len,
				off, ip,
				stack, sp+1
			);
		}
		if((0xe9 == buf[j])|(0xeb == buf[j])){
			if(0xe9 == buf[j]){
				off = *(int*)(buf+j+1);
				off += j+5;
			}
			if(0xeb == buf[j]){
				off = (char)buf[j+1];
				off += j+2;
			}

			//skip if address wrong
			if((off<0)|(off>=len)){
				printf("@jump %x(wrong address)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if going back
			if((off>=cur)&&(off<=j)){
				printf("@jump %x(going back)\n", off);
				j += isz;
				continue;
			}

			//skip if going skip
			if((far > j+isz)&&(off>=j+isz)&&(off<j+isz+128)){
				printf("@jump %x(still more: cur=%x,far=%x)\n", off, j,far);
				j += isz;
				continue;
			}

			//skip if address repeat
			if(knowncheck(knowntable, knowncount, off)){
				printf("@jump %x(known already)\n", off);
				j += isz;
				continue;
			}
			knowntable[knowncount] = off;
			knowncount += 1;

			printf("@jump %x(going faraway)\n", off);
			cur = off;
			goto begin;
		}

		j += isz;
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

	int ret = read(fd, buf, sz);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto release;
	}

	knowntable = malloc(0x100000);
	knowncount = 0;
	stacktable = malloc(0x100000);
	stackcount = 0;
	follow_x8664_one(buf, ret, at, 0, stacktable, stackcount);
	printf("knowncount=%d\n",knowncount);

release:
	free(buf);
theend:
	close(fd);
}
