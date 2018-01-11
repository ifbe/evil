#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u64 unsigned long long
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
void arm2ac(int argc, char** argv);
void att2ac(int argc, char** argv);
void intel2ac(int argc, char** argv);
void ac2arm(int argc, char** argv);
void ac2att(int argc, char** argv);
void ac2intel(int argc, char** argv);




int conv(int argc, char** argv)
{
	if(argc <= 2)return 0;

	if(strcmp(argv[1], "arm2ac") == 0)
	{
		arm2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "att2ac") == 0)
	{
		att2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "intel2ac") == 0)
	{
		intel2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2arm") == 0)
	{
		ac2arm(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2att") == 0)
	{
		ac2att(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2intel") == 0)
	{
		ac2intel(argc-1, argv+1);
	}
}
