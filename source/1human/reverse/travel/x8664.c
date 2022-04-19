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
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define _call_ hex32('c','a','l','l')
#define _cond_ hex32('c','o','n','d')
#define _jump_ hex32('j','u','m','p')
int hexstr2u32(void* str, void* dat);
int disasm_x8664_one(u8* buf, int rip);




struct hehe{
	int llll;	//head
	int rrrr;	//tail
	int addr;	//branch addr
	int from;	//branch from
};
static struct hehe* knowntable = 0;
static int knowncount = 0;
static void known_swap(struct hehe* lo, struct hehe* hi)
{
	int aa = lo->llll;
	int bb = lo->rrrr;
	int cc = lo->addr;
	int dd = lo->from;

	lo->llll = hi->llll;
	lo->rrrr = hi->rrrr;
	lo->addr = hi->addr;
	lo->from = hi->from;

	hi->llll = aa;
	hi->rrrr = bb;
	hi->addr = cc;
	hi->from = dd;
}
void known_sort(struct hehe* buf, int len)
{
	int lo,hi;
	int llll,rrrr,addr,from;
	if(1 >= len)return;
	if(2 == len){
		if(buf[0].addr < buf[1].addr)return;
		known_swap(&buf[0], &buf[1]);
		return;
	}

	lo = 0;
	hi = len-1;

	llll = buf[0].llll;
	rrrr = buf[0].rrrr;
	addr = buf[0].addr;
	from = buf[0].from;
	while(lo < hi){
		while(lo < hi){
			if(buf[hi].addr < addr)break;
			hi--;
		}
		buf[lo].llll = buf[hi].llll;
		buf[lo].rrrr = buf[hi].rrrr;
		buf[lo].addr = buf[hi].addr;
		buf[lo].from = buf[hi].from;

		while(lo < hi){
			if(buf[lo].addr > addr)break;
			lo++;
		}
		buf[hi].llll = buf[lo].llll;
		buf[hi].rrrr = buf[lo].rrrr;
		buf[hi].addr = buf[lo].addr;
		buf[hi].from = buf[lo].from;
	}
	buf[hi].llll = llll;
	buf[hi].rrrr = rrrr;
	buf[hi].addr = addr;
	buf[hi].from = from;

	known_sort(buf, hi);
	known_sort(buf+hi+1, len-hi-1);
}
void known_print()
{
	int j;
	printf("knowncount=%d\n",knowncount);
	for(j=0;j<knowncount;j++){
		printf("%8x,%8x	[%8x,%8x)\n",
		knowntable[j].addr, knowntable[j].from,
		knowntable[j].llll, knowntable[j].rrrr
		);
	}
}
static int known_check(int val)
{
	int j;
	for(j=0;j<knowncount;j++){
		if(val == knowntable[j].addr)return j;
	}
	return -1;
}




struct haha{
	int head;	//head
	int tail;	//tail
	int save;	//branch addr
	int type;	//branch type
};
static void* stacktable = 0;
static int stackcount = 0;
static int checkrecur(struct haha* stack, int sp, int val)
{
	int j;
	for(j=0;j<sp;j++){
		if(val == stack[j].head)return 1;
	}
	return 0;
}
static int checkmerge(struct haha* stack, int sp, int val)
{
	int j;
	for(j=0;j<sp;j++){
		if(val < stack[j].head)continue;
		if(val > stack[j].tail)continue;
		return j;
	}
	return -1;
}
void printstack(struct haha* stack, int sp)
{
	int j;
	for(j=0;j<sp;j++)printf("%x:	%8x,%8x,%8x,%.4s\n", j,
		stack[j].head, stack[j].tail,
		stack[j].save, (char*)&stack[j].type);
}




