#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
int decstr2data(void*, void*);




struct pointdata
{
	float x;
	float y;
	float z;
	float w;
};
#define maxlen 0x100000
static u8 pointindexbuf[maxlen];
static int pointindexfd;
static int pointindexlen;