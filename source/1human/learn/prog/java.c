#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#ifndef O_BINARY
	//mingw64 compatiable
	#define O_BINARY 0x0
#endif




//count
static int countbyte=0;
static int countline=0;




static int java_read(u8* buf, int len)
{
	unsigned char ch=0;
	countbyte = 0;

	for(;countbyte<0x100000;countbyte++)
	{
		ch=buf[countbyte];

		if(countbyte>len){
            break;
        }
		if(ch==0){
			break;
		}
		else if((ch==0xa) | (ch == 0xd) ){
			countline++;
			if((ch == 0xd) && (buf[countbyte+1] == 0xa))countbyte++;
        }
    }
    return 0;
}
static void java_write()
{
}
static void java_list()
{
}
static void java_choose()
{
}
static void java_stop()
{
/*
	printf("@%x@%d -> %d,%d,%d,%d\n\n\n\n\n",
		countbyte,
		countline,
		infunc,
		inmarco,
		innote,
		instr
	);
*/
	//write(outfile,"\n\n\n\n",4);
}
static void java_start()
{
	countbyte = countline = 0;
}
void java_delete()
{
}
void java_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x63;
	this[2] = (u64)java_start;
	this[3] = (u64)java_stop;
	this[4] = (u64)java_list;
	this[5] = (u64)java_choose;
	this[6] = (u64)java_read;
	this[7] = (u64)java_write;
}