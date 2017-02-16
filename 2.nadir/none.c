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




static int ignorecount;
static int none_write()
{
}
static int none_read(int start,int end)
{
	return 0;
}
static int none_list()
{
}
static int none_choose()
{
}
static int none_stop(int where)
{
	return 0;
}
static int none_start(char* thisfile,int size)
{
	ignorecount=0;
	return 0;
}
int none_delete()
{
	return 0;
}
int none_create(u64* that, u64* this)
{
        this[0] = 0x6573726170;
        this[1] = 0x656e6f6e;

        this[8] = (u64)none_create;
        this[9] = (u64)none_delete;
        this[10] = (u64)none_start;
        this[11] = (u64)none_stop;
        this[12] = (u64)none_list;
        this[13] = (u64)none_choose;
        this[14] = (u64)none_read;
        this[15] = (u64)none_write;
	return 0;
}
