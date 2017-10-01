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
#define u64 unsigned long long
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))




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
void* filetrav_write(void*, u64);
void* funcindx_write(u64);
void* stringhash_write(void*, int);
void connect_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype);
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
static char inbuf[0x100000];
static int outfile = -1;
static char outbuf[0x100000];




static void* fileinfo;
static void* funcinfo;
int worker_write(char* buf, int len, int type, int haha)
{
	void* thishash;
	if(type == 0)		//file
	{
		//string hash
		thishash = stringhash_write(buf, len);
		if(thishash == 0)
		{
			printf("error@1111\n");
			return 0;
		}

		//(name, size, attr, ...)
		fileinfo = filetrav_write(buf, haha);
		if(fileinfo == 0)
		{
			printf("error@2222\n");
			return 0;
		}

		//hash <- file (lchip, lfoot, ltype, rchip, rfoot, rtype)
		connect_write(
			thishash, 0, hex32('h','a','s','h'),
			fileinfo, 0, hex32('f','i','l','e')
		);
	}
	else if(type == 1)		//func
	{
		thishash = stringhash_write(buf, len);
		if(thishash == 0)
		{
			printf("error@3333\n");
			return 0;
		}

		//func item
		funcinfo = funcindx_write(haha);
		if(funcinfo == 0)
		{
			printf("error@4444\n");
			return 0;
		}

		//file <- func
		connect_write(
			fileinfo, haha, hex32('f','i','l','e'),
			funcinfo, 0, hex32('f','u','n','c')
		);

		//hash <- func
		connect_write(
			thishash, 0, hex32('h','a','s','h'),
			funcinfo, 0, hex32('f','u','n','c')
		);
	}
	else if(type == 2)
	{
		thishash = stringhash_write(buf, len);
		if(thishash == 0)
		{
			printf("error@5555\n");
			return 0;
		}

		//func <- hash
		connect_write(
			funcinfo, haha, hex32('f','u','n','c'),
			thishash, 0, hex32('h','a','s','h')
		);
	}
	return 1;
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
	return w[chosen].read(inbuf, ret, outbuf, 0x100000);
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
	return 0;
}
int worker_choose(char* p)
{
	int j,k=-1;
	u64 x = suffix_value(p);
	if(x == 0)return -1;

	for(j=0;j<20;j++)
	{
		//printf("%x\n",w[j].name);
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

	//check name
	ret = worker_choose(p);
	if(ret < 0)return -1;

//~~~~~~~~~~~~~~~~~~~~~~~1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//stat
	ret=stat(p, &statbuf);
	if(ret == -1)
	{
		printf("wrong@stat:%s\n", p);
		return -2;
	}

	size = statbuf.st_size;
	if(size <= 0)return -3;
	if(size > 0xfffff)
	{
		printf("wrong@size:%s\n", p);
		return -4;
	}

//~~~~~~~~~~~~~~~~~~~~~~~2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//open
	infile = open(p , O_RDONLY|O_BINARY);
	if(infile < 0)
	{
		printf("fail@open:%s\n", p);
		return -5;
	}

	w[chosen].start();

//~~~~~~~~~~~~~~~~~~~~~~~3~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//infomation
	for(ret=0;ret<256;ret++)
	{
		if(p[ret] == 0)
		{
			worker_write(p, ret, 0, size);
			break;
		}
	}
	//substr_write(p);
	return 1;
}
int worker_stop()
{
	w[chosen].stop();
	close(infile);
	return 0;
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

	struct_create(w, j);
	j += 0x100;

	//worker_list();
}
void worker_delete()
{
}
