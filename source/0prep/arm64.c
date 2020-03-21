#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long


/*
0 EQ equal (zero)
1 NE nonequal (nonzero)
2 CS=HS carry set
3 CC=LO carry clear
4 MI minus
5 PL positive or zero
6 VS overflow set
7 VC overflow clear
8 HI unsigned higher
9 LS unsigned lower or same
a GE signed greater or equal ?
b LT signed less ?
c GT signed greater than
d LE signed less than or equal
e AL always ?
f NV never ?
*/
static char* cond = "eq\0ne\0hs\0lo\0mi\0pl\0vs\0vc\0hi\0ls\0ge\0lt\0gt\0le\0al\0nv\0";
static int unknown[256];


//v=0: return 0b0001
//v=1: return 0b0011
//v=2: retrun 0b0111
//v=3: return 0b1111
u32 table32(int v)
{
	int j;
	u32 tmp = 0;
	for(j=0;j<v+1;j++)tmp |= ((u32)1<<j);
	return tmp;
}
u32 rotateleft32(u32 in, int c)
{
	u32 tmp = in>>(32-c);
	return (in<<c) | tmp;
}
u64 table64(int v)
{
	int j;
	u64 tmp = 0;
	for(j=0;j<v+1;j++)tmp |= ((u64)1<<j);
	return tmp;
}
u64 rotateleft64(u64 in, int c)
{
	u64 tmp = in>>(64-c);
	return (in<<c) | tmp;
}




