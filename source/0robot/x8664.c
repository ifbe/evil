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




#define rexb (1<<0)
#define rexx (1<<1)
#define rexr (1<<2)
#define rexw (1<<3)
static char* seg16[16] = {
"es","cs","ss","ds","fs","gs","-","-",
"es","cs","ss","ds","fs","gs","-","-"};
static char* cond[16] = {
"o", "no", "c", "nc",
"z", "nz", "na", "a",
"s", "ns", "pe", "po",
"l", "nl", "ng", "g"};
static char* reg64[16] = {
"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",
 "r8", "r9","r10","r11","r12","r13","r14","r15"};
static char* reg32[16] = {
"eax","ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
"r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};
static char* reg16[16] = {
 "ax", "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",
"r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"};
static char* reg08[16] = {
 "al", "cl",  "dl",  "bl",  "ah",  "ch",  "dh",  "bh",
"r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};
static char* rex08[16] = {
 "al", "cl",  "dl",  "bl", "spl", "bpl", "sil", "dil",
"r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"};

static char* symbol[8] = {
"+=", "|=", "+=", "-=", "&=", "-=", "^=", "-"};
static char* op80[8] = {
"add1","or1","adc1","sbb1","and1","sub1","xor1","cmp1"};
static char* op81[8] = {
"add4","or4","adc4","sbb4","and4","sub4","xor4","cmp4"};
static char* op83[8] = {
"add4","or4","adc4","sbb4","and4","sub4","xor4","cmp4"};
static char* opc0[8] = {
"rol1","ror1","rcl1","rcr1","shl1","shr1","error","sar1"};
static char* opc1[8] = {
"rol4","ror4","rcl4","rcr4","shl4","shr4","error","sar4"};
static char* opf6[8] = {
"test1","error1","not1","neg1","mul1","imul1","div1","idiv1"};
static char* opf7[8] = {
"test4","error4","not4","neg4","mul4","imul4","div4","idiv4"};
static char* opfe[2] = {"inc1","dec1"};
static char* opff[8] = {
"inc4","dec4","call8","call4","jmp8","jmp4","push8"};




void disasm_x8664_print(u8* p, int k)
{
	int j;
	u8 buf[32];

	for(j=0;j<k;j++){
		buf[j*2+0] = (p[j]>>4) + '0';
		if(buf[j*2+0] > '9')buf[j*2+0] += 7;

		buf[j*2+1] = (p[j]&0xf) + '0';
		if(buf[j*2+1] > '9')buf[j*2+1] += 7;
	}

	for(j=j+j;j<30;j++)buf[j] = '_';
	buf[30] = 0;
	printf("%s	",buf);
}
int disasm_x8664_1b(u8* p)
{
	char buf[16];
	char* str;

	if(0x26 == p[0]){
		str = "es";
		goto yes;
	}
	if(0x2e == p[0]){
		str = "cs";
		goto yes;
	}
	if(0x36 == p[0]){
		str = "ss";
		goto yes;
	}
	if(0x3e == p[0]){
		str = "ds";
		goto yes;
	}
	if(0x26 == p[0]){
		str = "es";
		goto yes;
	}
	if((p[0] >= 0x50)&&(p[0] <= 0x57)){
		snprintf(buf,16, "push	%s", reg64[p[0]&0x7]);
		str = buf;
		goto yes;
	}
	if((p[0] >= 0x58)&&(p[0] <= 0x5f)){
		snprintf(buf,16, "pop	%s", reg64[p[0]&0x7]);
		str = buf;
		goto yes;
	}
	if(0x64 == p[0]){
		str = "fs";
		goto yes;
	}
	if(0x65 == p[0]){
		str = "gs";
		goto yes;
	}
	if(0x6c == p[0]){
		str = "insb";
		goto yes;
	}
	if(0x6d == p[0]){
		str = "insd";
		goto yes;
	}
	if(0x6e == p[0]){
		str = "outsb";
		goto yes;
	}
	if(0x6f == p[0]){
		str = "outsd";
		goto yes;
	}
	if(0x90 == p[0]){
		str = "nop";
		goto yes;
	}
	if((p[0] >= 0x91)&&(p[0] <= 0x97)){
		snprintf(buf,16, "xchg	eax, %s", reg32[p[0]&0x7]);
		str = buf;
		goto yes;
	}
	if(0x98 == p[0]){
		str = "cwde";
		goto yes;
	}
	if(0x99 == p[0]){
		str = "cdq";
		goto yes;
	}
	if(0x9b == p[0]){
		str = "wait";
		goto yes;
	}
	if(0x9c == p[0]){
		str = "pushf";
		goto yes;
	}
	if(0x9d == p[0]){
		str = "popf";
		goto yes;
	}
	if(0x9e == p[0]){
		str = "sahf";
		goto yes;
	}
	if(0x9f == p[0]){
		str = "lahf";
		goto yes;
	}
	if(0xa4 == p[0]){
		str = "movsb";
		goto yes;
	}
	if(0xa5 == p[0]){
		str = "movsd";
		goto yes;
	}
	if(0xa6 == p[0]){
		str = "cmpsb";
		goto yes;
	}
	if(0xa7 == p[0]){
		str = "cmpsd";
		goto yes;
	}
	if(0xaa == p[0]){
		str = "stosb";
		goto yes;
	}
	if(0xab == p[0]){
		str = "stosd";
		goto yes;
	}
	if(0xac == p[0]){
		str = "lodsb";
		goto yes;
	}
	if(0xad == p[0]){
		str = "lodsd";
		goto yes;
	}
	if(0xae == p[0]){
		str = "scasb";
		goto yes;
	}
	if(0xaf == p[0]){
		str = "scasd";
		goto yes;
	}
	if(0xc3 == p[0]){
		str = "ret";
		goto yes;
	}
	if(0xcc == p[0]){
		str = "int3";
		goto yes;
	}
	if(0xc9 == p[0]){
		str = "leave";
		goto yes;
	}
	if(0xcb == p[0]){
		str = "retf";
		goto yes;
	}
	if(0xcf == p[0]){
		str = "iret";
		goto yes;
	}
	if(0xd6 == p[0]){
		str = "salc";
		goto yes;
	}
	if(0xd7 == p[0]){
		str = "xlatb";
		goto yes;
	}
	if(0xec == p[0]){
		str = "in	al = port[dx]";
		goto yes;
	}
	if(0xed == p[0]){
		str = "in	eax = port[dx]";
		goto yes;
	}
	if(0xee == p[0]){
		str = "out	port[dx] = al";
		goto yes;
	}
	if(0xef == p[0]){
		str = "out	port[dx] = eax";
		goto yes;
	}
	if(0xf0 == p[0]){
		str = "lock";
		goto yes;
	}
	if(0xf1 == p[0]){
		str = "int1";
		goto yes;
	}
	if(0xf2 == p[0]){
		str = "repne";
		goto yes;
	}
	if(0xf3 == p[0]){
		str = "rep";
		goto yes;
	}
	if(0xf4 == p[0]){
		str = "hlt";
		goto yes;
	}
	if(0xf5 == p[0]){
		str = "cmc";
		goto yes;
	}
	if(0xf8 == p[0]){
		str = "clc";
		goto yes;
	}
	if(0xf9 == p[0]){
		str = "stc";
		goto yes;
	}
	if(0xfa == p[0]){
		str = "cli";
		goto yes;
	}
	if(0xfb == p[0]){
		str = "sti";
		goto yes;
	}
	if(0xfc == p[0]){
		str = "cld";
		goto yes;
	}
	if(0xfd == p[0]){
		str = "std";
		goto yes;
	}
	return 0;

yes:
	disasm_x8664_print(p,1);
	printf("%s\n",str);
	return 1;
}
int disasm_x8664_sib(u64 rip, u8* p, char* str, s64* dat)
{
	u8 _s,_i,_b;
	u8 src,dst;
	s64 off;
/*
	//p[1]: [c0,ff]: register
	if(p[1] >= 0xc0){
		dst = p[1]&7;
		snprintf(str, 64, "%s", reg[dst]);
		return 2;
	}
*/
	//p[1]: [80,bf]
	if(p[1] >= 0x80){
		//p[1] != *4 *c
		if(4 != (p[1] &7)){
			dst = p[1]&7;
			off = *(int*)(p+2);
			snprintf(str, 64, "%s +0x%llx", reg64[dst], off);
			return 6;
		}

		//p[2] = [20,27] [60,67] [a0,a7] [e0,e7]
		_s = 1 << (p[2]>>6);
		_i = (p[2]>>3)&7;
		_b = p[2]&7;
		off = *(int*)(p+3);
		if(4 == _i){
			snprintf(str, 64, "%s +0x%llx", reg64[_b], off);
		}
		//p[2] != above
		else{
			snprintf(str, 64, "%d*%s +%s +0x%llx", _s, reg64[_i], reg64[_b], off);
		}
		return 7;
	}

	//p[1]: [40,7f]
	if(p[1] >= 0x40){
		//p[1] != *4 *c
		if(4 != (p[1] &7)){
			dst = p[1]&7;
			snprintf(str, 64, "%s %+d", reg64[dst], *(char*)(p+2));
			return 3;
		}

		//p[2] = [20,27] [60,67] [a0,a7] [e0,e7]
		_s = 1 << (p[2]>>6);
		_i = (p[2]>>3)&7;
		_b = p[2]&7;
		if(4 == _i){
			snprintf(str, 64, "%s %+d", reg64[_b], *(char*)(p+3));
		}
		//p[2] != above
		else{
			snprintf(str, 64, "%d*%s +%s %+d", _s, reg64[_i], reg64[_b], *(char*)(p+3));
		}
		return 4;
	}

	//p[1]: [00,3f]
	if(p[1] < 0x40){
		//p[1] != *4 *5 *c *d
		if(4 != (p[1] & 6)){
			dst = p[1]&7;
			snprintf(str, 64, "%s", reg64[dst]);
			return 2;
		}
		//p[1] = *5 *d
		if(5 == (p[1] &7)){
			off = *(int*)(p+2);
			if(str)snprintf(str, 64, "rel %llx", rip+6+off);
			if(dat)dat[0] = off;
			return 6;
		}

		//p[2] = *5 *d
		_s = 1 << (p[2]>>6);
		_i = (p[2]>>3)&7;
		_b = p[2]&7;
		if(5 == _b){
			off = *(int*)(p+3);
			if(4 == _i)snprintf(str, 64, "%llx", off);
			else snprintf(str, 64, "%d*%s+0x%llx", _s, reg64[_i], off);
			return 7;
		}
		//p[2] = else
		if(4 == _i)snprintf(str, 64, "%s", reg64[_b]);
		else snprintf(str, 64, "%d*%s+%s", _s, reg64[_i], reg64[_b]);
		return 3;
	}//add [rel 0x1234],al

	return 1;
}
int disasm_x8664_normal(u64 j, u8* p, u8 fix)
{
	int ret;
	s64 rel;
	char tmp[128];

	u8 bit6 = (p[1]>>6)&3;
	u8 bit3 = (p[1]>>3)&7;
	u8 bit0 = p[1]&7;

	//add
	if(0x0 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("add	%s += %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("add	[%s] += %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x1 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("add	%s += %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("add	[%s] += %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x2 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("add	%s += %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("add	%s += [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x3 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("add	%s += %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("add	%s += [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x4 == p[0]){
		disasm_x8664_print(p,2);
		printf("add	al += 0x%x\n", p[1]);
		return 2;
	}
	if(0x5 == p[0]){
		disasm_x8664_print(p,5);
		printf("add	eax += 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//6,7: unused

	//or
	if(0x8 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("or	%s |= %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("or	[%s] |= %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x9 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("or	%s |= %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("or	[%s] |= %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0xa == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("or	%s |= %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("or	%s |= [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0xb == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("or	%s |= %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("or	%s |= [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0xc == p[0]){
		disasm_x8664_print(p,2);
		printf("or	al |= 0x%x\n", p[1]);
		return 2;
	}
	if(0xd == p[0]){
		disasm_x8664_print(p,5);
		printf("or	eax |= 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//e: unused
	//f: special

	//adc
	if(0x10 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("adc	%s += %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("adc	[%s] += %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x11 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("adc	%s += %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("adc	[%s] += %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x12 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("adc	%s += %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("adc	%s += [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x13 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("adc	%s += %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("adc	%s += [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x14 == p[0]){
		disasm_x8664_print(p,2);
		printf("adc	al += 0x%x\n", p[1]);
		return 2;
	}
	if(0x15 == p[0]){
		disasm_x8664_print(p,5);
		printf("adc	eax += 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//16,17: unused

	//sbb
	if(0x18 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sbb	%s -= %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sbb	[%s] -= %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x19 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sbb	%s -= %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sbb	[%s] -= %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x1a == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sbb	%s -= %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sbb	%s -= [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x1b == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sbb	%s -= %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sbb	%s -= [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x1c == p[0]){
		disasm_x8664_print(p,2);
		printf("sbb	al -= 0x%x\n", p[1]);
		return 2;
	}
	if(0x1d == p[0]){
		disasm_x8664_print(p,5);
		printf("sbb	eax -= 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//1e,1f: unused

	//and
	if(0x20 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("and	%s &= %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("and	[%s] &= %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x21 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("and	%s &= %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("and	[%s] &= %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x22 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("and	%s &= %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("and	%s &= [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x23 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("and	%s &= %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("and	%s &= [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x24 == p[0]){
		disasm_x8664_print(p,2);
		printf("and	al &= 0x%x\n", p[1]);
		return 2;
	}
	if(0x25 == p[0]){
		disasm_x8664_print(p,5);
		printf("and	eax &= 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//26: es
	//27: unused

	//sub
	if(0x28 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sub	%s -= %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sub	[%s] -= %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x29 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sub	%s -= %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sub	[%s] -= %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x2a == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sub	%s -= %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sub	%s -= [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x2b == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("sub	%s -= %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("sub	%s -= [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x2c == p[0]){
		disasm_x8664_print(p,2);
		printf("sub	al -= 0x%x\n", p[1]);
		return 2;
	}
	if(0x2d == p[0]){
		disasm_x8664_print(p,5);
		printf("sub	eax -= 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//2e: cs
	//2f: unused

	//xor
	if(0x30 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("xor	%s ^= %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("xor	[%s] ^= %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x31 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("xor	%s ^= %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("xor	[%s] ^= %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x32 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("xor	%s ^= %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("xor	%s ^= [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x33 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("xor	%s ^= %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("xor	%s ^= [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x34 == p[0]){
		disasm_x8664_print(p,2);
		printf("xor	al ^= 0x%x\n", p[1]);
		return 2;
	}
	if(0x35 == p[0]){
		disasm_x8664_print(p,5);
		printf("xor	eax ^= 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//36: ss
	//37: rex

	//cmp
	if(0x38 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("cmp	%s - %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("cmp	[%s] - %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x39 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("cmp	%s - %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("cmp	[%s] - %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x3a == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("cmp	%s - %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("cmp	%s - [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x3b == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("cmp	%s - %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("cmp	%s - [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	if(0x3c == p[0]){
		disasm_x8664_print(p,2);
		printf("cmp	al - 0x%x\n", p[1]);
		return 2;
	}
	if(0x3d == p[0]){
		disasm_x8664_print(p,5);
		printf("cmp	eax - 0x%x\n", *(u32*)(p+1));
		return 5;
	}
	//3e: ds
	//3f: unused

	//[40,4f]: rex?

	//[50,5f]: push,pop
	//[60,63]: unused
	//64: fs
	//65: gs

	//66: operand-size override prefix
	//67: address-size override prefix

	//68: push qword 0xxxxx
	if(0x68 == p[0]){
		disasm_x8664_print(p,5);
		printf("push	0x%llx\n", (s64)*(int*)(p+1));
		return 5;
	}
	//69: imul dst,src,dword 0x12345678
	if(0x69 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,6);
			printf("imul4	%s = %s / %x\n",
				reg32[bit3], reg32[bit0], *(u32*)(p+2));
			return 6;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+4);
		printf("imul4	%s = ", reg32[bit3]);
		if(0x05 != (p[1]&0xc7))printf("[%s]", tmp);
		else printf("[rel %llx]", (j+ret+4)+rel);
		printf(" / 0x%x\n", *(u32*)(p+ret));
		return ret+4;
	}
	if(0x6a == p[0]){
		disasm_x8664_print(p,2);
		printf("push	0x%llx\n", (s64)(char)p[1]);
		return 2;
	}
	//6b: imul dst,src,byte 34
	if(0x6b == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,3);
			printf("imul1	%s = %s / %d\n",
				reg32[bit3], reg32[bit0], (char)p[2]);
			return 3;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+1);
		printf("imul1	%s = ", reg32[bit3]);
		if(0x05 != (p[1]&0xc7))printf("[%s]", tmp);
		else printf("[rel %llx]", (j+ret+1)+rel);
		printf(" / %d\n", (char)p[ret]);
		return ret+1;
	}

	//6c: insb
	//6d: insd
	//6e: outsb
	//6f: outsd

	//j: [70,7f]
	if(0x70 == (p[0]&0xf0)){
		disasm_x8664_print(p,2);
		printf("j%s	0x%llx\n", cond[p[0]&0xf], (char)p[1] + (j+2));
		return 2;
	}

	//80:
	if(0x80 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,3);
			printf("%s	%s %s byte %x\n",
			op80[bit3], reg32[bit0], symbol[bit3], p[2]);
			return 3;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+1);
		printf("%s	", op80[bit3]);
		if(0x05 != (p[1]&0xc7))printf("byte@[%s]", tmp);
		else printf("byte@[rel %llx]", (j+ret+1)+rel);
		printf(" %s byte 0x%x\n", symbol[bit3], p[ret]);
		return ret+1;
	}
	//81:
	if(0x81 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,6);
			printf("%s	%s %s dword %x\n",
			op81[bit3], reg32[bit0], symbol[bit3], *(u32*)(p+2));
			return 6;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+4);
		printf("%s	", op81[bit3]);
		if(0x05 != (p[1]&0xc7))printf("dword@[%s]", tmp);
		else printf("dword@[rel %llx]", (j+ret+4)+rel);
		printf(" %s dword 0x%x\n", symbol[bit3], *(u32*)(p+ret));
		return ret+4;
	}
	//82: unused
	//83:
	if(0x83 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,3);
			printf("%s	%s %s byte %x\n",
			op83[bit3], reg32[bit0], symbol[bit3], p[2]);
			return 3;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+1);
		printf("%s	", op83[bit3]);
		if(0x05 != (p[1]&0xc7))printf("dword@[%s]", tmp);
		else printf("dword@[rel %llx]", (j+ret+1)+rel);
		printf(" %s byte 0x%x\n", symbol[bit3], p[ret]);
		return ret+1;
	}

	//84: test
	if(0x84 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("test	%s & %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("test	%s & [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x85 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("test	%s & %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("test	%s & [%s]\n", reg32[bit3], tmp);
		return ret;
	}
	//86: xchg
	if(0x86 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("xchg	%s, %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("xchg	%s, [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x87 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("xchg	%s, %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("xchg	%s, [%s]\n", reg32[bit3], tmp);
		return ret;
	}

	//[88,8a]: mov
	if(0x88 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("mov	%s = %s\n", reg08[bit0], reg08[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("mov	[%s] = %s\n", tmp, reg08[bit3]);
		return ret;
	}
	if(0x89 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("mov	%s = %s\n", reg32[bit0], reg32[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("mov	[%s] = %s\n", tmp, reg32[bit3]);
		return ret;
	}
	if(0x8a == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("mov	%s = %s\n", reg08[bit3], reg08[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("mov	%s = [%s]\n", reg08[bit3], tmp);
		return ret;
	}
	if(0x8b == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,2);
			printf("mov	%s = %s\n", reg32[bit3], reg32[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("mov	%s = [%s]\n", reg32[bit3], tmp);
		return ret;
	}

	//8c: mov xxx,cs/ds/es/gs...
	//8d: lea
	if(0x8d == p[0]){
		ret = disasm_x8664_sib(j, p, tmp, 0);
		disasm_x8664_print(p,ret);
		printf("lea	%s = %s\n", reg32[bit3], tmp);
		return ret;
	}
	//8e: mov cs/ds/es/gs..., xxx
	//8f: pop

	//[90,97]: xchg exx,exx
	//98: cwde
	//99: cdq
	//9a: unused
	//9b: wait
	//9c: pushf
	//9d: popf
	//9d: sahf
	//9d: lahf

	//mov al,[qword 0x]
	if(0xa0 == p[0]){
		disasm_x8664_print(p,8);
		printf("mov	al = [0x%llx]\n", *(u64*)(p+1));
		return 9;
	}
	if(0xa1 == p[0]){
		disasm_x8664_print(p,8);
		printf("mov	eax = [0x%llx]\n", *(u64*)(p+1));
		return 9;
	}
	if(0xa2 == p[0]){
		disasm_x8664_print(p,8);
		printf("mov	[0x%llx] = al\n", *(u64*)(p+1));
		return 9;
	}
	if(0xa3 == p[0]){
		disasm_x8664_print(p,8);
		printf("mov	[0x%llx] = eax\n", *(u64*)(p+1));
		return 9;
	}
	//a4: movsb
	//a5: movsd
	//a6: cmpsb
	//a7: cmpsd

	//test
	else if(0xa8 == p[0])
	{
		disasm_x8664_print(p,2);
		printf("test	al & 0x%x\n", p[1]);
		return 2;
	}
	else if(0xa9 == p[0])
	{
		disasm_x8664_print(p,5);
		printf("test	eax & 0x%x\n", *(u32*)(p+1));
		return 5;
	}

	//aa: stosb
	//ab: stosd
	//ac: lodsb
	//ad: lodsd
	//ae: scasb
	//af: scasd

	//[b0,b7]: mov8 r,0xff
	if(0xb0 == (p[0]&0xf8)){
		disasm_x8664_print(p,2);
		printf("mov1	%s = [0x%x]\n", reg08[p[0]&7], p[1]);
		return 2;
	}

	//[b8,bf]: mov32 r,0xff
	if(0xb8 == (p[0]&0xf8)){
		disasm_x8664_print(p,5);
		printf("mov4	%s = 0x%x\n", reg32[p[0]&7], *(u32*)(p+1));
		return 5;
	}

	//c0: shift byte
	if(0xc0 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,3);
			printf("%s	%s, byte %x\n",
			opc0[bit3], reg08[bit0], p[2]);
			return 3;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+1);
		printf("%s	", opc0[bit3]);
		if(0x05 != (p[1]&0xc7))printf("byte@[%s]", tmp);
		else printf("byte@[rel %llx]", (j+ret+1)+rel);
		printf(", byte 0x%x\n", p[ret]);
		return ret+1;
	}
	//c1: shift dword
	if(0xc1 == p[0]){
		if(3 == bit6){
			disasm_x8664_print(p,3);
			printf("%s	%s, byte %x\n",
			opc1[bit3], reg32[bit0], p[2]);
			return 3;
		}
		ret = disasm_x8664_sib(j, p, tmp, &rel);
		disasm_x8664_print(p,ret+1);
		printf("%s	", opc1[bit3]);
		if(0x05 != (p[1]&0xc7))printf("dword@[%s]", tmp);
		else printf("dword@[rel %llx]", (j+ret+1)+rel);
		printf(", byte 0x%x\n", p[ret]);
		return ret+1;
	}
	if(0xc2 == p[0]){
		disasm_x8664_print(p,3);
		printf("ret	0x%x\n", *(u16*)(p+1));
		return 3;
	}
	//c3: ret

	//c4: unused
	//c5: xmm

	//c6: mov byte
	if(0xc6 == p[0]){
		//c6f8: xabort
		if(0xf8 == p[1]){
			disasm_x8664_print(p,3);
			printf("xabort	0x%x\n", p[2]);
			return 3;
		}
		switch(p[1]&0xf8){
		case 0x00:
		case 0x40:
		case 0x80:{
			ret = disasm_x8664_sib(j, p, tmp, &rel);
			disasm_x8664_print(p,ret+1);
			printf("mov1	");
			if(0x05 != (p[1]&0xc7))printf("[%s]", tmp);
			else printf("[rel %llx]", (j+ret+1)+rel);
			printf(" = 0x%x\n", p[ret]);
			return ret+1;
		}
		case 0xc0:{
			disasm_x8664_print(p,3);
			printf("mov1	[%s] = 0x%x\n", reg08[p[1]&7], p[2]);
			return 3;
		}
		}//switch
	}
	//c7: mov dworld
	if(0xc7 == p[0]){
		//c6f8: xabort
		if(0xf8 == p[1]){
			disasm_x8664_print(p,3);
			printf("xbegin	0x%llx\n", (s64)(*(int*)(p+2)));
			return 6;
		}
		switch(p[1]&0xf8){
		case 0x00:
		case 0x40:
		case 0x80:{
			ret = disasm_x8664_sib(j, p, tmp, &rel);
			disasm_x8664_print(p,ret+4);
			printf("mov4	");
			if(0x05 != (p[1]&0xc7))printf("[%s]", tmp);
			else printf("[rel %llx]", (j+ret+4)+rel);
			printf(" = 0x%x\n", *(u32*)(p+ret));
			return ret+4;
		}
		case 0xc0:{
			disasm_x8664_print(p,3);
			printf("mov4	[%s] = 0x%x\n", reg32[p[1]&7], *(u32*)(p+2));
			return 3;
		}
		}//switch
	}

	if(0xc8 == p[0]){
		disasm_x8664_print(p,4);
		printf("enter	0x%x,0x%x\n", *(u16*)(p+1), p[3]);
		return 4;
	}
	//c9: leave

	if(0xca == p[0]){
		disasm_x8664_print(p,3);
		printf("retf	0x%x\n", *(u16*)(p+1));
		return 3;
	}
	//cb: retf

	//cc: int3
	if(0xcd == p[0]){
		disasm_x8664_print(p,2);
		printf("int	0x%x\n", p[1]);
		return 2;
	}
	//ce: unused
	//cf: iret

	//[d0,d3]: shift
	//d4,d5: unused
	//d6:salc
	//d7:xlatb

	//[d8,df]:fadd,fmul,f***

	//[e0,e3]: loop*
	if(0xe0 == p[0]){
		disasm_x8664_print(p,2);
		printf("loopne	0x%llx\n", (char)p[1] + (j+2));
		return 2;
	}
	if(0xe1 == p[0]){
		disasm_x8664_print(p,2);
		printf("loope	0x%llx\n", (char)p[1] + (j+2));
		return 2;
	}
	if(0xe2 == p[0]){
		disasm_x8664_print(p,2);
		printf("loop	0x%llx\n", (char)p[1] + (j+2));
		return 2;
	}
	if(0xe3 == p[0]){
		disasm_x8664_print(p,2);
		printf("jrcxz	0x%llx\n", (char)p[1] + (j+2));
		return 2;
	}

	//in
	if(0xe4 == p[0]){
		disasm_x8664_print(p,2);
		printf("in	al = port[0x%x]\n", p[1]);
		return 2;
	}
	if(0xe5 == p[0]){
		disasm_x8664_print(p,2);
		printf("eax	eax = port[0x%x]\n", p[1]);
		return 2;
	}

	//out
	if(0xe6 == p[0]){
		disasm_x8664_print(p,2);
		printf("out	port[0x%x] = al\n", p[1]);
		return 2;
	}
	if(0xe7 == p[0]){
		disasm_x8664_print(p,2);
		printf("out	port[0x%x] = eax\n", p[1]);
		return 2;
	}

	if(0xe8 == p[0]){
		disasm_x8664_print(p,5);
		printf("call	0x%llx\n", (u64)(*(int*)(p+1) + (j+5)));
		return 5;
	}
	if(0xe9 == p[0]){
		disasm_x8664_print(p,5);
		printf("jmp	0x%llx\n", (u64)(*(int*)(p+1) + (j+5)));
		return 5;
	}
	//ea:unused
	if(0xeb == p[0]){
		disasm_x8664_print(p,2);
		printf("jmp	0x%llx\n", (u64)((char)p[1] + (j+2)));
		return 2;
	}
	//ec: in al,dx
	//ed: in eax,dx
	//ee: out dx,al
	//ef: out dx,eax

	//f0: lock
	//f1: int1
	//f2: repne
	//f3: rep
	//f4: hlt
	//f5: cmc

	//f6: 8bit
	if(0xf6 == p[0]){
		if(3 == bit6){
			if(0 == bit3){
				disasm_x8664_print(p,3);
				printf("%s	%s & %x\n", opf6[bit3], reg08[bit0], p[2]);
				return 3;
			}//0
			if(1 != bit3){
				disasm_x8664_print(p,2);
				printf("%s	%s\n", opf6[bit3], reg08[bit0]);
				return 2;
			}//234567
		}//test reg8, reg8
		else{
			if(0 == bit3){
				ret = disasm_x8664_sib(j, p, tmp, &rel);
				disasm_x8664_print(p,ret+1);
				printf("test1	");
				if(0x05 != (p[1]&0xc7))printf("[%s]", tmp);
				else printf("[rel %llx]", (j+ret+1)+rel);
				printf(" & 0x%x\n", p[ret]);
				return ret+1;
			}//0
			if(1 != bit3){
				ret = disasm_x8664_sib(j, p, tmp, 0);
				disasm_x8664_print(p,ret);
				printf("%s	[%s]\n", opf6[bit3], tmp);
				return ret;
			}//234567
		}//else
	}//f6
	//f7: 32bit
	if(0xf7 == p[0]){
		if(3 == bit6){
			if(0 == bit3){
				disasm_x8664_print(p,6);
				printf("%s	%s & %x\n", opf7[bit3], reg32[bit0], *(u32*)(p+2));
				return 6;
			}//0
			if(1 != bit3){
				disasm_x8664_print(p,2);
				printf("%s	%s\n", opf7[bit3], reg32[bit0]);
				return 2;
			}//234567
		}//test reg32, reg32
		else{
			if(0 == bit3){
				ret = disasm_x8664_sib(j, p, tmp, &rel);
				disasm_x8664_print(p,ret+4);
				printf("test4	");
				if(0x05 != (p[1]&0xc7))printf("[%s]", tmp);
				else printf("[rel %llx]", (j+ret+4)+rel);
				printf(" & 0x%x\n", *(u32*)(p+ret));
				return ret+4;
			}//0
			if(1 != bit3){
				ret = disasm_x8664_sib(j, p, tmp, 0);
				disasm_x8664_print(p,ret);
				printf("%s	[%s]\n", opf7[bit3], tmp);
				return ret;
			}//234567
		}//else
	}//f7

	//f8: clc
	//f9: stc
	//fa: cli
	//fb: sti
	//fc: cld
	//fd: unused

	//fe: 8bit
	if(0xfe == p[0]){
		switch(p[1]&0xf0){
		case 0xc0:{	//inc,dec
			disasm_x8664_print(p,2);
			printf("%s	%s\n", opfe[bit3], reg08[bit0]);
			return 2;
		}
		case 0x00:
		case 0x40:
		case 0x80:{
			ret = disasm_x8664_sib(j, p, tmp, 0);
			disasm_x8664_print(p,ret);
			printf("%s	[%s]\n", opfe[bit3], tmp);
			return ret;
		}
		}//switch
	}
	//ff: 32bit
	if(0xff == p[0]){
		switch(p[1]&0xf8){
		case 0xc0:	//inc4 reg
		case 0xc8:	//dec4 reg
		case 0xd0:	//call8 reg
		case 0xe0:	//jmp8 reg
		case 0xf0:{	//push8 reg
			disasm_x8664_print(p,2);
			printf("%s	%s\n", opff[bit3], reg32[bit0]);
			return 2;
		}

		case 0x00:	//inc4
		case 0x08:	//dec4
		case 0x10:	//call8
		case 0x18:	//call4
		case 0x20:	//jmp8
		case 0x28:	//jmp4
		case 0x30:	//push8

		case 0x40:	//inc4
		case 0x48:	//dec4
		case 0x50:	//call8
		case 0x58:	//call4
		case 0x60:	//jmp8
		case 0x68:	//jmp4
		case 0x70:	//push8

		case 0x80:	//inc4
		case 0x88:	//dec4
		case 0x90:	//call8
		case 0x98:	//call4
		case 0xa0:	//jmp8
		case 0xa8:	//jmp4
		case 0xb0:{	//push8
			ret = disasm_x8664_sib(j, p, tmp, 0);
			disasm_x8664_print(p,ret);
			printf("%s	[%s]\n", opff[bit3], tmp);
			return ret;
		}
		}//switch
	}

	return 0;
}
int disasm_x8664_0f(u64 rip, u8* p)
{
	if(0x1f == p[1])
	{
		if(0 == p[2])
		{
			disasm_x8664_print(p,3);
			printf("nop3\n");
			return 3;
		}
		else if(0x84 == p[2])
		{
			disasm_x8664_print(p,8);
			printf("nop8\n");
			return 8;
		}
		else if(0x80 == p[2])
		{
			disasm_x8664_print(p,7);
			printf("nop7\n");
			return 7;
		}
		else if(0x44 == p[2])
		{
			disasm_x8664_print(p,5);
			printf("nop5\n");
			return 5;
		}
		else
		{
			disasm_x8664_print(p,4);
			printf("nop4\n");
			return 4;
		}
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm0\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm1\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0x54 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm2\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0x9c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm3\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0xa4 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm4\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0xac == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm5\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0xb4 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm6\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x29 == p[1])&&(0xbc == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm7\n", p[4]);
		return 5;
	}
	else if((0x0f == p[0])&&(0x47 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("if(a)%s = %s\n", reg32[(p[2]&0x3f)>>3], reg32[p[2]&0x7]);
		return 3;
	}
	else if((0x0f == p[0])&&(0x80 == (p[1]&0xf0)))
	{
		disasm_x8664_print(p,6);
		if(0x80 == p[1])printf("if(o)");
		else if(0x81 == p[1])printf("if(no)");
		else if(0x82 == p[1])printf("if(c)");
		else if(0x83 == p[1])printf("if(nc)");
		else if(0x84 == p[1])printf("if(z)");
		else if(0x85 == p[1])printf("if(nz)");
		else if(0x86 == p[1])printf("if(na)");
		else if(0x87 == p[1])printf("if(a)");
		else if(0x88 == p[1])printf("if(s)");
		else if(0x89 == p[1])printf("if(ns)");
		else if(0x8a == p[1])printf("if(pe)");
		else if(0x8b == p[1])printf("if(po)");
		else if(0x8c == p[1])printf("if(l)");
		else if(0x8d == p[1])printf("if(nl)");
		else if(0x8e == p[1])printf("if(ng)");
		else if(0x8f == p[1])printf("if(g)");
		printf("jmp 0x%llx\n", *(int*)(p+2) + (rip+6));
		return 6;
	}
	else if((0x0f == p[0])&&(0x94 == p[1])&&(0xc1 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("setz cl\n");
		return 3;
	}
	else if((0x0f == p[0])&&(0xb6 == p[1])&&(0x02 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("eax = [rdx, 1]\n");
		return 3;
	}
	else if((0x0f == p[0])&&(0xb6 == p[1])&&(0x3b == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("edi = [rbx,1]\n");
		return 3;
	}
	else if((0x0f == p[0])&&(0xb6 == p[1])&&(0x75 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("esi = [rbp + 0x%x,1]\n", p[3]);
		return 4;
	}
	else if((0x0f == p[0])&&(0xb7 == p[1])&&(0x6a == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("ebp = [rdx+0x%x, 2]\n", p[3]);
		return 4;
	}
	else if((0x0f == p[0])&&(0xb7 == p[1])&&(0xc0 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("eax = ax\n");
		return 3;
	}
	else if((0x0f == p[0])&&(0xbe == p[1])&&(0x0a == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("ecx = (s32)[rdx,1]\n");
		return 3;
	}
	else if((0x0f == p[0])&&(0xbe == p[1])&&(0xc8 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("ecx = (s32)al\n");
		return 3;
	}

	disasm_x8664_print(p,1);
	printf("0f	error\n");
	return 1;
}
int disasm_x8664_4x(u64 rip, u8* p)
{
	disasm_x8664_print(p,1);
	printf("error\n");
	return 1;
}
int disasm_x8664_66(u64 rip, u8* p)
{
	if((0x0f == p[1])&&(0x1f == p[2]))
	{
		if(0 == p[3])
		{
			disasm_x8664_print(p,4);
			printf("nop4\n");
			return 4;
		}
		else if(0x84 == p[3])
		{
			disasm_x8664_print(p,9);
			printf("nop9\n");
			return 9;
		}
		else if(0x80 == p[3])
		{
			disasm_x8664_print(p,8);
			printf("nop8\n");
			return 8;
		}
		else if(0x44 == p[3])
		{
			disasm_x8664_print(p,6);
			printf("nop6\n");
			return 6;
		}
		else
		{
			disasm_x8664_print(p,5);
			printf("nop5\n");
			return 5;
		}
	}
	if((0x09 == p[0])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s |= %s\n", reg16[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x2e == p[1])&&(0x0f == p[2])&&(0x1f == p[3]))
	{
		disasm_x8664_print(p,10);
		printf("nop10\n");
		return 10;
	}
	if((0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s ^= %s\n", reg16[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s ? %s\n", reg16[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[1])&&(0x89 == p[2])&&(0x0 == (p[3]&0xc0)))
	{
		disasm_x8664_print(p,4);
		printf("[%d] = %s\n", 8+(p[3]&0x7), reg16[(p[3]&0x3f)>>3]);
		return 4;
	}
	if((0x41 == p[1])&&(0x89 == p[2])&&(0x41 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[r9+0x%x] = ax\n", p[4]);
		return 5;
	}
	if((0x83 == p[1])&&(0xc8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s |= 0x%x\n", reg16[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x83 == p[1])&&(0xf8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s ? 0x%x\n", reg16[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x89 == p[1])&&(0x0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("[%s] = %s\n", reg64[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x89 == p[1])&&(0x42 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[rdx+0x%x] = ax\n", p[3]);
		return 4;
	}
	if(0x90 == p[1])
	{
		disasm_x8664_print(p,2);
		printf("xchg ax,ax\n");
		return 2;
	}

	disasm_x8664_print(p,1);
	printf("error\n");
	return 1;
}
int disasm_x8664_67(u64 rip, u8* p)
{
	disasm_x8664_print(p,1);
	printf("error\n");
	return 1;
}
void disasm_x8664_one(u8* buf, int len)
{
	int j,k;

	j = 0;
	while(1)
	{
		if(j>=len)break;
		printf("%8x:	", j);

		if(disasm_x8664_1b(buf+j)){
			j += 1;
			continue;
		}

		if(0x0f == buf[j]){
			j += disasm_x8664_0f(j, buf+j);
			continue;
		}
		if(0x40 == (buf[j]&0xf0)){
			j += disasm_x8664_4x(j, buf+j);
			continue;
		}
		if(0x66 == buf[j]){
			j += disasm_x8664_66(j, buf+j);
			continue;
		}
		if(0x67 == buf[j]){
			j += disasm_x8664_67(j, buf+j);
			continue;
		}

		k = disasm_x8664_normal(j, buf+j, 0);
		if(k > 0){
			j += k;
			continue;
		}

		disasm_x8664_print(buf+j,1);
		printf("error\n");
		j++;
	}//while
}
void disasm_x8664(int argc, char** argv)
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
	disasm_x8664_one(buf, ret);

release:
	free(buf);
theend:
	close(fd);
}




/*
		else if((0x80 == p[0])&&(0x3d == p[1]))
		{
			disasm_x8664_print(p,7);
			printf("[rip+0x%x,1] ? 0x%x\n", *(u32*)(p+2), p[6]);
			j += 7;
		}
		else if((0x80 == p[0])&&(0x4a == p[1]))
		{
			disasm_x8664_print(p,4);
			printf("[rdx + 0x%x, 1] |= 0x80\n", p[2]);
			j += 4;
		}
		else if((0x80 == p[0])&&(0xc8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s |= 0x%x\n", reg08[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x80 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s ? 0x%x\n", reg08[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x81 == p[0])&&(0xc8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,6);
			printf("%s |= 0x%x\n", reg32[p[1]&0x7], *(u32*)(p+2));
			j += 6;
		}
		else if((0x81 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,6);
			printf("%s &= 0x%x\n", reg32[p[1]&0x7], *(u32*)(p+2));
			j += 6;
		}
		else if((0x81 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,6);
			printf("%s ? 0x%x\n", reg32[p[1]&0x7], *(u32*)(p+2));
			j += 6;
		}
		else if((0x83 == p[0])&&(0xc0 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s += 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x83 == p[0])&&(0xc8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s |= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x83 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s &= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x83 == p[0])&&(0xe8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s -= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x83 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s ? 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0x8d == p[0])&&(0x04 == p[1])&&(0x13 == p[2]))
		{
			disasm_x8664_print(p,3);
			printf("eax = rbx+rdx\n");
			j += 3;
		}
		else if((0x8d == p[0])&&(0x0c == p[1])&&(0x02 == p[2]))
		{
			disasm_x8664_print(p,3);
			printf("ecx = rdx+rax\n");
			j += 3;
		}
		else if((0x8d == p[0])&&(0x0c == p[1])&&(0x18 == p[2]))
		{
			disasm_x8664_print(p,3);
			printf("ecx = rax+rbx\n");
			j += 3;
		}
		else if((0x8d == p[0])&&(0x14 == p[1])&&(0x06 == p[2]))
		{
			disasm_x8664_print(p,3);
			printf("edx = rsi+rax\n");
			j += 3;
		}
		else if((0x8d == p[0])&&(0x40 == (p[1]&0xc0)))
		{
			disasm_x8664_print(p,3);
			printf("%s = %s", reg32[(p[1]&0x3f)>>3], reg64[p[1]&0x7]);
			if(p[2] < 0x80)printf(" + 0x%x\n", p[2]);
			else printf(" - 0x%x\n", 0x100-p[2]);
			j += 3;
		}
		else if((0x8d == p[0])&&(0x84 == p[1])&&(0x10 == p[2]))
		{
			disasm_x8664_print(p,7);
			printf("eax = rax + rdx + 0x%x\n", *(u32*)(p+3));
			j += 7;
		}
		else if(0xb8 == (p[0]&0xf8))
		{
			disasm_x8664_print(p,5);
			printf("%s = 0x%x\n", reg32[p[0]&0x7], *(u32*)(p+1));
			j += 5;
		}
		else if((0xc1 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("sar %s,0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0xc1 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s <<= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if((0xc1 == p[0])&&(0xe8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("%s >>= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 3;
		}
		else if(0xc6 == p[0])
		{
			if((0x04 == p[1])&&(0x17 == p[2]))
			{
				disasm_x8664_print(p,4);
				printf("[rdi+rdx] = 0x%x\n", p[3]);
				j += 4;
			}
			else if(0x42 == p[1])
			{
				disasm_x8664_print(p,4);
				printf("[rdx + 0x%x, 1] = 0x%x\n", p[2], p[3]);
				j += 4;
			}
			else if(0x5 == p[1])
			{
				disasm_x8664_print(p,7);
				printf("[rip + 0x%x, 1] = 0x%x\n", *(u32*)(p+2), p[6]);
				j += 7;
			}
			else
			{
				disasm_x8664_print(p,3);
				printf("[%s,1] = 0x%x\n", reg64[p[1]&0x7], p[2]);
				j += 3;
			}
		}
		else if((0xc7 == p[0])&&(0x43 == p[1]))
		{
			disasm_x8664_print(p,7);
			printf("[rbx + 0x%x, 4] = 0x%x\n", p[2], *(u32*)(p+3));
			j += 7;
		}
		else if((0xc7 == p[0])&&(0x44 == p[1])&&(0x24 == p[2]))
		{
			disasm_x8664_print(p,8);
			printf("[rsp + 0x%x,4] = 0x%x\n", p[3], *(u32*)(p+4));
			j += 8;
		}
		else if((0xd1 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,2);
			printf("%s <<= 1\n", reg32[p[1]&0x7]);
			j += 2;
		}
		else if((0xd1 == p[0])&&(0xe8 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,2);
			printf("%s >>= 1\n", reg32[p[1]&0x7]);
			j += 2;
		}
		else if((0xd3 == p[0])&&(0xe0 == (p[1]&0xf0)))
		{
			disasm_x8664_print(p,2);
			printf("%s <<= cl\n", reg32[p[1]&0x7]);
			j += 2;
		}
		else if((0xd3 == p[0])&&(0xe8 == (p[1]&0xf0)))
		{
			disasm_x8664_print(p,2);
			printf("%s >>= cl\n", reg32[p[1]&0x7]);
			j += 2;
		}
		else if((0xf2 == p[0])&&(0x0f == p[1])&&(0x10 == p[2])&&(0x01 == p[3]))
		{
			disasm_x8664_print(p,4);
			printf("xmm0 = [rcx]\n");
			j += 4;
		}
		else if((0xf6 == p[0])&&(0xc0 == (p[1]&0xf8)))
		{
			disasm_x8664_print(p,3);
			printf("flag = %s & 0x%x\n", reg08[p[1]&0x7], p[2]);
			j += 3;
		}
*/




/*
	if((0x40 == p[0])&&(0x0f == p[1])&&(0x94 == p[2])&&(0xc6 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("setz sil\n");
		return 4;
	}
	if((0x40 == p[0])&&(0x0f == p[1])&&(0xb6 == p[2])&&(0xf5 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("esi = bpl\n");
		return 4;
	}
	if((0x40 == p[0])&&(0x80 == p[1])&&(0xfe == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("sil ? 0x%x\n", p[3]);
		return 4;
	}
	if((0x40 == p[0])&&(0x80 == p[1])&&(0xff == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("dil ? 0x%x\n", p[3]);
		return 4;
	}
	if((0x40 == p[0])&&(0x84 == p[1])&&(0xce == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("flag = sil & cl\n");
		return 3;
	}
	if((0x40 == p[0])&&(0xf6 == p[1])&&(0xc6 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("sil ? 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x0f == p[1])&&(0x4d == p[2])&&(0xc0 == (p[3]&0xc0)))
	{
		disasm_x8664_print(p,4);
		printf("if(nl)%s = r%dd\n", reg32[(p[3]&0x3f)>>3], 8+(p[3]&7));
		return 4;
	}
	if((0x41 == p[0])&&(0x0f == p[1])&&(0xb6 == p[2])&&(0x0 == (p[3]&0xc0)))
	{
		if(4 == (p[3]&0x7))
		{
			disasm_x8664_print(p,5);
			if(0x24 == p[4])
			{
				printf("%s = [r%d,1]\n", reg32[p[3]>>3], 8+(p[3]&0x7));
			}
			else
			{
				printf("%s = [r%d + %s]\n",
					reg32[p[3]>>3],
					8+(p[4]&0x7),
					reg64[(p[4]&0x38)>>3]
				);
			}
			return 5;
		}
		else
		{
			disasm_x8664_print(p,4);
			printf("%s = [r%d,1]\n", reg32[p[3]>>3], 8+(p[3]&0x7));
			return 4;
		}
	}//410FB6341F        movzx esi,byte [r15+rbx]
	if((0x41 == p[0])&&(0x0f == p[1])&&(0xb6 == p[2])&&(0x45 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("eax = [r13+0x%x,1]\n", p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x7a == p[3]))
	{
		disasm_x8664_print(p,5);
		if(p[4] < 0x80)printf("edi = (s32)[r10 + 0x%x, 1]\n", p[4]);
		else printf("edi = (s32)[r10 - 0x%x, 1]\n", 0x100-p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x0c == p[3])&&(0x06 == p[4]))
	{
		disasm_x8664_print(p,5);
		printf("ecx = (s32)[r14+rax,1]\n");
		return 5;
	}
	if((0x41 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x3c == p[3])&&(0x36 == p[4]))
	{
		disasm_x8664_print(p,5);
		printf("edi = (s32)[r14+rsi,1]\n");
		return 5;
	}
	if((0x41 == p[0])&&(0x29 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%dd -= %s\n", 8+(p[2]&7), reg32[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%dd ? %s\n", 8+(p[2]&7), reg32[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[0])&&(0x50 == (p[1]&0xf8)))
	{
		disasm_x8664_print(p,2);
		printf("push r%d\n", 8+(p[1]&0x7));
		return 2;
	}
	if((0x41 == p[0])&&(0x58 == (p[1]&0xf8)))
	{
		disasm_x8664_print(p,2);
		printf("pop r%d\n", 8+(p[1]&0x7));
		return 2;
	}
	if((0x41 == p[0])&&(0x80 == p[1])&&(0x3c == p[2])&&(0x04 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[r12+rax,1] ? 0x%x\n", p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0x80 == p[1])&&(0x49 == p[2]))
	{
		disasm_x8664_print(p,5);
		printf("[r9+0x%x] |= 0x%x\n", p[3], p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0x83 == p[1])&&(0xc0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r%dd += 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x83 == p[1])&&(0xe6 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r14d &= 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x83 == p[1])&&(0xe8 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r8d -= 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x83 == p[1])&&(0xf9 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r9d ? 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x83 == p[1])&&(0xfd == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r13d ? 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x85 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("flag = r%dd & %s\n", 8+(p[2]&0x7), reg32[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[0])&&(0x88 == p[1])&&(0x0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("[r%d] = %s\n", 8+(p[2]&0x7), reg08[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[0])&&(0x88 == p[1])&&(0x41 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[r9 + 0x%x] = al\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x88 == p[1])&&(0x49 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[r9 + 0x%x] = cl\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x88 == p[1])&&(0x71 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[r9 + 0x%x] = sil\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x89 == p[1])&&(0x0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("[%d] = %s\n", 8+(p[2]&0x7), reg32[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[0])&&(0x89 == p[1])&&(0x50 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[r8+0x%x],edx\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%dd = %s\n", 8+(p[2]&0x7), reg32[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x41 == p[0])&&(0x8b == p[1])&&(0x10 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("edx = [r8]\n");
		return 3;
	}
	if((0x41 == p[0])&&(0x8b == p[1])&&(0x40 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,4);
		printf("%s = [r%d + 0x%x]\n", reg32[(p[2]&0x3f)>>3], 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x8d == p[1])&&(0x45 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("eax = r13 + 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0x8d == p[1])&&(0x6c == p[2])&&(0x35 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("ebp = r13 + rsi + 0x%x\n", p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0xc1 == p[1])&&(0xe0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r%dd <<= 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0xc1 == p[1])&&(0xe8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r%dd >>= 0x%x\n", p[2]&0x7, p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0xc1 == p[1])&&(0xfd == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("sar r13d, 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0xc6 == p[1])&&(0x01 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[r9, 1] = 0x%x\n", p[3]);
		return 4;
	}
	if((0x41 == p[0])&&(0xc6 == p[1])&&(0x04 == p[2])&&(0x0c == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[r12+rcx,1] = 0x%x\n", p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0xc6 == p[1])&&(0x41 == p[2]))
	{
		disasm_x8664_print(p,5);
		printf("[r9 + 0x%x, 1] = 0x%x\n", p[3], p[4]);
		return 5;
	}
	if((0x41 == p[0])&&(0xc7 == p[1])&&(0x40 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,8);
		printf("[r%d + 0x%x] = 0x%x\n", 8+(p[2]&0x7), p[3], *(u32*)(p+4));
		return 8;
	}
	if((0x41 == p[0])&&(0xf6 == p[1])&&(0xc4 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r12b ? 0x%x\n", p[3]);
		return 4;
	}
	if((0x43 == p[0])&&(0x8d == p[1])&&(0x74 == p[2])&&(0xad == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("esi = r13 + r13*4 + 0x%x\n", p[4]);
		return 5;
	}
	if((0x44 == p[0])&&(0x01 == p[1])&&(0xea == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("edx += r13d\n");
		return 3;
	}
	if((0x44 == p[0])&&(0x29 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s -= r%dd\n", reg32[p[2]&0x7], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x44 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s ? r%dd\n", reg32[p[2]&0x7], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x44 == p[0])&&(0x85 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("flag = %s & r%dd\n", reg32[(p[2]&0x7)], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x44 == p[0])&&(0x89 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = r9d\n", p[4]);
		return 5;
	}
	if((0x44 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s = r%dd\n", reg32[p[2]&0x7], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x44 == p[0])&&(0x8b == p[1])&&(0x05 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("r8d = [rip + 0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x44 == p[0])&&(0x8b == p[1])&&(0x3b == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("r15d = [rbx]\n");
		return 3;
	}
	if((0x44 == p[0])&&(0x8b == p[1])&&(0x4a == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r9d = [rdx+0x%x]\n", p[3]);
		return 4;
	}
	if((0x44 == p[0])&&(0x8b == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("r9d = [rsp + 0x%x]\n", p[4]);
		return 5;
	}
	if((0x44 == p[0])&&(0x8d == p[1])&&(0x34 == p[2])&&(0x03 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("r14d = rbx + rax\n");
		return 4;
	}
	if((0x44 == p[0])&&(0x8d == p[1])&&(0x6c == p[2])&&(0x77 == p[3]))
	{
		disasm_x8664_print(p,5);
		if(p[4] < 0x80)printf("r13d = rdi + rsi*2 + 0x%x\n", p[4]);
		else printf("r13d = rdi + rsi*2 - 0x%x\n", 0x100-p[4]);
		return 5;
	}
	if((0x44 == p[0])&&(0x8d == p[1])&&(0x40 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,4);
		printf("r%dd = %s", 8+((p[2]&0x3f)>>3), reg64[p[2]&0x7]);
		if(p[3] < 0x80)printf(" + 0x%x\n", p[3]);
		else printf(" - 0x%x\n", 0x100-p[3]);
		return 4;
	}
	if((0x45 == p[0])&&(0x0f == p[1])&&(0x49 == p[2])&&(0xee == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("if(ns)r13d = r14d\n");
		return 4;
	}
	if((0x45 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x0c == p[3])&&(0x36 == p[4]))
	{
		disasm_x8664_print(p,5);
		printf("r9d = [r14+rsi,1]\n");
		return 5;
	}
	if((0x45 == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%dd ^= r%dd\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x45 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%dd ? r%dd\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x45 == p[0])&&(0x85 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("flag = r%dd & r%dd\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x45 == p[0])&&(0x89 == p[1])&&(0x13 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("[r11] = r10d\n");
		return 3;
	}
	if((0x45 == p[0])&&(0x8d == p[1])&&(0x68 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r13d = r%d + 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x45 == p[0])&&(0x8d == p[1])&&(0x7d == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("r15d = r13 + 0x%x\n", p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s += %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x48 == p[0])&&(0x03 == p[1])&&(0x05 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rax += [+0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x03 == p[1])&&(0x1d == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rbx += [rip+0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x09 == p[0])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s |= %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x48 == p[0])&&(0x0f == p[1])&&(0x4f == p[2])&&(0x05 == p[3]))
	{
		disasm_x8664_print(p,8);
		printf("if(g)rax = [+%x]\n", *(u32*)(p+4));
		return 8;
	}
	if((0x48 == p[0])&&(0x0f == p[1])&&(0x4f == p[2])&&(0xc2 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("if(g)rax = rdx\n");
		return 4;
	}
	if((0x48 == p[0])&&(0x03 == p[1])&&(0x4d == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("rcx += [rbp + 0x%x]\n", p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x21 == p[1])&&(0xf8 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("rax &= rdi\n");
		return 3;
	}
	if((0x48 == p[0])&&(0x29 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s -= %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x48 == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s ^= %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x48 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s ? %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x48 == p[0])&&(0x3d == p[1]))
	{
		disasm_x8664_print(p,6);
		printf("rax ? 0x%x\n", *(u32*)(p+2));
		return 6;
	}
	if((0x48 == p[0])&&(0x63 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s = (s64)%s\n", reg64[(p[2]&0x3f)>>3], reg32[p[2]&0x7]);
		return 3;
	}
	if((0x48 == p[0])&&(0x81 == p[1])&&(0xc0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,7);
		printf("%s += 0x%x\n", reg64[p[2]&0x7], *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x81 == p[1])&&(0xec == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rsp -= 0x%x\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x83 == p[1])&&(0xc0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		if(p[3] < 0x80)printf("%s += 0x%x\n", reg64[p[2]&0x7], p[3]);
		else printf("%s -= 0x%x\n", reg64[p[2]&0x7], 0x100-p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x83 == p[1])&&(0xc8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s |= 0x%x\n", reg64[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x83 == p[1])&&(0xe8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s -= %x\n", reg64[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x83 == p[1])&&(0xf8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s ? 0x%x\n", reg64[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x02 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("[rdx] = rax\n");
		return 3;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x05 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("[+%x] = rax\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x45 == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("[rbp+0x%x] = rax\n", p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x3d == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("[+%x] = rdi\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = rax\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = rcx\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x54 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = rdx\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x74 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = rsi\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0x7c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = rdi\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s = %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x35 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rsi = [+%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x05 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rax = [+%llx]\n", *(int*)(p+3) + (rip+7));
		return 7;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x15 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rdx = [+0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x18 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("rbx = [rax]\n");
		return 3;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x39 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("rdi = [rcx]\n");
		return 3;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x3d == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rdi = [+0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x8b == p[1])&&(0x4d == p[2]))
	{
		disasm_x8664_print(p,4);
		printf("rcx = [rbp + 0x%x]\n", p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0x3d == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("rdi = rip + 0x%x\n", *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0x41 == p[2]))
	{
		disasm_x8664_print(p,4);
		if(p[3] < 0x80)printf("rax = rcx + 0x%x\n", p[3]);
		else printf("rax = rcx - 0x%x\n", 0x100 - p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("rax = rsp + 0x%x\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0x6c == p[2])&&(0x07 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("rbp = rdi + rax + 0x%x\n", p[4]);
		return 5;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0x84 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,8);
		printf("rax = [rsp + 0x%x]\n", p[4]);
		return 8;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0x80 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,7);
		printf("rax = %s + 0x%x\n", reg64[p[2]&0x7], *(u32*)(p+3));
		return 7;
	}
	if((0x48 == p[0])&&(0x8d == p[1])&&(0xbc == p[2])&&(0x17 == p[3]))
	{
		disasm_x8664_print(p,8);
		printf("rdi = rdi + rdx + 0x%x\n", *(u32*)(p+4));
		return 8;
	}
	if((0x48 == p[0])&&(0x98 == p[1]))
	{
		disasm_x8664_print(p,2);
		printf("cdqe\n");
		return 2;
	}
	if((0x48 == p[0])&&(0xc1 == p[1])&&(0xe0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s <<= %x\n", reg64[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0xc1 == p[1])&&(0xe8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("%s >>= %x\n", reg64[p[2]&0x7], p[3]);
		return 4;
	}
	if((0x48 == p[0])&&(0xc7 == p[1])&&(0x05 == p[2]))
	{
		disasm_x8664_print(p,11);
		printf("[+%x] = %x\n", *(u32*)(p+3), *(u32*)(p+7));
		return 11;
	}
	if((0x49 == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%d += %s\n", 8+(p[2]&0x7), reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x49 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,3);
		printf("r%d ? %s\n", 8+(p[2]&0x7), reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x49 == p[0])&&(0x63 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s = (s64)r%dd\n", reg64[(p[2]&0x3f)>>3], 8+(p[2]&0x7));
		return 3;
	}
	if((0x49 == p[0])&&(0x81 == p[1])&&(0xc2 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("r10 += 0x%x\n", *(u32*)(p+3));
		return 7;
	}
	if((0x49 == p[0])&&(0x83 == p[1])&&(0xc0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r%d += 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x49 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%d = %s\n", 8+(p[2]&0x7), reg64[(p[2]&0x3f)>>3]);
		return 3;
	}
	if((0x49 == p[0])&&(0x8d == p[1])&&(0x04 == p[2])&&(0x3c == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("rax = r12+rdi\n");
		return 4;
	}
	if((0x49 == p[0])&&(0x8d == p[1])&&(0x3c == p[2])&&(0x04 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("rdi = r12+rax\n");
		return 4;
	}
	if((0x49 == p[0])&&(0x8d == p[1])&&(0x58 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("rbx = r%d + 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x49 == p[0])&&(0xc1 == p[1])&&(0xe0 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r%d <<= 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x49 == p[0])&&(0xc1 == p[1])&&(0xe8 == (p[2]&0xf8)))
	{
		disasm_x8664_print(p,4);
		printf("r%d >>= 0x%x\n", 8+(p[2]&0x7), p[3]);
		return 4;
	}
	if((0x4b == p[0])&&(0x8d == p[1])&&(0x3c == p[2])&&(0x2c == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("rdi = r12+r13\n");
		return 4;
	}
	if((0x4a == p[0])&&(0x8d == p[1])&&(0x4c == p[2])&&(0x1f == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("rcx = rdi + r11 + 0x%x\n", p[4]);
		return 5;
	}
	if((0x4c == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s += r%d\n", reg64[p[2]&0x7], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x4c == p[0])&&(0x03 == p[1])&&(0x05 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("r8 += [rip + 0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x4c == p[0])&&(0x0f == p[1])&&(0x4c == p[2])&&(0xe8 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("if(l)r13 = rax\n");
		return 4;
	}
	if((0x4c == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s ? r%d\n", reg64[(p[2]&0x7)], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x4c == p[0])&&(0x63 == p[1])&&(0xef == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("r13 = (s64)edi\n");
		return 3;
	}
	if((0x4c == p[0])&&(0x89 == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = r8\n", p[4]);
		return 5;
	}
	if((0x4c == p[0])&&(0x89 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("[rsp + 0x%x] = r9\n", p[4]);
		return 5;
	}
	if((0x4c == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("%s = r%d\n", reg64[p[2]&0x7], 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x4c == p[0])&&(0x8b == p[1])&&(0x25 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("r12 = [+0x%x]\n", *(u32*)(p+3));
		return 7;
	}
	if((0x4c == p[0])&&(0x8b == p[1])&&(0x29 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("r13 = [rcx]\n");
		return 3;
	}
	if((0x4c == p[0])&&(0x8b == p[1])&&(0x7c == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("r15 = rsp + 0x%x\n", p[4]);
		return 5;
	}
	if((0x4c == p[0])&&(0x8d == p[1])&&(0x0c == p[2])&&(0x02 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("r9 = [rdx+rax]\n");
		return 4;
	}
	if((0x4c == p[0])&&(0x8d == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
	{
		disasm_x8664_print(p,5);
		printf("r8 = rsp + 0x%x\n", p[4]);
		return 5;
	}
	if((0x4d == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%d += r%d\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x4d == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%d ^= r%d\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x4d == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
	{
		disasm_x8664_print(p,3);
		printf("r%d ? r%d\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
		return 3;
	}
	if((0x4d == p[0])&&(0x63 == p[1])&&(0xc0 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("r8 = (s64)r8d\n");
		return 3;
	}
	if((0x4d == p[0])&&(0x8d == p[1])&&(0x54 == p[2]))
	{
		disasm_x8664_print(p,5);
		printf("r10 = r14 + rsi + 0x%x\n", p[4]);
		return 5;
	}
	if((0x4d == p[0])&&(0x8d == p[1])&&(0x99 == p[2]))
	{
		disasm_x8664_print(p,7);
		printf("r11 = r9 + 0x%x\n", *(u32*)(p+3));
		return 7;
	}

	disasm_x8664_print(p,1);
	printf("error\n");
*/
