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
int none_create(void*, void*);
int none_delete();
//count(example)
int count_create(void*, void*);
int count_delete();
//bash
int bash_create(void*, void*);
int bash_delete();
//c
int c_create(void*, void*);
int c_delete();
//cpp
int cpp_create(void*, void*);
int cpp_delete();
//class
int class_create(void*, void*);
int class_delete();
//dts
int dts_create(void*, void*);
int dts_delete();
//go
int go_create(void*, void*);
int go_delete();
//include
int include_create(void*, void*);
int include_delete();
//makefile
int makefile_create(void*, void*);
int makefile_delete();
//java
int java_create(void*, void*);
int java_delete();
//js
int js_create(void*, void*);
int js_delete();
//json
int json_create(void*, void*);
int json_delete();
//perl
int perl_create(void*, void*);
int perl_delete();
//php
int php_create(void*, void*);
int php_delete();
//python
int python_create(void*, void*);
int python_delete();
//ruby
int ruby_create(void*, void*);
int ruby_delete();
//struct
int struct_create(void*, void*);
int struct_delete();




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
	u64 type;
	u64 id;
	u64 at10;
	u64 at18;
	u64 at20;
	u64 at28;
	u64 at30;
	u64 at38;

	int (*create)();
	char pad0[8-sizeof(char*)];

	int (*delete)();
	char pad1[8-sizeof(char*)];

	int (*start)();
	char pad2[8-sizeof(char*)];

	int (*stop)();
	char pad3[8-sizeof(char*)];

	int (*list)();
	char pad4[8-sizeof(char*)];

	int (*choose)();
	char pad5[8-sizeof(char*)];

	int (*read)(char*, int, char*, int);
	char pad6[8-sizeof(char*)];

	int (*write)();
	char pad7[8-sizeof(char*)];
};
static struct worker w[20];
static int chosen;
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
	w[chosen].read(inbuf, ret, outbuf, 0x100000);
}
int worker_list()
{
	int j;
	for(j=0;j<20;j++)
	{
		printf("%llx,%llx\n", w[j].type, w[j].id);
		printf("%llx,%llx,%llx,%llx,%llx,%llx\n",
			(u64)w[j].start,
			(u64)w[j].stop,
			(u64)w[j].list,
			(u64)w[j].choose,
			(u64)w[j].read,
			(u64)w[j].write
		);
	}
}
int worker_choose(char* p)
{
	u64 x = suffix_value(p);
	if(x != 0x63)return 0;

	chosen = 1;
	return chosen;
}
int worker_start(char* p)
{
	struct stat statbuf;
	int size;
	int ret;
	ret = worker_choose(p);
	if(ret < 0)return 0;

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

	w[chosen].start();
	return 1;
}
int worker_stop()
{
	w[chosen].stop();
	close(infile);
}
void worker_create()
{
	char* j = (char*)w;

	none_create(w, j);	//how many file has been ignored
	j += 0x80;

	count_create(w, j);	//how many bytes and lines in this file
	j += 0x80;

	c_create(w, j);
	j += 0x80;

	cpp_create(w, j);
	j += 0x80;

	dts_create(w, j);
	j += 0x80;

	include_create(w, j);
	j += 0x80;

	struct_create(w, j);
	j += 0x80;

	worker_list();
	outfile = open(
		"code.seed",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
                S_IRWXU|S_IRWXG|S_IRWXO
	);
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
