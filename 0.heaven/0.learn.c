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
	//mingw64 compatiable
	#define O_BINARY 0x0
#endif




void filedata_start();
void filetrav_start();
void funcdata_start();
void funcindx_start();
void stringdata_start();
void stringhash_start();
void connect_start();
//
int traverse_start(char* p);
int traverse_stop();
char* traverse_read();
//
int worker_start(char* p);
int worker_stop();
int worker_read();




int learn(int argc,char** argv)
{
	int j,k;
	char* p;
	filedata_start();
	filetrav_start();
	funcdata_start();
	funcindx_start();
	stringdata_start();
	stringhash_start();
	connect_start();

	//example:	./a.out 1.c *.c /src/*.c */*.c
	for(j=1;j<argc;j++)
	{
		//do it
		traverse_start(argv[j]);
		while(1)
		{
			//get one(traverse.c)
			p = traverse_read();
			if(p == 0)break;
			//printf("file=%s\n",p);

			//choose worker
			k = worker_start(p);
			if(k <= 0)continue;
			//printf("worker=%d\n",k);

			//do it
			k = worker_read();
			//worker_write(k);

			//
			worker_stop();
		}
		traverse_stop();
	}//for
	return 0;
}
