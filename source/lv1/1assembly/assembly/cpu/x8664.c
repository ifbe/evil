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




//modr/m: mod(7:6), reg(5:3), rm(2:0)
//sib: s(7:6), i(5:3), b(2:0)
#define rexb (1<<0)	//(sib.b+=8 | modrm.rm+=8)rax->r8
#define rexx (1<<1)	//(sib.i+=8)rax->r8
#define rexr (1<<2)	//(modrm.reg+=8)eax -> r8d
#define rexw (1<<3)	//(modrm.reg64)eax -> rax
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
static char* name80[8] = {
"add1","or1","adc1","sbb1","and1","sub1","xor1","cmp1"};
static char* name81[8] = {
"add4","or4","adc4","sbb4","and4","sub4","xor4","cmp4"};
static char* name83[8] = {
"add4","or4","adc4","sbb4","and4","sub4","xor4","cmp4"};
static char* namec0[8] = {
"rol1","ror1","rcl1","rcr1","shl1","shr1","error","sar1"};
static char* namec1[8] = {
"rol4","ror4","rcl4","rcr4","shl4","shr4","error","sar4"};
static char* namef6[8] = {
"test1","error1","not1","neg1","mul1","imul1","div1","idiv1"};
static char* namef7[8] = {
"test4","error4","not4","neg4","mul4","imul4","div4","idiv4"};
static char* namefe[2] = {"inc1","dec1"};
static char* nameff[8] = {
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
void disasm_x8664_prefixprint(u8* pre, u8* p, int k)
{
	if(0 == pre){
		disasm_x8664_print(p, k);
		return;
	}

	int sz = p-pre;
	if(sz <= 3){
		disasm_x8664_print(pre, k+sz);
		return;
	}

	//error
	disasm_x8664_print(p, k);
}




int disasm_x8664_1b(u8* p, u64 rip)
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
		if(0x0f == p[1])return 0;
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
int disasm_x8664_sib(u64 rip, u8* p, char* str, s64* dat, char** istr, char** bstr)
{
	u8 _s,_i,_b;
	u8 src,dst;
	s64 off;
	if(0 == istr)istr = reg64;
	if(0 == bstr)bstr = reg64;
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
			snprintf(str, 64, "%s +0x%llx", bstr[dst], off);
			return 6;
		}

		//p[2] = [20,27] [60,67] [a0,a7] [e0,e7]
		_s = 1 << (p[2]>>6);
		_i = (p[2]>>3)&7;
		_b = p[2]&7;
		off = *(int*)(p+3);
		if(4 == _i){
			snprintf(str, 64, "%s +0x%llx", bstr[_b], off);
		}
		//p[2] != above
		else{
			snprintf(str, 64, "%d*%s +%s +0x%llx", _s, istr[_i], bstr[_b], off);
		}
		return 7;
	}

	//p[1]: [40,7f]
	if(p[1] >= 0x40){
		//p[1] != *4 *c
		if(4 != (p[1] &7)){
			dst = p[1]&7;
			snprintf(str, 64, "%s + 0x%x", bstr[dst], *(char*)(p+2));
			return 3;
		}

		//p[2] = [20,27] [60,67] [a0,a7] [e0,e7]
		_s = 1 << (p[2]>>6);
		_i = (p[2]>>3)&7;
		_b = p[2]&7;
		if(4 == _i){
			snprintf(str, 64, "%s + 0x%x", bstr[_b], *(char*)(p+3));
		}
		//p[2] != above
		else{
			snprintf(str, 64, "%d*%s +%s + 0x%x", _s, istr[_i], bstr[_b], *(char*)(p+3));
		}
		return 4;
	}

	//p[1]: [00,3f]
	if(p[1] < 0x40){
		//p[1] != *4 *5 *c *d
		if(4 != (p[1] & 6)){
			dst = p[1]&7;
			snprintf(str, 64, "%s", bstr[dst]);
			return 2;
		}
		//p[1] = *5 *d
		if(5 == (p[1] &7)){
			off = *(int*)(p+2);
			if(str)snprintf(str, 64, "rel 0x%llx", rip+6+off);
			if(dat)dat[0] = off;
			return 6;
		}

		//p[2] = *5 *d
		_s = 1 << (p[2]>>6);
		_i = (p[2]>>3)&7;
		_b = p[2]&7;
		if(5 == _b){
			off = *(int*)(p+3);
			if(4 == _i)snprintf(str, 64, "0x%llx", off);
			else snprintf(str, 64, "%d*%s+0x%llx", _s, istr[_i], off);
			return 7;
		}
		//p[2] = else
		if(4 == _i)snprintf(str, 64, "%s", bstr[_b]);
		else snprintf(str, 64, "%d*%s+%s", _s, istr[_i], bstr[_b]);
		return 3;
	}//add [rel 0x1234],al

	return 1;
}
int disasm_x8664_normal(u8* pre, u8* opc, u64 rip)
{
	int ret;
	s64 rel;
	char tmp[128];

	u8 fix66 = 0;
	u8 fix67 = 0;
	u8 fix48 = 0;
	u8 fix44 = 0;
	u8 fix42 = 0;
	u8 fix41 = 0;
	u8 fix40 = 0;
	for(ret=0;ret<opc-pre;ret++){
		if(0x66 == pre[ret])fix66 = 1;
		if(0x67 == pre[ret])fix67 = 1;
		if(0x40 == (pre[ret] & 0xf0)){
			if(rexw & pre[ret])fix48 = 1;
			if(rexr & pre[ret])fix44 = 1;
			if(rexx & pre[ret])fix42 = 1;
			if(rexb & pre[ret])fix41 = 1;
			if(0x40== pre[ret])fix40 = 1;
		}
	}

	//----------------8bit op----------------
	char** fixbyte;
	if(fix40)fixbyte = rex08;
	else fixbyte = reg08;

	//---------32bit.normal, mode reg r/m--------
	char** fix012;
	char** fix345;
	if(fix48)fix012 = fix345 = reg64;
	else if(fix66)fix012 = fix345 = reg16;
	else fix012 = fix345 = reg32;

	if(fix44)fix345 = &fix345[8];
	if(fix41)fix012 = &fix012[8];

	//-------32bit.special, scale index base---------
	char** bstr;
	char** istr;
	if(fix67)bstr = istr = reg32;
	else bstr = istr = reg64;

	if(fix42)istr = &istr[8];
	if(fix41)bstr = &bstr[8];


//printf("%llx,%llx\n",fix,opc);
//printf("fix66=%d,fix67=%d,fix48=%d,fix44=%d,fix42=%d,fix41=%d,fix40=%d\n",
//fix66,fix67,fix48,fix44,fix42,fix41,fix40);

	u8 bit6 = (opc[1]>>6)&3;
	u8 bit3 = (opc[1]>>3)&7;
	u8 bit0 = opc[1]&7;

	//add
	if(0x0 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("add	%s += %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("add	[%s] += %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x1 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("add	%s += %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("add	[%s] += %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x2 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("add	%s += %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("add	%s += [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x3 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("add	%s += %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("add	%s += [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x4 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("add	al += 0x%x\n", opc[1]);
		return 2;
	}
	if(0x5 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("add	eax += 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//6,7: unused

	//or
	if(0x8 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("or	%s |= %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("or	[%s] |= %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x9 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("or	%s |= %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("or	[%s] |= %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0xa == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("or	%s |= %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("or	%s |= [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0xb == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("or	%s |= %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("or	%s |= [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0xc == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("or	al |= 0x%x\n", opc[1]);
		return 2;
	}
	if(0xd == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("or	eax |= 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//e: unused
	//f: special

	//adc
	if(0x10 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("adc	%s += %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("adc	[%s] += %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x11 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("adc	%s += %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("adc	[%s] += %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x12 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("adc	%s += %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("adc	%s += [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x13 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("adc	%s += %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("adc	%s += [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x14 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("adc	al += 0x%x\n", opc[1]);
		return 2;
	}
	if(0x15 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("adc	eax += 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//16,17: unused

	//sbb
	if(0x18 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sbb	%s -= %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sbb	[%s] -= %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x19 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sbb	%s -= %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sbb	[%s] -= %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x1a == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sbb	%s -= %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sbb	%s -= [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x1b == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sbb	%s -= %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sbb	%s -= [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x1c == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("sbb	al -= 0x%x\n", opc[1]);
		return 2;
	}
	if(0x1d == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("sbb	eax -= 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//1e,1f: unused

	//and
	if(0x20 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("and	%s &= %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("and	[%s] &= %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x21 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("and	%s &= %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("and	[%s] &= %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x22 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("and	%s &= %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("and	%s &= [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x23 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("and	%s &= %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("and	%s &= [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x24 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("and	al &= 0x%x\n", opc[1]);
		return 2;
	}
	if(0x25 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("and	eax &= 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//26: es
	//27: unused

	//sub
	if(0x28 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sub	%s -= %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sub	[%s] -= %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x29 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sub	%s -= %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sub	[%s] -= %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x2a == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sub	%s -= %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sub	%s -= [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x2b == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("sub	%s -= %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("sub	%s -= [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x2c == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("sub	al -= 0x%x\n", opc[1]);
		return 2;
	}
	if(0x2d == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("sub	eax -= 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//2e: cs
	//2f: unused

	//xor
	if(0x30 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("xor	%s ^= %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("xor	[%s] ^= %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x31 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("xor	%s ^= %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("xor	[%s] ^= %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x32 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("xor	%s ^= %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("xor	%s ^= [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x33 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("xor	%s ^= %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("xor	%s ^= [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x34 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("xor	al ^= 0x%x\n", opc[1]);
		return 2;
	}
	if(0x35 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("xor	eax ^= 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//36: ss
	//37: rex

	//cmp
	if(0x38 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("cmp	%s - %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("cmp	[%s] - %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x39 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("cmp	%s - %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("cmp	[%s] - %s\n", tmp, fix345[bit3]);
		return ret;
	}
	if(0x3a == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("cmp	%s - %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("cmp	%s - [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x3b == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("cmp	%s - %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("cmp	%s - [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	if(0x3c == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("cmp	al - 0x%x\n", opc[1]);
		return 2;
	}
	if(0x3d == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("cmp	eax - 0x%x\n", *(u32*)(opc+1));
		return 5;
	}
	//3e: ds
	//3f: unused

	//[40,4f]: rex?

	//[50,5f]: push,pop
	//[60,62]: unused
	if((0x63 == opc[0]) && (pre < opc)){
		if(3 == bit6){
			fix012 = reg32;
			if(fix41)fix012 = &fix012[8];

			disasm_x8664_print(pre, opc-pre+2);
			printf("movsxd	%s = %s\n", fix345[bit3], fix012[bit0]);
			return opc-pre+2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("movsxd	%s = dword@[%s]\n", fix345[bit3], tmp);
		return opc-pre+ret;
	}
	//64: fs
	//65: gs

	//66: operand-size override prefix
	//67: address-size override prefix

	//68: push qword 0xxxxx
	if(0x68 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("push	0x%llx\n", (s64)*(int*)(opc+1));
		return 5;
	}
	//69: imul dst,src,dword 0x12345678
	if(0x69 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+6);
			printf("imul4	%s = %s / 0x%x\n",
				fix345[bit3], fix012[bit0], *(u32*)(opc+2));
			return 6;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+4);
		printf("imul4	%s = ", fix345[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("[%s]", tmp);
		else printf("[rel %llx]", (rip+ret+4)+rel);
		printf(" / 0x%x\n", *(u32*)(opc+ret));
		return ret+4;
	}
	if(0x6a == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("push	0x%llx\n", (s64)(char)opc[1]);
		return 2;
	}
	//6b: imul dst,src,byte 34
	if(0x6b == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+3);
			printf("imul1	%s = %s / %d\n",
				fix345[bit3], fix012[bit0], (char)opc[2]);
			return 3;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+1);
		printf("imul1	%s = ", fix345[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("[%s]", tmp);
		else printf("[rel %llx]", (rip+ret+1)+rel);
		printf(" / %d\n", (char)opc[ret]);
		return ret+1;
	}

	//6c: insb
	//6d: insd
	//6e: outsb
	//6f: outsd

	//j: [70,7f]
	if(0x70 == (opc[0]&0xf0)){
		disasm_x8664_print(pre, opc-pre+2);
		printf("j%s	rip = 0x%llx\n", cond[opc[0]&0xf], (char)opc[1] + (rip+2));
		return 2;
	}

	//80:
	if(0x80 == opc[0]){
		if(0xc0 == (opc[1]&0xf8)){
			disasm_x8664_print(pre, opc-pre+3);
			printf("add	%s += 0x%x\n", reg08[opc[1]&0x7], opc[2]);
			return 3;
		}
		if(0xe8 == (opc[1]&0xf8)){
			disasm_x8664_print(pre, opc-pre+3);
			printf("sub	%s -= 0x%x\n", reg08[opc[1]&0x7], opc[2]);
			return 3;
		}

		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+3);
			printf("%s	%s %s byte 0x%x\n",
			name80[bit3], fix012[bit0], symbol[bit3], opc[2]);
			return 3;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+1);
		printf("%s	", name80[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("byte@[%s]", tmp);
		else printf("byte@[rel %llx]", (rip+ret+1)+rel);
		printf(" %s byte 0x%x\n", symbol[bit3], opc[ret]);
		return ret+1;
	}
	//81:
	if(0x81 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+6);
			printf("%s	%s %s dword 0x%x\n",
			name81[bit3], fix012[bit0], symbol[bit3], *(u32*)(opc+2));
			return 6;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+4);
		printf("%s	", name81[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("dword@[%s]", tmp);
		else printf("dword@[rel %llx]", (rip+ret+4)+rel);
		printf(" %s dword 0x%x\n", symbol[bit3], *(u32*)(opc+ret));
		return ret+4;
	}
	//82: unused
	//83:
	if(0x83 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+3);
			printf("%s	%s %s byte 0x%x\n",
			name83[bit3], fix012[bit0], symbol[bit3], opc[2]);
			return 3;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+1);
		printf("%s	", name83[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("dword@[%s]", tmp);
		else printf("dword@[rel %llx]", (rip+ret+1)+rel);
		printf(" %s byte 0x%x\n", symbol[bit3], opc[ret]);
		return ret+1;
	}

	//84: test
	if(0x84 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("test	%s & %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("test	%s & [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x85 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("test	%s & %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("test	%s & [%s]\n", fix345[bit3], tmp);
		return ret;
	}
	//86: xchg
	if(0x86 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("xchg	%s, %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("xchg	%s, [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x87 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("xchg	%s, %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("xchg	%s, [%s]\n", fix345[bit3], tmp);
		return ret;
	}

	//[88,8a]: mov
	if(0x88 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("mov	%s = %s\n", fixbyte[bit0], fixbyte[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("mov	[%s] = %s\n", tmp, fixbyte[bit3]);
		return ret;
	}
	if(0x89 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("mov	%s = %s\n", fix012[bit0], fix345[bit3]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("mov	[%s] = ", tmp);
		printf("%s\n", fix345[bit3]);
		return ret;
	}
	if(0x8a == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("mov	%s = %s\n", fixbyte[bit3], fixbyte[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("mov	%s = [%s]\n", fixbyte[bit3], tmp);
		return ret;
	}
	if(0x8b == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+2);
			printf("mov	%s = %s\n", fix345[bit3], fix012[bit0]);
			return 2;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("mov	%s = [%s]\n", fix345[bit3], tmp);
		return ret;
	}

	//8c: mov xxx,cs/ds/es/gs...
	//8d: lea
	if(0x8d == opc[0]){
		ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret);
		printf("lea	");
		printf("%s", fix345[bit3]);
		printf(" = %s\n", tmp);
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
	if(0xa0 == opc[0]){
		disasm_x8664_print(pre, opc-pre+8);
		printf("mov	al = [0x%llx]\n", *(u64*)(opc+1));
		return 9;
	}
	if(0xa1 == opc[0]){
		disasm_x8664_print(pre, opc-pre+8);
		printf("mov	eax = [0x%llx]\n", *(u64*)(opc+1));
		return 9;
	}
	if(0xa2 == opc[0]){
		disasm_x8664_print(pre, opc-pre+8);
		printf("mov	[0x%llx] = al\n", *(u64*)(opc+1));
		return 9;
	}
	if(0xa3 == opc[0]){
		disasm_x8664_print(pre, opc-pre+8);
		printf("mov	[0x%llx] = eax\n", *(u64*)(opc+1));
		return 9;
	}
	//a4: movsb
	//a5: movsd
	//a6: cmpsb
	//a7: cmpsd

	//test
	if(0xa8 == opc[0])
	{
		disasm_x8664_print(pre, opc-pre+2);
		printf("test	al & 0x%x\n", opc[1]);
		return 2;
	}
	if(0xa9 == opc[0])
	{
		disasm_x8664_print(pre, opc-pre+5);
		printf("test	eax & 0x%x\n", *(u32*)(opc+1));
		return 5;
	}

	//aa: stosb
	//ab: stosd
	//ac: lodsb
	//ad: lodsd
	//ae: scasb
	//af: scasd

	//[b0,b7]: mov8 r,0xff
	if(0xb0 == (opc[0]&0xf8)){
		disasm_x8664_print(pre, opc-pre+2);
		if(fix66){
			printf("mov1	%s = 0x%x\n", reg16[opc[0]&7], opc[1]);
		}
		else{
			printf("mov1	%s = 0x%x\n", fixbyte[opc[0]&7], opc[1]);
		}
		return 2;
	}

	//[b8,bf]: mov32 r,0xff
	if(0xb8 == (opc[0]&0xf8)){
		disasm_x8664_print(pre, opc-pre+5);
		printf("mov4	%s = 0x%x\n", fix012[opc[0]&7], *(u32*)(opc+1));
		return 5;
	}

	//c0: shift byte
	if(0xc0 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+3);
			printf("%s	%s, byte 0x%x\n",
			namec0[bit3], fixbyte[bit0], opc[2]);
			return 3;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+1);
		printf("%s	", namec0[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("byte@[%s]", tmp);
		else printf("byte@[rel %llx]", (rip+ret+1)+rel);
		printf(", byte 0x%x\n", opc[ret]);
		return ret+1;
	}
	//c1: shift dword
	if(0xc1 == opc[0]){
		if(3 == bit6){
			disasm_x8664_print(pre, opc-pre+3);
			printf("%s	%s, byte 0x%x\n",
			namec1[bit3], fix012[bit0], opc[2]);
			return 3;
		}
		ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
		disasm_x8664_print(pre, opc-pre+ret+1);
		printf("%s	", namec1[bit3]);
		if(0x05 != (opc[1]&0xc7))printf("dword@[%s]", tmp);
		else printf("dword@[rel %llx]", (rip+ret+1)+rel);
		printf(", byte 0x%x\n", opc[ret]);
		return ret+1;
	}
	if(0xc2 == opc[0]){
		disasm_x8664_print(pre, opc-pre+3);
		printf("ret	0x%x\n", *(u16*)(opc+1));
		return 3;
	}
	//c3: ret

	//c4: unused
	//c5: xmm

	//c6: mov byte
	if(0xc6 == opc[0]){
		//c6f8: xabort
		if(0xf8 == opc[1]){
			disasm_x8664_print(pre, opc-pre+3);
			printf("xabort	0x%x\n", opc[2]);
			return 3;
		}
		switch(opc[1]&0xf8){
		case 0x00:
		case 0x40:
		case 0x80:{
			ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
			disasm_x8664_print(pre, opc-pre+ret+1);
			printf("mov1	");
			if(0x05 != (opc[1]&0xc7))printf("[%s]", tmp);
			else printf("[rel %llx]", (rip+ret+1)+rel);
			printf(" = 0x%x\n", opc[ret]);
			return ret+1;
		}
		case 0xc0:{
			disasm_x8664_print(pre, opc-pre+3);
			printf("mov1	[%s] = 0x%x\n", fixbyte[opc[1]&7], opc[2]);
			return 3;
		}
		}//switch
	}
	//c7: mov dworld
	if(0xc7 == opc[0]){
		//c6f8: xabort
		if(0xf8 == opc[1]){
			disasm_x8664_print(pre, opc-pre+3);
			printf("xbegin	0x%llx\n", (s64)(*(int*)(opc+2)));
			return 6;
		}
		switch(opc[1]&0xf8){
		case 0x00:
		case 0x40:
		case 0x80:{
			ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
			disasm_x8664_print(pre, opc-pre+ret+4);
			printf("mov4	");
			if(0x05 != (opc[1]&0xc7))printf("[%s]", tmp);
			else printf("[rel %llx]", (rip+ret+4)+rel);
			printf(" = 0x%x\n", *(u32*)(opc+ret));
			return ret+4;
		}
		case 0xc0:{
			disasm_x8664_print(pre, opc-pre+6);
			//printf("mov4	[%s] = 0x%x\n", reg32[opc[1]&7], *(u32*)(opc+2));
			printf("mov64	%s = 0x%x\n", reg64[opc[1]&7], *(u32*)(opc+2));
			return 6;
		}
		}//switch
	}

	if(0xc8 == opc[0]){
		disasm_x8664_print(pre, opc-pre+4);
		printf("enter	0x%x,0x%x\n", *(u16*)(opc+1), opc[3]);
		return 4;
	}
	//c9: leave

	if(0xca == opc[0]){
		disasm_x8664_print(pre, opc-pre+3);
		printf("retf	0x%x\n", *(u16*)(opc+1));
		return 3;
	}
	//cb: retf

	//cc: int3
	if(0xcd == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("int	0x%x\n", opc[1]);
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
	if(0xe0 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("loopne	0x%llx\n", (char)opc[1] + (rip+2));
		return 2;
	}
	if(0xe1 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("loope	0x%llx\n", (char)opc[1] + (rip+2));
		return 2;
	}
	if(0xe2 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("loop	0x%llx\n", (char)opc[1] + (rip+2));
		return 2;
	}
	if(0xe3 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("jrcxz	0x%llx\n", (char)opc[1] + (rip+2));
		return 2;
	}

	//in
	if(0xe4 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("in	al = port[0x%x]\n", opc[1]);
		return 2;
	}
	if(0xe5 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("eax	eax = port[0x%x]\n", opc[1]);
		return 2;
	}

	//out
	if(0xe6 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("out	port[0x%x] = al\n", opc[1]);
		return 2;
	}
	if(0xe7 == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("out	port[0x%x] = eax\n", opc[1]);
		return 2;
	}

	if(0xe8 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("call	push 0x%llx, rip = 0x%llx\n", rip+5, (u64)(*(int*)(opc+1) + (rip+5)));
		return 5;
	}
	if(0xe9 == opc[0]){
		disasm_x8664_print(pre, opc-pre+5);
		printf("jmp	rip = 0x%llx\n", (u64)(*(int*)(opc+1) + (rip+5)));
		return 5;
	}
	//ea:unused
	if(0xeb == opc[0]){
		disasm_x8664_print(pre, opc-pre+2);
		printf("jmp	rip = 0x%llx\n", (u64)((char)opc[1] + (rip+2)));
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
	if((0xf3 == opc[0])&&(0x0f == opc[1])&&(0x1e == opc[2])&&(0xfa == opc[3])){
		disasm_x8664_print(pre, opc-pre+4);
		printf("endbr64\n");
		return 4;
	}
	//f4: hlt
	//f5: cmc

	//f6: 8bit
	if(0xf6 == opc[0]){
		if(3 == bit6){
			if(0 == bit3){
				disasm_x8664_print(pre, opc-pre+3);
				printf("%s	%s & 0x%x\n", namef6[bit3], fixbyte[bit0], opc[2]);
				return 3;
			}//0
			if(1 != bit3){
				disasm_x8664_print(pre, opc-pre+2);
				printf("%s	%s\n", namef6[bit3], fixbyte[bit0]);
				return 2;
			}//234567
		}//test reg8, reg8
		else{
			if(0 == bit3){
				ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
				disasm_x8664_print(pre, opc-pre+ret+1);
				printf("test1	");
				if(0x05 != (opc[1]&0xc7))printf("[%s]", tmp);
				else printf("[rel %llx]", (rip+ret+1)+rel);
				printf(" & 0x%x\n", opc[ret]);
				return ret+1;
			}//0
			if(1 != bit3){
				ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
				disasm_x8664_print(pre, opc-pre+ret);
				printf("%s	[%s]\n", namef6[bit3], tmp);
				return ret;
			}//234567
		}//else
	}//f6
	//f7: 32bit
	if(0xf7 == opc[0]){
		if(3 == bit6){
			if(0 == bit3){
				disasm_x8664_print(pre, opc-pre+6);
				printf("%s	%s & 0x%x\n", namef7[bit3], fix012[bit0], *(u32*)(opc+2));
				return 6;
			}//0
			if(1 != bit3){
				disasm_x8664_print(pre, opc-pre+2);
				printf("%s	%s\n", namef7[bit3], fix012[bit0]);
				return 2;
			}//234567
		}//test reg32, reg32
		else{
			if(0 == bit3){
				ret = disasm_x8664_sib(rip, opc, tmp, &rel, istr, bstr);
				disasm_x8664_print(pre, opc-pre+ret+4);
				printf("test4	");
				if(0x05 != (opc[1]&0xc7))printf("[%s]", tmp);
				else printf("[rel %llx]", (rip+ret+4)+rel);
				printf(" & 0x%x\n", *(u32*)(opc+ret));
				return ret+4;
			}//0
			if(1 != bit3){
				ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
				disasm_x8664_print(pre, opc-pre+ret);
				printf("%s	[%s]\n", namef7[bit3], tmp);
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
	if(0xfe == opc[0]){
		switch(opc[1]&0xf0){
		case 0xc0:{	//inc,dec
			disasm_x8664_print(pre, opc-pre+2);
			printf("%s	%s\n", namefe[bit3], fixbyte[bit0]);
			return 2;
		}
		case 0x00:
		case 0x40:
		case 0x80:{
			ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
			disasm_x8664_print(pre, opc-pre+ret);
			printf("%s	[%s]\n", namefe[bit3], tmp);
			return ret;
		}
		}//switch
	}
	//ff: 32bit
	if(0xff == opc[0]){
		switch(opc[1]&0xf8){
		case 0xc0:	//inc4 reg
		case 0xc8:	//dec4 reg
		case 0xd0:	//call8 reg
		case 0xe0:	//jmp8 reg
		case 0xf0:{	//push8 reg
			disasm_x8664_print(pre, opc-pre+2);
			printf("%s	%s\n", nameff[bit3], fix012[bit0]);
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
			ret = disasm_x8664_sib(rip, opc, tmp, 0, istr, bstr);
			disasm_x8664_print(pre, opc-pre+ret);
			printf("%s	[%s]\n", nameff[bit3], tmp);
			return ret;
		}
		}//switch
	}

	return 0;
}
int disasm_x8664_0f(u8* pre, u8* p, u64 rip)
{
	//00:
	//	[00,07]: sldt r/m
	//	[08,0f]: str r/m
	//	[10,17]: lldt r/m
	//	[18,1f]: ltr r/m
	//	[20,27]: verr r/m
	//	[28,2f]: verw r/m
	//	[30,37]: jmpe r/m
	//01:
	if(0x01 == p[1]){
		if(0x07 == p[2]){
			disasm_x8664_print(p,3);
			printf("sgdt rdi\n");
			return 3;
		}
		if(0x0f == p[2]){
			disasm_x8664_print(p,3);
			printf("sidt rdi\n");
			return 3;
		}
	}
	//	[00,07]: sgdt r/m
	//	[08,0f]: sidt r/m
	//	[10,17]: lgdt r/m
	//	[18,1f]: lidt r/m
	//	[20,27]: smss r/m
	//	[30,37]: lmsw r/m
	//	[38,3f]: invlpg r/m
	//	c0: enclv
	//	c1: vmcall
	//	c2: vmlaunch
	//	c3: vmresume
	//	c4: vmxoff
	//	c5: pconfig
	//	c8: monitor
	//	c9: mwait
	//	ca: clac
	//	cb: stac
	//	cf: encls
	//	d0: xgetbv
	//	d1: xsetbv
	//	d4: vmfunc
	//	d5: xsend
	//	d6: xtest
	//	d7: enclu
	//	d8: vmrun
	//	d9: vmmcall
	//	da: vmload
	//	db: vmsave
	//	dc: stgi
	//	dd: clgi
	//	de: skinit
	//	df: invlpga rax,ecx
	//	[e0,e7]: smsw reg32
	//	ee: rdpkru
	//	ef: wrpkru
	//	[f0,f7]: lmsw reg16
	//	f8: swapgs
	//	f9: rdtscp
	//	fa: monitorx
	//	fb: mwaitx
	//	fc: clzero
	//02: lar reg, r/m
	//03: lsl reg, r/m

	if(0x05 == p[1]){
		disasm_x8664_print(p,2);
		printf("syscall\n");
		return 2;
	}
	if(0x06 == p[1]){
		disasm_x8664_print(p,2);
		printf("clts\n");
		return 2;
	}
	if(0x07 == p[1]){
		disasm_x8664_print(p,2);
		printf("sysret\n");
		return 2;
	}
	if(0x08 == p[1]){
		disasm_x8664_print(p,2);
		printf("invd\n");
		return 2;
	}
	if(0x09 == p[1]){
		disasm_x8664_print(p,2);
		printf("wbinvd\n");
		return 2;
	}
	if(0x0b == p[1]){
		disasm_x8664_print(p,2);
		printf("ud2	trigger(#ud)\n");
		return 2;
	}
	//0d: prefetch
	if(0x0e == p[1]){
		disasm_x8664_print(p,2);
		printf("femms\n");
		return 2;
	}
	//0f: mm*
	//10: movups xmm*, r/m
	//11: movups r/m, xmm*
	//12: movlps xmm*, r/m
	//13: movlps r/m, xmm*
	//14: unpcklps xmm*, r/m
	//15: unpckhps xmm*, r/m
	//16: movhps xmm*, r/m
	//17: movhps r/m, xmm*
	//[18,1e]: hint_nop?

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

	if((0x20 == p[1])&&(p[2]>=0xc0)){
		u8 b0 = p[2]&0x7;
		u8 b3 = (p[2]>>3)&0x7;
		disasm_x8664_print(p,3);
		printf("mov	%s = cr%d\n", reg64[b0], b3);
		return 3;
	}
	if((0x21 == p[1])&&(p[2]>=0xc0)){
		u8 b0 = p[2]&0x7;
		u8 b3 = (p[2]>>3)&0x7;
		disasm_x8664_print(p,3);
		printf("mov	%s = dr%d\n", reg64[b0], b3);
		return 3;
	}
	if((0x22 == p[1])&&(p[2]>=0xc0)){
		u8 b0 = p[2]&0x7;
		u8 b3 = (p[2]>>3)&0x7;
		disasm_x8664_print(p,3);
		printf("mov	cr%d = %s\n", b3, reg64[b0]);
		return 3;
	}
	if((0x23 == p[1])&&(p[2]>=0xc0)){
		u8 b0 = p[2]&0x7;
		u8 b3 = (p[2]>>3)&0x7;
		disasm_x8664_print(p,3);
		printf("mov	dr%d = %s\n", b3, reg64[b0]);
		return 3;
	}
	//28: movaps xmm*, r/m
	//29: movaps r/m, xmm*
	if(0x29 == p[1]){
		if((0x44 == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm0\n", p[4]);
		return 5;
		}
		if((0x4c == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm1\n", p[4]);
		return 5;
		}
		if((0x54 == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm2\n", p[4]);
		return 5;
		}
		if((0x9c == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm3\n", p[4]);
		return 5;
		}
		if((0xa4 == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm4\n", p[4]);
		return 5;
		}
		if((0xac == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm5\n", p[4]);
		return 5;
		}
		if((0xb4 == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm6\n", p[4]);
		return 5;
		}
		if((0xbc == p[2])&&(0x24 == p[3]))
		{
		disasm_x8664_print(p,5);
		printf("[rsp+0x%x] = xmm7\n", p[4]);
		return 5;
		}
	}
	//2a: cvtpi2ps xmm*, r/m
	//2b: movntps r/m, xmm*
	//2c: cvttps2pi mm*, r/m
	//2d: cvtps2pi mm*, r/m
	//2e: ucomiss xmm*, r/m
	//2f: comiss xmm*, r/m
	if(0x30 == p[1]){
		disasm_x8664_print(p,2);
		printf("wrmsr\n");
		return 2;
	}
	if(0x31 == p[1]){
		disasm_x8664_print(p,2);
		printf("rdtsc\n");
		return 2;
	}
	if(0x32 == p[1]){
		disasm_x8664_print(p,2);
		printf("rdmsr\n");
		return 2;
	}
	if(0x33 == p[1]){
		disasm_x8664_print(p,2);
		printf("rdpmc\n");
		return 2;
	}
	if(0x34 == p[1]){
		disasm_x8664_print(p,2);
		printf("sysenter\n");
		return 2;
	}
	if(0x35 == p[1]){
		disasm_x8664_print(p,2);
		printf("sysexit\n");
		return 2;
	}
	//36: rdshr r/m
	if(0x37 == p[1]){
		disasm_x8664_print(p,2);
		printf("rdsec\n");
		return 2;
	}
	//38: pshufb mm*, r/m
	if(0x39 == p[1]){
		disasm_x8664_print(p,2);
		printf("dmint\n");
		return 2;
	}
	//0F3A0F: palignr mm*, r/m, byte
	//0F3ACC: sha1rnds4 xmm*, r/m, byte
	if(0x3c == p[1]){
		disasm_x8664_print(p,2);
		printf("cpu_write\n");
		return 2;
	}
	if(0x3d == p[1]){
		disasm_x8664_print(p,2);
		printf("cpu_read\n");
		return 2;
	}
	if(0x40 == (p[1]&0xf0)){
		u8 b0 = p[2]&0x7;
		u8 b3 = (p[2]>>3)&0x7;
		u8 b6 = (p[2]>>6)&0x3;
		if(3 == b6){
			disasm_x8664_print(p,2);
			printf("cmov%s	%s = %s\n", cond[p[1]&0xf], reg32[b3], reg32[b0]);
			return 3;
		}

		char tmp[64];
		int ret = disasm_x8664_sib(rip, p+1, tmp, 0, 0, 0);
		disasm_x8664_print(p,ret+1);
		printf("cmov%s	%s = %s\n", cond[p[1]&0xf], reg32[b3], tmp);
		return ret+1;
	}
	//50: paveb mm*, r/m
	//51: sqrtps xmm*, r/m
	//52: rsqrtps xmm*, r/m
	//53: rcpps xmm*, r/m
	//54: andps xmm*, r/m
	//55: andnps xmm*, r/m
	//56: orps xmm*, r/m
	//57: xorps xmm*, r/m
	//58: addps xmm*, r/m
	//59: mulps xmm*, r/m
	//5a: cvtps2pd xmm*, r/m
	//5b: cvtdq2ps xmm*, r/m
	//5c: subps xmm*, r/m
	//5c: subps xmm*, r/m
	//5d: minps xmm*, r/m
	//5e: divps xmm*, r/m
	//5f: maxps xmm*, r/m

	//60: punpcklbw mm*, r/m
	//61: punpcklwd mm*, r/m
	//62: punpckldq mm*, r/m
	//63: packsswb mm*, r/m
	//64: pcmpgtb mm*, r/m
	//65: pcmpgtw mm*, r/m
	//66: pcmpgtd mm*, r/m
	//67: packuswb mm*, r/m
	//68: punpckhbw mm*, r/m
	//69: punpckhwd mm*, r/m
	//6a: punpckhdq mm*, r/m
	//6b: packssdw mm*, r/m
	//6e: movd mm*, r/m
	//6f: movq mm*, r/m

	//70: pshufw mm*, r/m, byte
	//71:
	//	[d0,d7]: psrlw mm*, byte
	//	[e0,e7]: psraw mm*, byte
	//	[f0,f7]: psllw mm*, byte
	//72:
	//	[d0,d7]: psrld mm*, byte
	//	[e0,e7]: psrad mm*, byte
	//	[f0,f7]: pslld mm*, byte
	//73:
	//	[d0,d7]: psrlq mm*, byte
	//	[f0,f7]: psllq mm*, byte
	//74: pcmpeqb mm*, r/m
	//75: pcmpeqw mm*, r/m
	//76: pcmpeqd mm*, r/m
	if(0x77 == p[1]){
		disasm_x8664_print(p,2);
		printf("emms\n");
		return 2;
	}

	//78: vmread r/m, reg
	//79: vmwrite r/m, reg
	//7b: rsldt r/m
	//7c: svts r/m
	//7d: rsts r/m
	//7e: movd r/m, mm*
	//7f: movq r/m, mm*

	//[80,8f]: jump +-32bit
	if(0x80 == (p[1]&0xf0))
	{
		disasm_x8664_print(p,6);
		printf("j%s	rip = 0x%llx\n", cond[p[1]&0xf], *(int*)(p+2) + (rip+6));
		return 6;
	}

	//[90,9f]: seto r/m
	if(0x90 == (p[1]&0xf0))
	{
		if(0xc0 == (p[2]&0xf8)){
			disasm_x8664_print(p,3);
			printf("set%s	%s\n", cond[p[1]&0xf], reg08[p[2]&7]);
			return 3;
		}
	}

	if(0xa0 == p[1]){
		disasm_x8664_print(p,2);
		printf("push	fs\n");
		return 2;
	}
	if(0xa1 == p[1]){
		disasm_x8664_print(p,2);
		printf("pop	fs\n");
		return 2;
	}
	if(0xa2 == p[1]){
		disasm_x8664_print(p,2);
		printf("cpuid\n");
		return 2;
	}
	//a3: bt r/m, reg
	//a4: shld r/m, reg32, byte
	//a5: shld r/m, reg32, reg08
	if(0xa6 == p[1]){
		if(0xc0 == p[2]){
			disasm_x8664_print(p,3);
			printf("montmul\n");
			return 3;
		}
		if(0xc8 == p[2]){
			disasm_x8664_print(p,3);
			printf("xsha1\n");
			return 3;
		}
		if(0xd0 == p[2]){
			disasm_x8664_print(p,3);
			printf("xsha256\n");
			return 3;
		}
	}
	if(0xa7 == p[1]){
		if(0xc0 == p[2]){
			disasm_x8664_print(p,3);
			printf("xstore\n");
			return 3;
		}
		else if(0xc8 == p[2]){
			disasm_x8664_print(p,3);
			printf("xcryptecb\n");
			return 3;
		}
		else if(0xd0 == p[2]){
			disasm_x8664_print(p,3);
			printf("xcryptcbc\n");
			return 3;
		}
		else if(0xd8 == p[2]){
			disasm_x8664_print(p,3);
			printf("xcryptctr\n");
			return 3;
		}
		else if(0xe0 == p[2]){
			disasm_x8664_print(p,3);
			printf("xcryptcfb\n");
			return 3;
		}
		else if(0xe8 == p[2]){
			disasm_x8664_print(p,3);
			printf("xcryptofb\n");
			return 3;
		}
	}
	if(0xa8 == p[1]){
		disasm_x8664_print(p,2);
		printf("push	gs\n");
		return 2;
	}
	if(0xa9 == p[1]){
		disasm_x8664_print(p,2);
		printf("pop	gs\n");
		return 2;
	}
	if(0xaa == p[1]){
		disasm_x8664_print(p,2);
		printf("rsm\n");
		return 2;
	}
	//ab: bts r/m, reg
	//ac: shrd r/m, reg, byte
	//ad: shrd r/m, reg, reg08
	//ae:
	//	[00,07]: fxsave
	//	[08,0f]: fxrstor
	//	[10,17]: ldmxcsr
	//	[18,1f]: stmxcsr
	//	[20,27]: xsave
	//	[28,2f]: xrstor
	//	[30,37]: xsaveopt
	//	[38,3f]: clflush
	//	...
	//	[e0,e7]: ptwrite reg
	//	e8: lfence
	//	f0: mfence
	//	f8: sfence

	//af: imul reg, r/m
	if(0xaf == p[1]){
		u8 bit0 = p[2]&7;
		u8 bit3 = (p[2]>>3)&7;
		u8 bit6 = (p[2]>>6)&7;
		if(3 == (p[2]>>6)){
			disasm_x8664_prefixprint(pre, p, 3);
			if(pre)printf("imul	%s *= %s\n", reg64[bit3+(pre[0]&4)*2], reg64[bit0+(pre[0]&1)*8]);
			else printf("imul	%s *= %s\n", reg64[bit3], reg64[bit0]);
			return 3;
		}

		char tmp[64];
		int ret = disasm_x8664_sib(rip, p+1, tmp, 0, 0, 0);
		disasm_x8664_prefixprint(pre, p, ret+1);
		if(pre)printf("imul	%s *= qword@[%s]\n", reg64[bit3+(pre[0]&4)*2], tmp);
		else printf("imul	%s *= qword@[%s]\n", reg64[bit3], tmp);
		return ret+1;
	}

	//b0: cmpxchg r/m, reg08
	//b1: cmpxchg r/m, reg32

	//b2: lss reg, r/m
	//b3: btr r/m, reg
	//b4: lfs reg, r/m
	//b5: lgs reg, r/m

	//b6: movzx reg32, r/m08
	if(0xb6 == p[1])
	{
		u8 bit0 = p[2]&7;
		u8 bit3 = (p[2]>>3)&7;
		u8 bit6 = (p[2]>>6)&7;
		if(3 == (p[2]>>6)){
			disasm_x8664_print(p, 3);
			printf("movzx	%s = %s\n", reg32[bit3], reg08[bit0]);
			return 3;
		}

		char tmp[64];
		int ret = disasm_x8664_sib(rip, p+1, tmp, 0, 0, 0);
		disasm_x8664_print(p, ret+1);
		printf("movzx	%s = byte@[%s]\n", reg32[bit3], tmp);
		return ret+1;
	}

	//b7: movzx reg32, r/m16
	if(0xb7 == p[1])
	{
		u8 bit0 = p[2]&7;
		u8 bit3 = (p[2]>>3)&7;
		u8 bit6 = (p[2]>>6)&7;
		if(3 == (p[2]>>6)){
			disasm_x8664_print(p, 3);
			printf("movzx	%s = %s\n", reg32[bit3], reg16[bit0]);
			return 3;
		}

		char tmp[64];
		int ret = disasm_x8664_sib(rip, p+1, tmp, 0, 0, 0);
		disasm_x8664_print(p, ret+1);
		printf("movzx	%s = word@[%s]\n", reg32[bit3], tmp);
		return ret+1;
	}

	if(0xb8 == p[1]){
		disasm_x8664_print(p,6);
		printf("jmpe	rip = %llx\n", *(int*)(p+2) + (rip+6));
		return 6;
	}
	//ba: bt dword r/m, byte
	//bb: btc r/m, reg32
	//bc: bsf reg, r/m
	//bd: bsr reg, r/m

	//be: movsx reg32, r/m08
	if((0xbe == p[1])&&(0x0a == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("ecx = (s32)[rdx,1]\n");
		return 3;
	}
	if((0xbe == p[1])&&(0xc8 == p[2]))
	{
		disasm_x8664_print(p,3);
		printf("ecx = (s32)al\n");
		return 3;
	}

	//bf: movsx reg32, r/m16

	//c0: xadd r/m, reg08
	//c1: xadd r/m, reg32
	//c2: cmpps xmm*, r/m, byte
	//c3: movnti r/m, reg32
	//c4: pinsrw mm*, r/m, byte
	//c5: pextrw reg32, mm*, byte
	//c6: shufps xmm*, r/m, byte

	if(0xc7 == p[1]){
	switch(p[2]&0xf8){
		//08: cmpxchg8b r/m
		//48: cmpxchg8b r/m
		//88: cmpxchg8b r/m

		//18: xrstors r/m
		//58: xrstors r/m
		//98: xrstors r/m

		//20: xsavec r/m
		//60: xsavec r/m
		//a0: xsavec r/m

		//28: xsaves r/m
		//68: xsaves r/m
		//a8: xsaves r/m

		//30: vmptrld r/m
		//70: vmptrld r/m
		//b0: vmptrld r/m

		//38: vmptrst r/m
		//78: vmptrst r/m
		//b8: vmptrst r/m

		case 0xf0:{
			disasm_x8664_print(p,3);
			printf("rdrand	%s\n", reg32[p[2]&7]);
			return 3;
		}//f0
		case 0xf8:{
			disasm_x8664_print(p,3);
			printf("rdseed	%s\n", reg32[p[2]&7]);
			return 3;
		}//f8
	}//switch
	}//c7

	//[c8,cf]: bswap
	if(0xc8 == (p[1]&0xf8)){
		if(0x48 == pre[0]){
			disasm_x8664_print(pre,3);
			printf("bswap	%s\n", reg64[p[1]&7]);
		}
		else{
			disasm_x8664_print(p,2);
			printf("bswap	%s\n", reg32[p[1]&7]);
		}
		return 2;
	}

	//d1: psrlw mm*, r/m
	//d2: psrld mm*, r/m
	//d3: psrlq mm*, r/m
	//d4: paddq mm*, r/m
	//d5: pmullw mm*, r/m
	//d7c0: pmovmskb reg32, mm*
	//d8: psubusb mm*, r/m
	//d9: psubusw mm*, r/m
	//da: pminub mm*, r/m
	//db: pand mm*, r/m
	//dc: paddusb mm*, r/m
	//dd: paddusw mm*, r/m
	//de: pmaxub mm*, r/m
	//df: pandn mm*, r/m

	//e0: pavgb mm*, r/m
	//e1: psraw mm*, r/m
	//e2: psrad mm*, r/m
	//e3: pavgw mm*, r/m
	//e4: pmulhuw mm*, r/m
	//e5: pmulhw mm*, r/m
	//e6: pmulhw mm*, r/m

	//e7: movntq r/m, mm*
	//e8: psubsb mm*, r/m
	//e9: psubsw mm*, r/m
	//ea: pminsw mm*, r/m
	//eb: por mm*, r/m
	//ec: paddsb mm*, r/m
	//ed: paddsw mm*, r/m
	//ee: pmaxsw mm*, r/m
	//ef: pxor mm*, r/m

	//f1: psllw mm*, r/m
	//f2: pslld mm*, r/m
	//f3: psllq mm*, r/m
	//f4: pmuludq mm*, r/m
	//f5: pmaddwd mm*, r/m
	//f6: psadbw mm*, r/m
	//f7c0: maskmovq mm*, mm*
	//f8: psubb mm*, r/m
	//f9: psubw mm*, r/m
	//fa: psubd mm*, r/m
	//fb: psubq mm*, r/m
	//fc: paddb mm*, r/m
	//fd: paddw mm*, r/m
	//fe: paddd mm*, r/m

	if(0xff == p[1]){
		disasm_x8664_print(p,2);
		printf("ud0	trigger(#ud0)\n");
		return 2;
	}

	disasm_x8664_prefixprint(pre, p, 1);
	printf("0f	error\n");
	return 1;
}
int disasm_x8664_4x(u8* pre, u8* p, u64 rip)
{
	int k;
	if(0x40 == p[0]){
		if(0x56 == p[1]){
			disasm_x8664_print(p,2);
			printf("push rsi\n");
			return 2;
		}
	}
	else if(0x41 == p[0]){
		k = p[1] & 0xf8;
		if(0x50 == k){
			k = p[1]&7;
			disasm_x8664_print(p,2);
			printf("push	%s\n", reg64[k+8]);
			return 2;
		}
		if(0x58 == k){
			k = p[1]&7;
			disasm_x8664_print(p,2);
			printf("pop	%s\n", reg64[k+8]);
			return 2;
		}
	}

	if(0x98 == p[1]){
		disasm_x8664_print(p,2);
		printf("cwde\n");
		return 2;
	}
	else if(0x0f == p[1]){
		k = disasm_x8664_0f(p, p+1, rip+1);
		if(k > 0)return k+1;
	}
	else{
		k = disasm_x8664_normal(p, p+1, rip+1);
		if(k > 0)return k+1;
	}

	disasm_x8664_print(p,1);
	printf("error\n");
	return 1;
}
int disasm_x8664_66(u8* pre, u8* p, u64 rip)
{
	if((0x0f == p[1])&&(0x00 == p[2])&&(0xc8 == p[3]))
	{
		disasm_x8664_print(p,4);
		printf("str ax\n");
		return 4;
	}
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
	if((0x66 == p[1])&&(0x2e == p[2])&&(0x0f == p[3])&&(0x1f == p[4]))
	{
		disasm_x8664_print(p,11);
		printf("nop11\n");
		return 11;
	}
	if((0x2e == p[1])&&(0x0f == p[2])&&(0x1f == p[3]))
	{
		disasm_x8664_print(p,10);
		printf("nop10\n");
		return 10;
	}
	if(0x90 == p[1])
	{
		disasm_x8664_print(p,2);
		printf("xchg	ax,ax\n");
		return 2;
	}

	int j;
	for(j=0;j<8;j++){
		if(0x66 == p[j])continue;
		if(0x67 == p[j])continue;
		if(0x40 == (p[j]&0xf0))continue;
		break;
	}

	int k = disasm_x8664_normal(p, p+j, rip+j);
	if(k > 0)return k+j;

	disasm_x8664_print(p,1);
	printf("error\n");
	return 1;
}
int disasm_x8664_67(u8* pre, u8* opc, u64 rip)
{
	int k = disasm_x8664_normal(opc, opc+1, rip+1);
	if(k > 0)return k+1;

	disasm_x8664_print(opc,1);
	printf("error\n");
	return 1;
}
int disasm_x8664_one(u8* buf, u64 rip)
{
	printf("%8llx:	", rip);

	if(disasm_x8664_1b(buf, rip)){
		return 1;
	}

	if(0x0f == buf[0]){
		return disasm_x8664_0f(buf, buf, rip);
	}
	if(0x40 == (buf[0]&0xf0)){
		return disasm_x8664_4x(buf, buf, rip);
	}
	if(0x66 == buf[0]){
		return disasm_x8664_66(buf, buf, rip);
	}
	if(0x67 == buf[0]){
		return disasm_x8664_67(buf, buf, rip);
	}

	int k = disasm_x8664_normal(buf, buf, rip);
	if(k > 0)return k;

	disasm_x8664_print(buf,1);
	printf("error\n");
	return 1;
}
void disasm_x8664_all(u8* buf, int len, u64 rip)
{
	int j = 0;
	while(1)
	{
		if(j>=len)break;

		j += disasm_x8664_one(buf+j, rip+j);
	}//while
}




struct offlen{
	u8 off;
	u8 len;
};
struct typedata{
	int type;
	int data;
};
u32 x8664_str2data(u8* p){
	char* buf = (char*)p;
	if( ('0' == buf[0]) && ('x' == buf[1]) )return strtol((char*)buf+2, 0, 16);
	else return atoi((char*)buf);
}
int which08(u8* buf, int len)
{
	int j;
	for(j=0;j<16;j++){
		if(0 == strncmp(reg08[j], (char*)buf, len)){
			return j;
		}
	}
	return -1;
}
int which16(u8* buf, int len)
{
	int j;
	for(j=0;j<16;j++){
		if(0 == strncmp(reg16[j], (char*)buf, len)){
			return j;
		}
	}
	return -1;
}
int which32(u8* buf, int len)
{
	int j;
	for(j=0;j<16;j++){
		if(0 == strncmp(reg32[j], (char*)buf, len)){
			return j;
		}
	}
	return -1;
}
int which64(u8* buf, int len)
{
	int j;
	for(j=0;j<16;j++){
		if(0 == strncmp(reg64[j], (char*)buf, len)){
			return j;
		}
	}
	return -1;
}
int whichreg(u8* buf, int len, int* type, int* reg)
{
	int ret;

	ret = which64(buf,len);
	if(ret >= 0){
		*type = 64;
		*reg = ret;
		return 1;
	}

	ret = which32(buf,len);
	if(ret >= 0){
		*type = 32;
		*reg = ret;
		return 1;
	}

	ret = which16(buf,len);
	if(ret >= 0){
		*type = 16;
		*reg = ret;
		return 1;
	}

	ret = which08(buf,len);
	if(ret >= 0){
		*type = 8;
		*reg = ret;
		return 1;
	}

	if(buf[0]>='0' && buf[0]<='9'){
		*type = 0;
		*reg = x8664_str2data(buf);
		return 1;
	}
	return 0;
}
int x8664_scaleindexbaseimm(
	u8* buf, int len, struct offlen* tab, int cnt,
	struct typedata* td)
{
	int j;

	for(j=0;j<cnt;j++){
		if(']'==buf[tab[j].off])break;
		//printf("%d:%.*s\n", j, tab[j].len, buf+tab[j].off);
	}
	//printf("j=%d\n", j);

	int type0 = -1, data0 = -1;
	int type2 = -1, data2 = -1;
	int type4 = -1, data4 = -1;
	int type6 = -1, data6 = -1;
	if(j>=1){
		whichreg(buf+tab[0].off, tab[0].len, &type0, &data0);
	}
	if(j>=3){
		whichreg(buf+tab[2].off, tab[2].len, &type2, &data2);
	}
	if(j>=5){
		whichreg(buf+tab[4].off, tab[4].len, &type4, &data4);
	}
	if(j>=7){
		whichreg(buf+tab[6].off, tab[6].len, &type6, &data6);
	}

	if(1 == j){		//	1 arg
		//	0x8
		if(0 == type0){
			td[0].type = td[1].type = td[2].type = -1;
			td[3].type = 1;

			td[0].data = 0;
			td[1].data = 0;
			td[2].data = 0;
			td[3].data = data0;
			printf("1op: imm=0x%x\n", data0);
		}
		//	rax
		else{
			td[0].type = td[1].type = td[3].type = -1;
			td[2].type = 1;

			td[0].data = 0;
			td[1].data = 0;
			td[2].data = data0;
			td[3].data = 0;
			printf("1op: base=%d\n", data0);
		}
	}
	else if(3 == j){		//	2 arg
		//	0x8 * rax
		//	rax * 0x8
		if('*' == buf[tab[1].off]){
			td[0].type = td[1].type = 1;
			td[2].type = td[3].type =-1;

			if(0 == type0){
				td[0].data = data0;
				td[1].data = data2;
				td[2].data = 0;
				td[3].data = 0;
				printf("2op: scale=%d,index=%d\n", data0, data2);
			}
			else if(0 == type2){
				td[0].data = data2;
				td[1].data = data0;
				td[2].data = 0;
				td[3].data = 0;
				printf("2op: scale=%d,index=%d\n", data2, data0);
			}
		}
		//	0x8 + rax
		//	rax + 0x8
		else if('+' == buf[tab[1].off]){
			td[0].type = td[1].type =-1;
			td[2].type = td[3].type = 1;
			if(0 == type0){
				td[0].data = 0;
				td[1].data = 0;
				td[2].data = data2;
				td[3].data = data0;
				printf("2op: base=%d,imm=0x%x\n", data2, data0);
			}
			else if(0 == type2){
				td[0].data = 0;
				td[1].data = 0;
				td[2].data = data0;
				td[3].data = data2;
				printf("2op: base=%d,imm=0x%x\n", data0, data2);
			}
		}
	}
	else if(5 == j){		//	3 arg
		//	* +
		//	rax * 0x8 + 0x5
		//	rax * 0x8 + rbx
		//	0x8 * rax + 0x5
		//	0x8 * rax + rbx
		if('*' == buf[tab[1].off]){
			if(0 == type2){
				td[0].data = data2;
				td[1].data = data0;
				if(0 == type4){
					td[2].data = 0;
					td[3].data = data4;
					printf("3op: scale=%d,index=%d,imm=%d\n", data2, data0, data4);
					td[0].type = td[1].type = td[3].type =1;
					td[2].type = -1;
				}
				else{
					td[2].data = data4;
					td[3].data = 0;
					printf("3op: scale=%d,index=%d,base=%d\n", data2, data0, data4);
					td[0].type = td[1].type = td[2].type =1;
					td[3].type = -1;
				}
			}
			else if(0 == type0){
				td[0].data = data0;
				td[1].data = data2;
				if(0 == type4){
					td[2].data = 0;
					td[3].data = data4;
					printf("3op: scale=%d,index=%d,base=%d\n", data0, data2, data4);
					td[0].type = td[1].type = td[3].type =1;
					td[2].type = -1;
				}
				else{
					td[2].data = data4;
					td[3].data = 0;
					printf("3op: scale=%d,index=%d,base=%d\n", data0, data2, data4);
					td[0].type = td[1].type = td[2].type =1;
					td[3].type = -1;
				}
			}
		}
		//	+ *
		//	0x3 + rbx * 0x8
		//	rax + rbx * 0x8
		//	0x3 + 0x8 * rbx
		//	rax + 0x8 * rbx
		else if('*' == buf[tab[3].off]){
			if(0 == type4){
				td[0].data = data4;
				td[1].data = data2;
				if(0 == type0){
					td[2].data = 0;
					td[3].data = data0;
					printf("3op: scale=%d,index=%d,imm=%d\n", data4, data2, data0);
					td[0].type = td[1].type = td[3].type =1;
					td[2].type = -1;
				}
				else{
					td[2].data = data0;
					td[3].data = 0;
					printf("3op: scale=%d,index=%d,base=%d\n", data4, data2, data0);
					td[0].type = td[1].type = td[2].type =1;
					td[3].type = -1;
				}
			}
			else if(0 == type2){
				td[0].data = data2;
				td[1].data = data4;
				if(0 == type0){
					td[2].data = 0;
					td[3].data = data0;
					printf("3op: scale=%d,index=%d,imm=%d\n", data2, data4, data0);
					td[0].type = td[1].type = td[3].type =1;
					td[2].type = -1;
				}
				else{
					td[2].data = data0;
					td[3].data = 0;
					printf("3op: scale=%d,index=%d,base=%d\n", data2, data4, data0);
					td[0].type = td[1].type = td[2].type =1;
					td[3].type = -1;
				}
			}
		}
		//	+ +
		//	rax + rbx + 0x8
		//	rax + 0x8 + rbx
		//	0x8 + rax + rbx
		else if( ('+' == buf[tab[1].off]) && ('+' == buf[tab[3].off]) ){
			td[0].type = td[1].type = td[2].type = td[3].type = 1;
			if(0 == type4){
				td[0].data = 1;
				td[1].data = data0;
				td[2].data = data2;
				td[3].data = data4;
				printf("3op: index=%d,base=%d,imm=0x%x\n", data0, data2, data4);
			}
			else if(0 == type2){
				td[0].data = 1;
				td[1].data = data0;
				td[2].data = data4;
				td[3].data = data2;
				printf("3op: index=%d,base=%d,imm=0x%x\n", data0, data4, data2);
			}
			else if(0 == type0){
				td[0].data = 1;
				td[1].data = data2;
				td[2].data = data4;
				td[3].data = data0;
				printf("3op: index=%d,base=%d,imm=0x%x\n", data2, data4, data0);
			}
		}
	}
	else if(7 == j){		//	4 arg
		td[0].type = td[1].type = td[2].type = td[3].type = 1;
		//	* + +
		//	rax * 0x8 + rbx + 0x4
		//	rax * 0x8 + 0x4 + rbx
		//	0x8 * rax + rbx + 0x4
		//	0x8 * rax + 0x4 + rbx
		if('*' == buf[tab[1].off]){
			if( (0 == type2) && (0 == type6) ){
				td[0].data = data2;
				td[1].data = data0;
				td[2].data = data4;
				td[3].data = data6;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data2, data0, data4, data6);
			}
			else if( (0 == type2) && (0 == type4) ){
				td[0].data = data2;
				td[1].data = data0;
				td[2].data = data6;
				td[3].data = data4;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data2, data0, data6, data4);
			}
			else if( (0 == type0) && (0 == type6) ){
				td[0].data = data0;
				td[1].data = data2;
				td[2].data = data4;
				td[3].data = data6;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data0, data2, data4, data6);
			}
			else if( (0 == type0) && (0 == type4) ){
				td[0].data = data0;
				td[1].data = data2;
				td[2].data = data6;
				td[3].data = data4;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data0, data2, data6, data4);
			}
		}
		//	+ * +
		//	rbx + rax * 0x8 + 0x4
		//	0x4 + rax * 0x8 + rbx
		//	rbx + 0x8 * rax + 0x4
		//	0x4 + 0x8 * rax + rbx
		if('*' == buf[tab[3].off]){
			if( (0 == type4) && (0 == type6) ){
				td[0].data = data4;
				td[1].data = data2;
				td[2].data = data0;
				td[3].data = data6;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data4, data2, data0, data6);
			}
			else if( (0 == type0) && (0 == type4) ){
				td[0].data = data4;
				td[1].data = data2;
				td[2].data = data6;
				td[3].data = data0;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data4, data2, data6, data0);
			}
			else if( (0 == type2) && (0 == type6) ){
				td[0].data = data2;
				td[1].data = data4;
				td[2].data = data0;
				td[3].data = data6;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data2, data4, data0, data6);
			}
			else if( (0 == type0) && (0 == type2) ){
				td[0].data = data2;
				td[1].data = data4;
				td[2].data = data6;
				td[3].data = data0;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data2, data4, data6, data0);
			}
		}
		//	+ + *
		//	rbx + 0x4 + rax * 0x8
		//	0x4 + rbx + rax * 0x8
		//	rbx + 0x4 + 0x8 * rax
		//	0x4 + rbx + 0x8 * rax
		if('*' == buf[tab[5].off]){
			if( (0 == type2) && (0 == type6) ){
				td[0].data = data6;
				td[1].data = data4;
				td[2].data = data0;
				td[3].data = data2;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data6, data4, data0, data2);
			}
			else if( (0 == type0) && (0 == type6) ){
				td[0].data = data6;
				td[1].data = data4;
				td[2].data = data2;
				td[3].data = data0;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data6, data4, data2, data0);
			}
			else if( (0 == type2) && (0 == type4) ){
				td[0].data = data4;
				td[1].data = data6;
				td[2].data = data0;
				td[3].data = data2;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data4, data6, data0, data2);
			}
			else if( (0 == type0) && (0 == type4) ){
				td[0].data = data4;
				td[1].data = data6;
				td[2].data = data2;
				td[3].data = data0;
				printf("4op: scale=%d,index=%d,base=%d,imm=0x%x\n", data4, data6, data2, data0);
			}
		}
	}
	return j;
}
int get_reg_or_sib(u8* buf, int len, struct offlen* tab, int cnt, struct typedata* td)
{
	//printf("%c\n", buf[tab[0].off]);
	if('[' != buf[tab[0].off]){
		whichreg(buf+tab[0].off, tab[0].len, &td[0].type, &td[0].data);
		printf("%x,%x\n", td[0].type, td[0].data);
		return 0;
	}
	else{
		td[0].type = 99;
		td[0].data = 0;
		int ret = x8664_scaleindexbaseimm(buf,len, &tab[1],cnt-1, &td[1]);
		if(td[1].data >= 0){
			if(1==td[1].data)td[1].data = 0;
			else if(2==td[1].data)td[1].data = 1;
			else if(4==td[1].data)td[1].data = 2;
			else if(8==td[1].data)td[1].data = 3;
		}
		return ret+1;
	}
}
void modrm_sib_1dstnsrc(u8* bin, struct typedata* dst, struct typedata* src)
{
	struct typedata* _s = &src[0];
	struct typedata* _i = &src[1];
	struct typedata* _b = &src[2];
	struct typedata* _d = &src[3];
	if( (_i->type < 0) && (_b->type < 0) ){	//mov rax,[0x1234]		//index=none,base=none
		bin[2] = (0<<6) | (dst[0].data<<3) | (4);
		bin[3] = 0x25;
		*(int*)(bin+4) = _d->data;
	}
	else if( (_i->type < 0) && (_d->type < 0) ){	//mov rax,[rbx]		//index=none,imm=none
		if(5 == _b->data){	//mov rax,[rbp]
			bin[2] = (1<<6) | (dst[0].data<<3) | (_b->data);
			bin[3] = 0;
		}
		else{
			bin[2] = (0<<6) | (dst[0].data<<3) | (_b->data);
		}
	}
	else if(_i->type < 0){	//mov rax,[rbp+0x1234]		//index=none
		if(_d->data < 0x100){
			bin[2] = (1<<6) | (dst[0].data<<3) | (_b->data);		//mod r r/m
			bin[3] = _d->data;
		}
		else{
			bin[2] = (2<<6) | (dst[0].data<<3) | (_b->data);		//mod r r/m
			*(int*)(bin+3) = _d->data;
		}
	}
	else if(_b->type < 0){	//mov rax,[rax*8+0x1234]	//base=none
		bin[2] = (0<<6) | (dst[0].data<<3) | (4);		//mod r r/m
		bin[3] = (_s->data<<6) | (_i->data<<3) | (5);	//sib
		*(int*)(bin+4) = _d->data;
	}
	else{	//mov rax,[rcx*8+rbp+0x1234]
		bin[2] = (2<<6) | (dst[0].data<<3) | (4);		//mod r r/m
		bin[3] = (_s->data<<6) | (_i->data<<3) | (_b->data);	//sib
		*(int*)(bin+4) = _d->data;
	}
}
void modrm_sib_ndst1src(u8* bin, struct typedata* dst, struct typedata* src)
{
	struct typedata* _s = &dst[0];
	struct typedata* _i = &dst[1];
	struct typedata* _b = &dst[2];
	struct typedata* _d = &dst[3];
	if( (_i->type < 0) && (_b->type < 0) ){	//mov rax,[0x1234]		//index=none,base=none
		bin[2] = (0<<6) | (src[0].data<<3) | (4);
		bin[3] = 0x25;
		*(int*)(bin+4) = _d->data;
	}
	else if( (_i->type < 0) && (_d->type < 0) ){	//mov rax,[rbx]		//index=none,imm=none
		if(5 == _b->data){	//mov rax,[rbp]
			bin[2] = (1<<6) | (src[0].data<<3) | (_b->data);
			bin[3] = 0;
		}
		else{
			bin[2] = (0<<6) | (src[0].data<<3) | (_b->data);
		}
	}
	else if(_i->type < 0){	//mov rax,[rbp+0x1234]		//index=none
		if(_d->data < 0x100){
			bin[2] = (1<<6) | (src[0].data<<3) | (_b->data);		//mod r r/m
			bin[3] = _d->data;
		}
		else{
			bin[2] = (2<<6) | (src[0].data<<3) | (_b->data);		//mod r r/m
			*(int*)(bin+3) = _d->data;
		}
	}
	else if(_b->type < 0){	//mov rax,[rax*8+0x1234]	//base=none
		bin[2] = (0<<6) | (src[0].data<<3) | (4);		//mod r r/m
		bin[3] = (_s->data<<6) | (_i->data<<3) | (5);	//sib
		*(int*)(bin+4) = _d->data;
	}
	else{	//mov rax,[rcx*8+rbp+0x1234]
		bin[2] = (2<<6) | (src[0].data<<3) | (4);		//mod r r/m
		bin[3] = (_s->data<<6) | (_i->data<<3) | (_b->data);	//sib
		*(int*)(bin+4) = _d->data;
	}
}
void assembly_x8664_opcode(u8* bin, int opcode, struct typedata* src, struct typedata* dst)
{
	if( (dst[0].type==16) && (src[0].type==16) ){
		bin[0] = 0x66;
		bin[1] = opcode;
		bin[2] = (dst[0].data) | (src[0].data<<3) | (3<<6);
	}
	if( (dst[0].type==32) && (src[0].type==32) ){
		bin[0] = opcode;
		bin[1] = (dst[0].data) | (src[0].data<<3) | (3<<6);
	}
	if( (dst[0].type==64) && (src[0].type==64) ){
		bin[0] = 0x48;
		bin[1] = opcode;
		bin[2] = (dst[0].data) | (src[0].data<<3) | (3<<6);
	}
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x34;
			bin[1] = src[0].data;
		}
		else{
			//todo: all reg except al is not allowed?
			bin[0] = 0x80;
			bin[1] = 0xf0 | (dst[0].data);
			bin[2] = src[0].data;
		}
	}
	if( (dst[0].type==16) && (src[0].type==0) ){
		bin[0] = 0x66;
		bin[1] = opcode;
		bin[2] = (dst[0].data) | (3<<6);
		bin[3] = src[0].data;
	}
	if( (dst[0].type==32) && (src[0].type==0) ){
		bin[0] = opcode;
		bin[1] = (dst[0].data) | (3<<6);
		bin[2] = src[0].data;
	}
	if( (dst[0].type==64) && (src[0].type==0) ){
		bin[0] = 0x48;
		bin[1] = opcode;
		bin[2] = (dst[0].data) | (3<<6);
		bin[3] = src[0].data;
	}

	if( (64==dst[0].type) && (99==src[0].type) ){	//add rax,[]
		printf("%x,%x,%x,%x\n",src[1].data,src[2].data,src[3].data,src[4].data);
		bin[0] = 0x48;
		bin[1] = opcode | 2;
		modrm_sib_1dstnsrc(bin, dst, &src[1]);
	}
	if( (99==dst[0].type) && (64==src[0].type) ){	//add [],rax
		printf("%x,%x,%x,%x\n",dst[1].data,dst[2].data,dst[3].data,dst[4].data);
		bin[0] = 0x48;
		bin[1] = opcode;
		modrm_sib_ndst1src(bin, &dst[1], src);
	}
}
#define OPCODE_LEA 0x8d
void assembly_x8664_lea(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst;		//regorbracket, scale, index, base, data
	whichreg(buf+tab[0].off, tab[0].len, &dst.type, &dst.data);

	struct typedata src[4];
	int ret = x8664_scaleindexbaseimm(buf,len, &tab[2],cnt-2, src);

	u8 bin[8];
	if( (64==dst.type) ){	//mov rax,[]
		printf("%x,%x,%x,%x\n",src[0].data,src[1].data,src[2].data,src[3].data);
		bin[0] = 0x48;
		bin[1] = OPCODE_LEA;
		modrm_sib_1dstnsrc(bin, &dst, src);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_MOV 0x89
void assembly_x8664_mov(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (8==dst[0].type) && (8==src[0].type) ){
		bin[0] = 0x88;
		bin[1] = (dst[0].data) | (src[0].data<<3) | (3<<6);
	}
	else if( (dst[0].type==8) && (src[0].type==0) ){
		bin[0] = 0xb0 | (dst[0].data);
		bin[1] = src[0].data;
	}
	else if( (dst[0].type==16) && (src[0].type==0) ){
		bin[0] = 0x66;
		bin[1] = 0xb0 | (dst[0].data);
		*(short*)(bin+2) = src[0].data;
	}
	else if( (dst[0].type==32) && (src[0].type==0) ){
		bin[0] = 0xb8 | (dst[0].data);
		*(int*)(bin+1) = src[0].data;
	}
	else if( (dst[0].type==64) && (src[0].type==0) ){
		bin[0] = 0x48;
		bin[1] = 0xc7;
		bin[2] = 0xc0 | (dst[0].data);
		*(int*)(bin+3) = src[0].data;
	}
	else if( (99==dst[0].type) && (0==src[0].type) ){	//mov [],0x12	//mov [],0x1234	//mov [],0x12345678
		bin[0] = 0xc7;
		bin[1] = (1<<6) | (0<<3) | (dst[3].data);
		bin[2] = dst[4].data;
		*(int*)(bin+3) = src[0].data;
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_MOV, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_ADD 0x1
void assembly_x8664_add(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x04;
			bin[1] = src[0].data;
		}
		else{
			bin[0] = 0x80;
			bin[1] = 0xc0 | (dst[0].data);
			bin[2] = src[0].data;
		}
	}
	else if( (dst[0].type==16) && (src[0].type==0) ){
		bin[0] = 0x66;
		bin[1] = 0x83;
		bin[2] = 0xc0 | (dst[0].data);
		bin[3] = src[0].data;
	}
	else if( (dst[0].type==32) && (src[0].type==0) ){
		bin[0] = 0x83;
		bin[1] = 0xc0 | (dst[0].data);
		bin[2] = src[0].data;
	}
	else if( (dst[0].type==64) && (src[0].type==0) ){
		bin[0] = 0x48;
		bin[1] = 0x83;
		bin[2] = 0xc0 | (dst[0].data);
		bin[3] = src[0].data;
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_ADD, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_SUB 0x29
void assembly_x8664_sub(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x2c;
			bin[1] = src[0].data;
		}
		else{
			bin[0] = 0x80;
			bin[1] = 0xe8 | (dst[0].data);
			bin[2] = src[0].data;
		}
	}
	else if( (dst[0].type==16) && (src[0].type==0) ){
		bin[0] = 0x66;
		bin[1] = 0x83;
		bin[2] = 0xe8 | (dst[0].data);
		bin[3] = src[0].data;
	}
	else if( (dst[0].type==32) && (src[0].type==0) ){
		bin[0] = 0x83;
		bin[1] = 0xe8 | (dst[0].data);
		bin[2] = src[0].data;
	}
	else if( (dst[0].type==64) && (src[0].type==0) ){
		bin[0] = 0x48;
		bin[1] = 0x83;
		bin[2] = 0xe8 | (dst[0].data);
		bin[3] = src[0].data;
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_SUB, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_CMP 0x39
void assembly_x8664_cmp(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x04;
			bin[1] = src[0].data;
		}
		else{
			//todo: all reg except al is not allowed?
			bin[0] = 0x80;
			bin[1] = (dst[0].data) | (3<<6);
			bin[2] = src[0].data;
		}
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_CMP, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_OR 0x9
void assembly_x8664_or(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x04;
			bin[1] = src[0].data;
		}
		else{
			//todo: all reg except al is not allowed?
			bin[0] = 0x80;
			bin[1] = (dst[0].data) | (3<<6);
			bin[2] = src[0].data;
		}
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_OR, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_AND 0x21
void assembly_x8664_and(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x04;
			bin[1] = src[0].data;
		}
		else{
			//todo: all reg except al is not allowed?
			bin[0] = 0x80;
			bin[1] = (dst[0].data) | (3<<6);
			bin[2] = src[0].data;
		}
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_AND, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_XOR 0x31
void assembly_x8664_xor(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
		if(0 == dst[0].data){
			bin[0] = 0x34;
			bin[1] = src[0].data;
		}
		else{
			//todo: all reg except al is not allowed?
			bin[0] = 0x80;
			bin[1] = 0xf0 | (dst[0].data);
			bin[2] = src[0].data;
		}
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_XOR, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_TEST 0x85
void assembly_x8664_test(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (64==dst[0].type) && (99==src[0].type) ){	//add rax,[]
		//error
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_TEST, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_XCHG 0x87
void assembly_x8664_xchg(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

	u8 bin[8];
	if( (99==dst[0].type) && (64==src[0].type) ){	//add [],rax
		//error
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_XCHG, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
#define OPCODE_IMUL 0x0f
void assembly_x8664_imul(u8* buf, int len, struct offlen* tab, int cnt)
{
	struct typedata dst[5];		//regorbracket, scale, index, base, data
	int end0 = 0 + get_reg_or_sib(buf,len, tab,cnt, dst);

	struct typedata src[5];
	int end1 = end0+2 + get_reg_or_sib(buf,len, &tab[end0+2],cnt-(end0+2), src);

/*
imul ecx		// eax = eax * ecx, edx=0
imul ecx,2		// ecx *= 2
imul eax,ecx	// eax *= ecx
imul ecx,edx,2	// ecx = edx * 2
*/
	u8 bin[8];
	if( (dst[0].type==8) && (src[0].type==0) ){
	}
	else{
		assembly_x8664_opcode(bin, OPCODE_IMUL, src, dst);
	}
	disasm_x8664_one(bin, 0);
}
void assembly_compile_x8664(u8* buf, int len, struct offlen* tab, int cnt)
{
/*	int j;
	for(j=0;j<cnt;j++){
		printf("%d: %.*s\n", j, tab[j].len, buf+tab[j].off);
	}
*/
	if(0 == strncmp((char*)buf+tab[0].off, "lea", 3)){
		assembly_x8664_lea(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "mov", 3)){
		assembly_x8664_mov(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "add", 3)){
		assembly_x8664_add(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "sub", 3)){
		assembly_x8664_sub(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "cmp", 3)){
		assembly_x8664_cmp(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "or", 2)){
		assembly_x8664_or(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "and", 3)){
		assembly_x8664_and(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "xor", 3)){
		assembly_x8664_xor(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "test", 4)){
		assembly_x8664_test(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "xchg", 4)){
		assembly_x8664_xchg(buf, len, &tab[1], cnt-1);
	}
	else if(0 == strncmp((char*)buf+tab[0].off, "imul", 4)){
		assembly_x8664_imul(buf, len, &tab[1], cnt-1);
	}
}