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




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
typedef struct hash* hash;
//
static int wirefd;
static int wirelen;
static u8 wirebuf[0x100000];




//'dir'		xxxxxxx,	dirname,	filename
//'file',	linenum,	filename,	funcname
//'call',	linenum,	funcname,	callname
//'struct',	linenum,	structname,	elementname
void connect(hash first, hash last, u64 relation, u64 detail)
{
}




void connect_create()
{
	int j;
	char* buf;

	buf = (void*)wirebuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	wirefd = open(
		".42/42.wire",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	wirelen = 0;
}
void connect_delete()
{
	write(wirefd, wirebuf, wirelen);
	close(wirefd);
}