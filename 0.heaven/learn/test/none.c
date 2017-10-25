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
static void none_write()
{
}
static void none_read(int start,int end)
{
}
static void none_list()
{
}
static void none_choose()
{
}
static void none_stop(int where)
{
}
static void none_start(char* thisfile,int size)
{
	ignorecount=0;
}
void none_delete()
{
}
void none_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x656e6f6e;
	this[2] = (u64)none_start;
	this[3] = (u64)none_stop;
	this[4] = (u64)none_list;
	this[5] = (u64)none_choose;
	this[6] = (u64)none_read;
	this[7] = (u64)none_write;
}