void disasm_arm64_one(u8* buf, int len)
{
int j,cnt=0;
u32 code;
for(j=0;j<255;j++)unknown[j] = 0;

for(j=0;j<len;j+=4){
	code = *(u32*)(buf+j);
	if(0xd503201f == code){
		printf("%x:	nop\n", j);
	}
	else if(0xd65f03c0 == code){
		printf("%x:	ret\n", j);
	}
//-----------------jmp---------------------
	else if(0x14000000 == (code&0xfc000000)){
		u32 off = code&0x3ffffff;
		if(off&0x2000000){
			off = (0x4000000-off)<<2;
			printf("%x:	b 0x%x (pc-=0x%x)\n", j, j-off, off);
		}
		else{
			off = off<<2;
			printf("%x:	b 0x%x (pc+=0x%x)\n", j, j+off, off);
		}
	}
	else if(0x94000000 == (code&0xfc000000)){
		u32 off = code&0x3ffffff;
		if(off&0x2000000){
			off = (0x4000000-off)<<2;
			printf("%x:	bl 0x%x (lr=0x%x,pc-=0x%x)\n", j, j-off, j+4, off);
		}
		else{
			off = off<<2;
			printf("%x:	bl 0x%x (lr=0x%x,pc+=0x%x)\n", j, j+off, j+4, off);
		}
	}
	else if(0x54 == buf[j+3]){
		u8 c = code&0x1f;
		u32 off = (code>>5)&0x7ffff;
		printf("%x:	if(%s)b ", j, cond+c*3);
		if(code&0x800000){
			off = (0x80000-off)<<2;
			printf("0x%x(pc-=0x%x)\n", j-off, off);
		}
		else{
			off = off<<2;
			printf("0x%x(pc+=0x%x)\n", j+off, off);
		}
	}
//-----------------mov-------------------
	else if(0xaa0003e0 == (code&0xffc003e0)){
		printf("%x:	x%d = x%d\n", j, code&0x1f, (code>>16)&0x1f);
	}
	else if(0x2a0003e0 == (code&0xffc003e0)){
		printf("%x:	w%d = w%d\n", j, code&0x1f, (code>>16)&0x1f);
	}
	else if(0xd2800000 == (code&0xfff00000)){
		printf("%x:	x%d = %d\n", j, (code&0x1f), (code>>5)&0x7fff);
	}
	else if(0x52800000 == (code&0xfff00000)){
		printf("%x:	w%d = %d\n", j, (code&0x1f), (code>>5)&0x7fff);
	}
	else if(0xf2800000 == (code&0xff800000)){
		u8 r0 = code&0x1f;
		u8 sh = ((code>>21)&0x3)<<4;
		u32 val = (code>>5)&0xffff;
		printf("%x:	x%d.[%d,%d] = %d\n", j, r0, sh,sh+15, val);
	}
	else if(0x72800000 == (code&0xff800000)){
		u8 r0 = code&0x1f;
		u8 sh = ((code>>21)&0x3)<<4;
		u32 val = (code>>5)&0xffff;
		printf("%x:	w%d.[%d,%d] = %d\n", j, r0, sh,sh+15, val);
	}
//-----------------cmp------------------
	else if(0xeb00001f == (code&0xffc0001f)){
		if(code&0x200000)printf("%x:	x%d ? (u64)w%d\n", j, (code>>5)&0x1f, (code>>16)&0x1f);
		else printf("%x:	x%d ? x%d\n", j, (code>>5)&0x1f, (code>>16)&0x1f);
	}
	else if(0x6b00001f == (code&0xffc0001f)){
		printf("%x:	w%d ? w%d\n", j, (code>>5)&0x1f, (code>>16)&0x1f);
	}
	else if(0xf100001f == (code&0xffc0001f)){
		printf("%x:	x%d ? %d\n", j, (code>>5)&0x1f, (code>>10)&0xfff);
	}
	else if(0x7100001f == (code&0xffc0001f)){
		printf("%x:	w%d ? %d\n", j, (code>>5)&0x1f, (code>>10)&0xfff);
	}
//-----------------add-----------------
	else if(0x8b000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		u8 sh = (code>>10)&0x3f;
		printf("%x:	x%d = x%d + ", j, r0, r1);

		if(code&0x200000)printf("(u64)w%d\n", r2);
		else if(code&0x400000)printf("x%d>>%d\n", r2, sh);
		else if(sh)printf("x%d<<%d\n", r2, sh);
		else printf("x%d\n", r2);
	}
	else if(0x91000000 == (code&0xff400000)){
		printf("%x:	x%d = x%d + %d\n", j, code&0x1f, (code>>5)&0x1f, (code>>10)&0xfff);
	}
	else if(0x91400000 == (code&0xff400000)){
		printf("%x:	x%d = x%d + %d<<12\n", j, code&0x1f, (code>>5)&0x1f, (code>>10)&0xfff);
	}
	else if(0x11000000 == (code&0xff000000)){
		printf("%x:	w%d = w%d + %d\n", j, code&0x1f, (code>>5)&0x1f, (code>>10)&0xfff);
	}
//--------------------sub----------------------
	else if(0xcb000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	x%d = x%d - x%d\n", j, r0, r1, r2);
	}
	else if(0x4b000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	w%d = w%d - w%d\n", j, r0, r1, r2);
	}
	else if(0xd1000000 == (code&0xff400000)){
		printf("%x:	x%d = x%d - %d\n", j, code&0x1f, (code>>5)&0x1f, (code>>10)&0xfff);
	}
	else if(0xd1400000 == (code&0xff400000)){
		printf("%x:	x%d = x%d - %d<<12\n", j, code&0x1f, (code>>5)&0x1f, (code>>10)&0xfff);
	}
	else if(0x51000000 == (code&0xff000000)){
		printf("%x:	w%d = w%d - %d\n", j, code&0x1f, (code>>5)&0x1f, (code>>10)&0xfff);
	}
//-----------------and-------------------
	else if(0x8a000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	x%d = x%d & x%d\n", j, r0, r1, r2);
	}
	else if(0x0a000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	w%d = w%d & w%d\n", j, r0, r1, r2);
	}
	else if(0x92000000 == (code&0xff800000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		printf("%x:	x%d = x%d & ", j, r0, r1);

		u8 bit10 = (code>>10)&0x3f;
		u8 bit16 = (code>>16)&0x3f;
		u64 mask = table64(bit10);
		if(0x400000 != (code&0xff0000)){
			mask = rotateleft64(mask, 0x80-bit16);
		}
		printf("%llx\n", mask);
	}
	else if(0x12000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		printf("%x:	w%d = w%d & ", j, r0, r1);

		u8 bit10 = (code>>10)&0x1f;
		u8 bit16 = (code>>16)&0x1f;
		u32 mask = table32(bit10);
		//printf("(%x,%x,%x)",bit10,bit16,mask);
		//if(0x000000 == (code&0xff0000)){
			mask = rotateleft32(mask, 0x20-bit16);
		//}
		printf("0x%x\n", mask);
	}
//-----------------orr-------------------
	else if(0xaa000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 sh = (code>>10)&0x3f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	x%d = x%d | x%d<<%d\n", j, r0, r1, r2, sh);
	}
	else if(0x2a000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 sh = (code>>10)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	w%d = w%d | w%d<<%d\n", j, r0, r1, r2, sh);
	}
	else if(0xb2400000 == (code&0xff800000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		printf("%x:	x%d = x%d | ", j, r0, r1);

		u8 bit10 = (code>>10)&0x3f;
		u8 bit16 = (code>>16)&0x3f;
		u64 mask = table64(bit10);
		if(0x400000 != (code&0xff0000)){
			mask = rotateleft64(mask, 0x80-bit16);
		}
		printf("%llx\n", mask);
	}
	else if(0x32000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		printf("%x:	w%d = w%d | ", j, r0, r1);

		u8 bit10 = (code>>10)&0x1f;
		u8 bit16 = (code>>16)&0x1f;
		u32 mask = table32(bit10);
		//printf("(%x,%x,%x)",bit10,bit16,mask);
		//if(0x000000 == (code&0xff0000)){
			mask = rotateleft32(mask, 0x20-bit16);
		//}
		printf("0x%x\n", mask);
	}
//-----------------sh--------------------
	else if(0x9ac02000 == (code&0xffc02000)){
		if(code&0x400){
			printf("%x:	x%d = x%d >> x%d\n", j, code&0x1f, (code>>5)&0x1f, (code>>16)&0x1f);
		}
		else{
			printf("%x:	x%d = x%d << x%d\n", j, code&0x1f, (code>>5)&0x1f, (code>>16)&0x1f);
		}
	}
	else if(0xd3400000 == (code&0xffc00000)){
		u8 lsb = (code>>16)&0x3f;
		u8 msb = (code>>10)&0x3f;
		printf("%x:	x%d = x%d.[%d,%d]\n", j, code&0x1f, (code>>5)&0x1f, lsb, msb);
/*
		if(0xfc <= ((code>>8)&0xff)){
			printf("%x:	x%d = x%d >> %d\n", j, code&0x1f, (code>>5)&0x1f, sh-0x40);
		}
		else{
			printf("%x:	x%d = x%d << %d\n", j, code&0x1f, (code>>5)&0x1f, 0x80-sh);
		}
*/
	}
//---------------mem-----------------
	else if(0x58000000 == (code&0xff800000)){
		u8 r0 = code&0x1f;
		int off = ((code>>5)&0x3ffff)<<2;
		printf("%x:	x%d = [0x%x]\n", j, r0, off);
	}
	else if(0x18000000 == (code&0xff800000)){
		u8 r0 = code&0x1f;
		int off = ((code>>5)&0x3ffff)<<2;
		printf("%x:	w%d = [0x%x]\n", j, r0, off);
	}
	else if(0xb8000400 == (code&0xff400400)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		int off = (code>>12)&0xff;
		if(code&0x100000)off -= 256;
		printf("%x:	[x%d] %+d = w%d\n", j, r1, off, r0);
	}
	else if(0xb8400400 == (code&0xff400400)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		int off = (code>>12)&0xff;
		printf("%x:	w%d = [x%d] %+d\n", j, r0, r1, off);
	}
	else if(0xb9000000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		int off = (code>>10)&0xfff;
		printf("%x:	[x%d %+d] = w%d\n", j, r1, off<<2, r0);
	}
	else if(0xb9400000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		int off = (code>>10)&0xfff;
		printf("%x:	w%d = [x%d %+d]\n", j, r0, r1, off<<2);
	}
	else if(0xf8000000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>15)&0x3f;
		if(code&0x100000)off -= 0x40;

		printf("%x:	[x%d %+d] = x%d\n", j, rr, off*8, r0);
	}
	else if(0xf8400000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>15)&0x3f;
		if(code&0x100000)off -= 0x40;

		printf("%x:	x%d = [x%d %+d]\n", j, r0, rr, off*8);
	}
	else if(0xf9000000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>10)&0x3f;
		printf("%x:	[x%d %+d] = x%d\n", j, rr, off*8, r0);
	}
	else if(0xf9400000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>10)&0x3f;
		printf("%x:	x%d = [x%d %+d]\n", j, r0, rr, off*8);
	}
	else if(0xa9000000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>10)&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>15)&0x3f;
		if(code&0x200000)off -= 0x40;

		if(code&0x800000)printf("%x:	[x%d %+d]! = x%d,x%d\n", j, rr, off*8, r0, r1);
		else printf("%x:	[x%d %+d] = x%d,x%d\n", j, rr, off*8, r0, r1);
	}
	else if(0xa9400000 == (code&0xff400000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>10)&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>15)&0x3f;
		if(code&0x200000)off -= 0x40;

		if(code&0x800000)printf("%x:	x%d,x%d = [x%d %+d]!\n", j, r0, r1, rr, off*8);
		else printf("%x:	x%d,x%d = [x%d %+d]\n", j, r0, r1, rr, off*8);
	}
