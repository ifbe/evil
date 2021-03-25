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
void follow_arm64_one(u8* buf, int len, int cur, int ip, int* stack, int sp)
{
	int j;
	int off,far;
	u32 code;
	printf("@enter: depth=%d\n", sp);

begin:
	j = cur&0xfffffffc;
	far = j;
	while(1)
	{
		if(j >= len)break;

		code = *(u32*)(buf+j);
		disasm_arm64_one(code, ip+j);

		if((0xd65f03c0 == code)|(0xd65f03e0 == code)){
			//still more to follow
			if((far > j)&&(far<j+128)){
				printf("@ret(still more: cur=%x,far=%x)\n", j, far);
				j += 4;
				continue;
			}
			break;
		}

		//b.cond
		if(0x54000000 == (code&0xff000010)){
			off = ((code>>5)&0x7ffff)<<2;
			if(code&0x800000)off -= 0x200000;
			off += j;

			printf("@b.cond: off=%x,far=%x\n", off, far);
			if(far < off)far = off;

			j += 4;
			continue;
		}
		//b.zero
		if(0x34000000 == (code&0x7e000000)){
			off = ((code>>5)&0x7ffff)<<2;
			if(off >= 0x100000)off -= 0x200000;
			off += j;

			printf("@b.zero: off=%x,far=%x\n", off, far);
			if(far < off)far = off;

			j += 4;
			continue;
		}
		//b.test
		if(0x36000000 == (code&0x7e000000)){
			off = ((code>>5)&0x3fff)<<2;
			if(off >= 0x8000)off -= 0x10000;
			off += j;

			printf("@b.test: off=%x,far=%x\n", off, far);
			if(far < off)far = off;

			j += 4;
			continue;
		}

		if(0x14000000 == (code&0xfc000000)){
			//skip if address wrong
			off = (code&0x3ffffff)<<2;
			if(off >= 0x08000000)off -= 0x10000000;
			off += j;
			if((off<0)|(off>=len)){
				printf("@jump %x(wrong address)\n", off);
				j += 4;
				continue;
			}

			//skip if going back
			if((off>=cur)&&(off<=j)){
				printf("@jump %x(going back)\n", off);
				j += 4;
				continue;
			}

			//skip if going skip
			if((far > j)&&(off>=j)&&(off<j+128)){
				printf("@jump %x(still more: cur=%x,far=%x)\n", off, j,far);
				j += 4;
				continue;
			}

			//skip if address repeat
			if(knowncheck(knowntable, knowncount, off)){
				printf("@jump %x(known already)\n", off);
				j += 4;
				continue;
			}
			knowntable[knowncount] = off;
			knowncount += 1;

			printf("@jump %x(going faraway)\n", off);
			cur = off;
			goto begin;
		}//jump
		if(0x94000000 == (code&0xfc000000)){
			//skip if address wrong
			off = (code&0x3ffffff)<<2;
			if(off >= 0x08000000)off -= 0x10000000;
			off += j;
			if((off<0)|(off>=len)){
				printf("@call %x(wrong address)\n", off);
				j += 4;
				continue;
			}

			//skip if stack recursive
			if(stackcheck(stack, sp, off)){
				printf("@call %x(stack recursive)\n", off);
				j += 4;
				continue;
			}
			stack[sp] = off;

			//skip if address repeat
			if(knowncheck(knowntable, knowncount, off)){
				printf("@call %x(known already)\n", off);
				j += 4;
				continue;
			}
			knowntable[knowncount] = off;
			knowncount += 1;

			follow_arm64_one(
				buf, len,
				off, ip,
				stack, sp+1
			);
		}//call

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

	knowntable = malloc(0x100000);
	knowncount = 0;
	stacktable = malloc(0x100000);
	stackcount = 0;
	follow_arm64_one(buf, ret, at, 0, stacktable, stackcount);
	printf("knowncount=%d\n",knowncount);

release:
	free(buf);
theend:
	close(fd);
}
