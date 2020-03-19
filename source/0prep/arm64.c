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
int j;
u32 code;
for(j=0;j<len;j+=4){
	code = *(u32*)(buf+j);
	if(0xd503201f == code){
		printf("%x:	nop\n", j);
	}
	if(0xd65f03c0 == code){
		printf("%x:	ret\n", j);
	}
	else if(0x17 == buf[j+3]){
		printf("%x:	b %x\n", j, j+(disasm_3btoint(buf+j)*4));
	}
	else if(0x94 == buf[j+3]){
		printf("%x:	bl %x\n", j, j+(disasm_3btoint(buf+j)*4));
	}
/*	else if(0x92 == buf[j+3]){
		printf("%x:	and x%d,x%d,#?\n", j, code&0x1f, (code>>5)&0x1f);
	}*/
	else if(0xeb00001f == (code&0xebc0001f)){
		printf("%x:	x%d ? x%d\n", j, (code>>5)&0x1f, (code>>16)&0x1f);
	}
	else if(0x6b00001f == (code&0x6bc0001f)){
		printf("%x:	w%d ? w%d\n", j, (code>>5)&0x1f, (code>>16)&0x1f);
	}
	else if(0xaa0003e0 == (code&0xaac003e0)){
		printf("%x:	x%d = x%d\n", j, code&0x1f, (code>>16)&0x1f);
	}
	else if(0x2a0003e0 == (code&0x2ac003e0)){
		printf("%x:	w%d = w%d\n", j, code&0x1f, (code>>16)&0x1f);
	}
	else{
		printf("%x:	%02x %02x %02x %02x\n", j,
		buf[j+0],buf[j+1],buf[j+2],buf[j+3]);
	}
}//for
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