//--------------???0000000000000000000
	else{
		printf("%x:	%02x %02x %02x %02x\n", j,
		buf[j+0],buf[j+1],buf[j+2],buf[j+3]);
		cnt++;

		unknown[buf[j+3]] += 1;
	}
}//for

for(j=0;j<256;j+=16){
	printf("%04x: %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n",j,
	unknown[j+ 0],unknown[j+ 1],unknown[j+ 2],unknown[j+ 3],
	unknown[j+ 4],unknown[j+ 5],unknown[j+ 6],unknown[j+ 7],
	unknown[j+ 8],unknown[j+ 9],unknown[j+10],unknown[j+11],
	unknown[j+12],unknown[j+13],unknown[j+14],unknown[j+15]
	);
};
printf("fail / total = %d / %d = %d%%\n", cnt, len/4, (100*cnt)/(len/4));
}
void disasm_arm64(int argc, char** argv)
{
	int fd,ret;
	u8 buf[0x100000];
	if(argc < 2)return;

	fd = open(argv[1] , O_RDONLY);
	if(fd <= 0)
	{
		printf("error@open\n");
		return;
	}

	ret = read(fd, buf, 0x100000);
	if(ret <= 0)
	{
		goto theend;
	}

	disasm_arm64_one(buf, ret);

theend:
	close(fd);
}
