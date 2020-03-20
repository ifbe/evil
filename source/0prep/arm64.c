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




int disasm_3btoint(u8* buf)
{
	u32 u;
	u = *(u32*)buf;
	u &= 0xffffff;
	if(u&0x800000)return u-0x01000000;
	return u;
}
void disasm_arm64_one(u8* buf, int len)
{
int j,cnt=0;
u32 code;
for(j=0;j<len;j+=4){
	code = *(u32*)(buf+j);
	if(0xd503201f == code){
		printf("%x:	nop\n", j);
	}
	if(0xd65f03c0 == code){
		printf("%x:	ret\n", j);
	}
//-----------------jmp---------------------
	else if(0x17 == buf[j+3]){
		printf("%x:	b %x\n", j, j+(disasm_3btoint(buf+j)*4));
	}
	else if(0x94 == buf[j+3]){
		printf("%x:	bl %x\n", j, j+(disasm_3btoint(buf+j)*4));
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
//-----------------orr-------------------
	else if(0xaa000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	x%d = x%d | x%d\n", j, r0, r1, r2);
	}
	else if(0x2a000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 r2 = (code>>16)&0x1f;
		printf("%x:	w%d = w%d | w%d\n", j, r0, r1, r2);
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
	}
}//for
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
