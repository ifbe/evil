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
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif




struct wire
{
	u32 pintype;
	u32 detail;
	u32 samepinlastchip;
	u32 sameinnextchip;

	u32 chipinfo;
	u32 footinfo;
	u32 samechiplastpin;
	u32 samechipnextpin;
};
static u8 wirebuf[0x100000];
static int wirefd;
static int wirelen;




//'dir'		xxxxxxx,	dirname,	filename
//'file',	linenum,	filename,	funcname
//'call',	linenum,	funcname,	callname
//'struct',	linenum,	structname,	elementname
void connect_write(u64 upper, u64 below, u64 relation, u64 detail)
{
	printf("%llx, %llx\n", upper, below);
}
void connect_read()
{
}
void connect_list()
{
}
void connect_choose()
{
}
void connect_start()
{
	lseek(wirefd, 0, SEEK_SET);
	wirelen = 0;
}
void connect_stop()
{
}
void connect_create()
{
	int j;
	char* buf;

	buf = (void*)wirebuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//wire
	wirefd = open(
		".42/42.wire",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	wirelen = read(wirefd, wirebuf, 0x100000);
	printf("wire:	%x\n", wirelen);

	connect_start();
}
void connect_delete()
{
	write(wirefd, wirebuf, wirelen);
	close(wirefd);
}