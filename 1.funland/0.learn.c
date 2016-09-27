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




//none(example)
int none_init(void*);
int none_kill();
//count(example)
int count_init(void*);
int count_kill();
//bash
int bash_init(void*);
int bash_kill();
//c
int c_init(void*);
int c_kill();
//cpp
int cpp_init(void*);
int cpp_kill();
//class
int class_init(void*);
int class_kill();
//dts
int dts_init(void*);
int dts_kill();
//go
int go_init(void*);
int go_kill();
//include
int include_init(void*);
int include_kill();
//makefile
int makefile_init(void*);
int makefile_kill();
//java
int java_init(void*);
int java_kill();
//js
int js_init(void*);
int js_kill();
//json
int json_init(void*);
int json_kill();
//perl
int perl_init(void*);
int perl_kill();
//php
int php_init(void*);
int php_kill();
//python
int python_init(void*);
int python_kill();
//ruby
int ruby_init(void*);
int ruby_kill();
//struct
int struct_init(void*);
int struct_kill();




//string.c
void string_create();
void string_delete();
u64 suffix_value(char*);
//traverse.c
void traverse_create();
void traverse_delete();
int traverse_write();
char* traverse_read();




struct worker
{
	u64 id;

	int (*explain)(char*,int);
	char pad2[8-sizeof(char*)];

	int (*start)();
	char pad0[8-sizeof(char*)];

	int (*stop)();
	char pad1[8-sizeof(char*)];
};
static struct worker w[20];
//
static int chosen;
static struct stat statbuf;
//
static int infile = -1;
static unsigned char inbuf[0x100000];
static int outfile = -1;
static unsigned char outbuf[0x100000];




int worker_write(int count)
{
	//write(outfile, outbuf, count);
}
int worker_read()
{
	int ret = read(infile, inbuf, 0x100000);
	if(ret < 0)
	{
		printf("fail@read\n");
		close(infile);
		return -4;
	}
	inbuf[ret]=0;
	c_explain(inbuf, ret, outbuf, 0x100000);
	//w[chosen].explain(0,ret);
}




int worker_start(char* p)
{
	int ret;
	int size;
	u64 x = suffix_value(p);
	if(x != 0x63)return 0;




//~~~~~~~~~~~~~~~~~~~~~~~1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//stat
	ret=stat(p, &statbuf);
	if(ret == -1)
	{
		//printf("wrong@stat\n");
		return -1;
	}

	size = statbuf.st_size;
	if( (size <= 0) | (size > 0xfffff) )
	{
		//printf("wrong@size\n");
		return -2;
	}

	//infomation
	ret = snprintf(outbuf, 256, "#name:       %s\n", p);
	printf("%s", outbuf);

	ret = snprintf(outbuf, 256, "#size:       %d(0x%x)\n", size, size);
	printf("%s", outbuf);




//~~~~~~~~~~~~~~~~~~~~~~~2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//open
	infile = open(p , O_RDONLY|O_BINARY);
	if(infile < 0)
	{
		printf("fail@open\n");
		return -3;
	}
	c_start();
	//w[chosen].start();

	return 1;
}
int worker_stop()
{
	c_stop();
	close(infile);
}
void worker_create()
{
	char* j = (char*)w;

	//none_init(j);	//how many file has been ignored
	j += 0x20;

	count_init(j);	//how many bytes and lines in this file
	j += 0x20;

	c_init(j);
	j += 0x20;

	cpp_init(j);
	j += 0x20;

	dts_init(j);
	j += 0x20;

	include_init(j);
	j += 0x20;

	struct_init(j);
	j += 0x20;

	outfile = open("code.seed", O_WRONLY|O_BINARY);
}
void worker_delete()
{
	if(outfile > 0)close(outfile);
}




int learn(int argc,char** argv)
{
	int j,k;
	char* p;

	//before
	string_create();
	traverse_create();
	worker_create();

	//example:	./a.out 1.c *.c /src/*.c */*.c
	for(j=1;j<argc;j++)
	{
		//do it
		traverse_write(argv[j]);
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
	}//for

	//after
	string_delete();
	traverse_delete();
	worker_delete();
}