void travel_x8664_one(u8* buf, int len, int entry, int ip, struct haha* stack, int sp)
{
	int j,k;
	int isz,off;
	int self,that;

	knowntable[0].llll = entry;
	knowntable[0].rrrr = entry;
	knowntable[0].addr = entry;
	knowntable[0].from = entry;
	knowncount += 1;

	self = 0;
	j = entry;
	while(1)
	{
		if(j >= len)break;

		//next: back to call/cond point
		that = known_check(j);
		if((that >= 0)&&(j != entry)){

			printf("merge [%x,%x] with [%x,%x]\n",
				entry, j,
				knowntable[that].llll, knowntable[that].rrrr
			);
			knowntable[self].rrrr = knowntable[that].rrrr;
			knowntable[that].llll = knowntable[self].llll;	//entry;

			printf("@back_at(%x,%x,%x)\n", entry,j,sp);
			while(1){
				if(0 == sp)return;
				sp--;
				if(_call_ == stack[sp].type)break;
				if(_cond_ == stack[sp].type)break;
			}

			entry = stack[sp].head;
			j = stack[sp].save;
			self = known_check(entry);
			printf("@back_to(%x,%x,%x)\n", entry,j,sp);
			continue;
		}

		isz = disasm_x8664_one(buf+j, ip+j);

		//ret: back to call/cond point
		if(0xc3 == buf[j]){
			//update known
			knowntable[self].rrrr = j+isz;

			printf("@ret_at(%x,%x,%x)\n", entry,j+isz,sp);
			while(1){
				if(0 == sp)return;
				sp--;
				if(_call_ == stack[sp].type)break;
				if(_cond_ == stack[sp].type)break;
			}

			entry = stack[sp].head;
			j = stack[sp].save;
			self = known_check(entry);
			printf("@ret_to(%x,%x,%x)\n", entry,j,sp);
			continue;
		}

		//call: push next, jump into
		if(0xe8 == buf[j]){
			//update known
			knowntable[self].rrrr = j+isz;

			//skip if address wrong
			off = *(int*)(buf+j+1);
			off += j+5;
			if((off<0)|(off>=len)){
				printf("@call %x(wrong address)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if stack recursive
			if(checkrecur(stack, sp, off)){
				printf("@call %x(stack recursive)\n", off);
				j += isz;
				continue;
			}

			//skip if address repeat
			if(known_check(off) >= 0){
				printf("@call %x(known already)\n", off);
				j += isz;
				continue;
			}
			knowntable[knowncount].llll = off;
			knowntable[knowncount].rrrr = off;
			knowntable[knowncount].addr = off;
			knowntable[knowncount].from = off;	//call.parent=itself
			knowncount += 1;

			stack[sp].head = entry;
			stack[sp].tail = j+isz;
			stack[sp].save = j+isz;
			stack[sp].type = _call_;
			printf("@call_at(%x,%x,%x)\n", entry,j+isz,sp);

			sp++;
			entry = off;
			j = off;
			self = known_check(entry);
			printf("@call_to(%x,%x,%x)\n", entry,j,sp);
			continue;
		}

		//cond
		if(	(0x70 == (buf[j]&0xf0)) |
			((0x0f == buf[j])&&(0x80 == (buf[j+1]&0xf0)))	){

			//update known
			knowntable[self].rrrr = j+isz;

			//
			if(0x0f != buf[j])off = (char)buf[j+1];
			else off = *(int*)(buf+j+2);
			off += j+isz;

			//skip if address wrong
			if((off<0)|(off>=len)){
				printf("@cond %x(wrong address)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if going back
			if((off>=entry)&&(off<j)){
				printf("@cond %x(still inside)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if address known
			if(known_check(off) >= 0){
				printf("@cond %x(known already)\n", off);
				j += isz;
				continue;
			}
			knowntable[knowncount].llll = off;
			knowntable[knowncount].rrrr = off;
			knowntable[knowncount].addr = off;
			knowntable[knowncount].from = entry;
			knowncount += 1;

			//skip if stack recursive
			if(checkmerge(stack, sp, off) >= 0){
				printf("@cond %x(stack recursive)\n", off);
				j += isz;
				continue;
			}

			stack[sp].head = entry;
			stack[sp].tail = j+isz;
			stack[sp].save = j+isz;
			stack[sp].type = _cond_;
			printf("@cond_at(%x,%x,%x)\n", entry,j+isz,sp);

			sp++;
			entry = off;
			j = off;
			self = known_check(entry);
			printf("@cond_to(%x,%x,%x)\n", entry,j,sp);
			continue;
		}

		//jump
		if((0xe9 == buf[j])|(0xeb == buf[j])){
			//update known
			knowntable[self].rrrr = j+isz;

			//
			if(0xe9 == buf[j])off = *(int*)(buf+j+1);
			if(0xeb == buf[j])off = (char)buf[j+1];
			off += j+isz;

			//skip if address wrong
			if((off<0)|(off>=len)){
				printf("@jump %x(wrong address)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if going back
			if((off>=entry)&&(off<j)){
				printf("@jump %x(still inside)\n", off);
				j += isz;
				continue;
			}//valid addr

			//skip if address known
			if(known_check(off) >= 0){
				printf("@jump %x(known already)\n", off);
				printf("@back_at(%x,%x,%x)\n", entry,j,sp);
				while(1){
					if(0 == sp)return;
					sp--;
					if(_jump_ == stack[sp].type)continue;
					break;
				}

				entry = stack[sp].head;
				j = stack[sp].save;
				self = known_check(entry);
				printf("@back_to(%x,%x,%x)\n", entry,j,sp);
				continue;
			}
			knowntable[knowncount].llll = off;
			knowntable[knowncount].rrrr = off;
			knowntable[knowncount].addr = off;
			knowntable[knowncount].from = entry;
			knowncount += 1;

			//skip if stack recursive
			if(checkmerge(stack, sp, off) >= 0){
				printf("@jump %x(stack recursive)\n", off);

				printf("@giveup: (%x,%x,%x)", entry,j+isz,sp);
				while(1){
					if(0 == sp)return;
					sp--;
					if(_jump_ == stack[sp].type)continue;
					break;
				}

				entry = stack[sp].head;
				j = stack[sp].save;
				self = known_check(entry);
				printf("->(%x,%x,%x)\n", entry,j,sp);
				continue;
			}

			stack[sp].head = entry;
			stack[sp].tail = j+isz;
			stack[sp].save = j+isz;
			stack[sp].type = _jump_;
			printf("@jump_at(%x,%x,%x)\n", entry,j+isz,sp);

			sp++;
			entry = off;
			j = off;
			self = known_check(entry);
			printf("@jump_to(%x,%x,%x)\n", entry,j,sp);
			continue;
		}

		j += isz;
	}
}
void travel_x8664(int argc, char** argv)
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
	travel_x8664_one(buf, ret, at, 0, stacktable, stackcount);

	known_sort(knowntable, knowncount);
	known_print();

release:
	free(buf);
theend:
	close(fd);
}
