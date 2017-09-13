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




//json(data)
int json_create(void*, void*);
int json_delete();
//dts(data)
int dts_create(void*, void*);
int dts_delete();
//class(data)
int class_create(void*, void*);
int class_delete();
//struct(data)
int struct_create(void*, void*);
int struct_delete();
//c(prog)
int c_create(void*, void*);
int c_delete();
//cpp(prog)
int cpp_create(void*, void*);
int cpp_delete();
//go(prog)
int go_create(void*, void*);
int go_delete();
//java(prog)
int java_create(void*, void*);
int java_delete();
//js(prog)
int js_create(void*, void*);
int js_delete();
//perl(prog)
int perl_create(void*, void*);
int perl_delete();
//php(prog)
int php_create(void*, void*);
int php_delete();
//python(prog)
int python_create(void*, void*);
int python_delete();
//ruby(prog)
int ruby_create(void*, void*);
int ruby_delete();
//bash(script)
int bash_create(void*, void*);
int bash_delete();
//makefile(script)
int makefile_create(void*, void*);
int makefile_delete();
//none(test)
int none_create(void*, void*);
int none_delete();
//count(test)
int count_create(void*, void*);
int count_delete();
//include(test)
int include_create(void*, void*);
int include_delete();




//
void* hash_write(void*, int);
void connect(u64, u64, void*, void*);
//
//string.c
u64 suffix_value(char*);
//traverse.c
int traverse_write();
char* traverse_read();




struct worker
{
	//[00,07]
	u64 type;

	//[08,0f]
	u64 name;

	//[10,17]
	int (*start)();
	char pad2[8-sizeof(char*)];

	//[18,1f]
	int (*stop)();
	char pad3[8-sizeof(char*)];

	//[20,27]
	int (*list)();
	char pad4[8-sizeof(char*)];

	//[28,2f]
	int (*choose)();
	char pad5[8-sizeof(char*)];

	//[30,37]
	int (*read)(char*, int, char*, int);
	char pad6[8-sizeof(char*)];

	//[38,3f]
	int (*write)();
	char pad7[8-sizeof(char*)];

	//[40,ff]
	u8 data[0xc0];
};
static struct worker w[20];
static int chosen;
//
static int infile = -1;
static u8 inbuf[0x100000];
static int outfile = -1;
static u8 outbuf[0x100000];




int worker_write(char* p, int len, int type)
{
	int j;
	if(type == 0)	//file
	{
		for(j=0;j<256;j++)
		{
			if(p[j] == 0)
			{
				hash_write(p, j);
				break;
			}
		}
	}
	else if(type == 1)
	{
		hash_write(p, len);
	}
	else if(type == 2)
	{
		hash_write(p, len);
	}
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
		if(w[j].type == 0)break;

		printf("%-8s %-8s %llx,%llx,%llx,%llx,%llx,%llx\n",
			(char*)&w[j].type,
			(char*)&w[j].name,
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
	int j,k=0;
	u64 x = suffix_value(p);

	for(j=0;j<20;j++)
	{
		if(w[j].name == x)
		{
			k = j;
			break;
		}
	}

	if(k > 0)chosen = k;
	return k;
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
		printf("wrong@stat\n");
		return -1;
	}

	size = statbuf.st_size;
	if( (size <= 0) | (size > 0xfffff) )
	{
		printf("wrong@size\n");
		return -2;
	}

//~~~~~~~~~~~~~~~~~~~~~~~2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//open
	infile = open(p , O_RDONLY|O_BINARY);
	if(infile < 0)
	{
		printf("fail@open\n");
		return -3;
	}

	w[chosen].start();

//~~~~~~~~~~~~~~~~~~~~~~~3~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//infomation
	ret = snprintf(outbuf, 256, "#name:       %s\n", p);
	printf("%s", outbuf);

	ret = snprintf(outbuf, 256, "#size:       %d(0x%x)\n", size, size);
	printf("%s", outbuf);

	worker_write(p, 0, 0);
	return 1;
}
int worker_stop()
{
	w[chosen].stop();
	close(infile);

	printf("\n\n");
}
void worker_create()
{
	int t;
	char* j = (char*)w;
	for(t=0;t<0x100*20;t++)j[t] = 0;

	none_create(w, j);	//how many file has been ignored
	j += 0x100;

	count_create(w, j);	//how many bytes and lines in this file
	j += 0x100;

	c_create(w, j);
	j += 0x100;

	cpp_create(w, j);
	j += 0x100;

	dts_create(w, j);
	j += 0x100;

	include_create(w, j);
	j += 0x100;

	struct_create(w, j);
	j += 0x100;

	//worker_list();
}
void worker_delete()
{
}