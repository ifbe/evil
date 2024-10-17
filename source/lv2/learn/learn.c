#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "evil.h"
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif




//data
int json_create(void*, void*);
int json_delete();
int dts_create(void*, void*);
int dts_delete();
int class_create(void*, void*);
int class_delete();
int struct_create(void*, void*);
int struct_delete();
int utf8_create(void*, void*);
int utf8_delete();
//prog
int asm_create(void*, void*);
int asm_delete();
int c_create(void*, void*);
int c_delete();
int include_create(void*, void*);
int include_delete();
int cpp_create(void*, void*);
int cpp_delete();
int go_create(void*, void*);
int go_delete();
int java_create(void*, void*);
int java_delete();
int js_create(void*, void*);
int js_delete();
int perl_create(void*, void*);
int perl_delete();
int php_create(void*, void*);
int php_delete();
int python_create(void*, void*);
int python_delete();
int ruby_create(void*, void*);
int ruby_delete();
//script
int bash_create(void*, void*);
int bash_delete();
int makefile_create(void*, void*);
int makefile_delete();
//test
int none_create(void*, void*);
int none_delete();
int count_create(void*, void*);
int count_delete();
//
int cir_create(void*, void*);
int cir_delete();
int three_create(void*, void*);
int three_delete();
//
int map_create(void*, void*);
int map_delete();
//
int traverse_start(char* p);
int traverse_stop();
char* traverse_read(int*, int*);
u64 suffix_value(char*);
//
void* filemd5_write(void*, u64);
void* funcindex_write(u64);
void* strhash_write(void*, int);
void* strhash_read(u64);




struct worker
{
	//[00,07]
	u64 type;

	//[08,0f]
	u64 name;

	//[10,17]
	u64 pad2;

	//[18,1f]
	u64 pad3;

	//[20,27]
	int (*start)();
	char pad4[8-sizeof(char*)];

	//[28,2f]
	int (*stop)();
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
static struct worker wk[20];
static int chosen;
//
static int infile = -1;
static char inbuf[0x100000];
static int outfile = -1;
static char outbuf[0x100000];
//
static u64 strhash;
static void* fileobj;
static void* funcobj;




void string_infile_global(u8* buf, int len, u64 file, int line)
{
	void* thishash = strhash_write(buf, len);
	if(0 == thishash)
	{
		printf("error@9999\n");
		return;
	}

	//file <- func
	relationcreate(fileobj, line, _file_, _globalstr_, thishash, 0, _hash_, _owner_);
}
void string_infile_infunc(u8* buf, int len, u64 file, int line)
{
	void* thishash = strhash_write(buf, len);
	if(0 == thishash)
	{
		printf("error@5555\n");
		return;
	}

	//func <- hash
	relationcreate(funcobj, line, _func_, _localstr_, thishash, 0, _hash_, _owner_);
}
void funcname_called_by_func(u8* buf, int len, u64 file, int line)
{
	void* thishash = strhash_write(buf, len);
	if(0 == thishash)
	{
		printf("error@5555\n");
		return;
	}

	//func <- hash
	relationcreate(funcobj, line, _func_, _subfunc_, thishash, 0, _hash_, _caller_);
}
void connect_func_to_file(u8* buf, int len, u64 file, int line)
{
	void* thishash = strhash_write(buf, len);
	if(0 == thishash)
	{
		printf("error@3333\n");
		return;
	}

	//func item
	funcobj = funcindex_write(line);
	if(0 == funcobj)
	{
		printf("error@4444\n");
		return;
	}

	//file <- func
	relationcreate(fileobj, line, _file_, _func_, funcobj, 0, _func_, _file_);

	//hash <- func
	relationcreate(thishash, 0, _hash_, _func_, funcobj, 0, _func_, _name_);
}
void connect_file_to_hash(char* buf, int len)
{
	//(name, size, attr, ...)
	fileobj = filemd5_write(buf, len);
	if(0 == fileobj)
	{
		printf("error@2222\n");
		return;
	}

	//string hash
	void* thishash = strhash_read(strhash);
	if(0 == thishash)
	{
		printf("error@0000\n");
		return;
	}

	//hash <- file (lchip, lfoot, ltype, rchip, rfoot, rtype)
	relationcreate(thishash, 0, _hash_, _file_, fileobj, 0, _file_, _name_);
}
void connect_path_to_name(char* path,int plen, char* name,int nlen)
{
	void* pathhash = strhash_write(path, plen);
	if(0 == pathhash)
	{
		printf("error@0000\n");
		return;
	}

	void* namehash = strhash_write(name, nlen);
	if(0 == namehash)
	{
		printf("error@0000\n");
		return;
	}

	relationcreate(pathhash, 0, _hash_, _name_, namehash, 0, _hash_, _path_);
}




int worker_read()
{
	//read
	int ret = read(infile, inbuf, 0x100000);
	if(ret < 0)
	{
		printf("fail@read\n");
		close(infile);
		return -4;
	}
	inbuf[ret] = 0;

	//hash?
	//worker_write(inbuf, ret, 0, 0);
	connect_file_to_hash(inbuf, ret);

	//start
	wk[chosen].start();

	//explain
	ret = wk[chosen].read(inbuf, ret, outbuf, 0x100000);

	//stop
	wk[chosen].stop();
	return ret;
}
int worker_list()
{
	int j;
	for(j=0;j<20;j++)
	{
		if(wk[j].type == 0)break;

		printf("%-8s %-8s %p,%p,%p,%p\n",
			(char*)&wk[j].type,
			(char*)&wk[j].name,
			wk[j].start,
			wk[j].stop,
			wk[j].read,
			wk[j].write
		);
	}
	return 0;
}
int worker_choose(char* name)
{
	int j;
	chosen = -1;

	u64 x = suffix_value(name);
	if(x == 0)return -1;

	for(j=0;j<20;j++)
	{
		if(wk[j].name == x)
		{
			chosen = j;
			break;
		}
	}
	return chosen;
}
int worker_start(char* name)
{
	struct stat statbuf;
	int size;
	int ret;
	if(name == 0)return -1;
	if(name[0] == '.')
	{
		if((name[1] != '/')&&(name[1] != '.'))return -1;
	}

	//worker_write(name, strlen(name), 4, 0);
	void* thisobj = strhash_write(name, strlen(name));
	if(0 == thisobj)
	{
		printf("error@1111\n");
		return -1;
	}
	strhash = *(u64*)thisobj;

	//get suffix
	chosen = worker_choose(name);
	if(chosen < 0)return -1;

	//stat
	ret=stat(name, &statbuf);
	if(ret == -1)
	{
		printf("wrong@stat:%s\n", name);
		return -2;
	}

	size = statbuf.st_size;
	if(size <= 0)return -3;
	if(size > 0xfffff)
	{
		printf("wrong@size:%s\n", name);
		return -4;
	}

	//open
	infile = open(name , O_RDONLY|O_BINARY);
	if(infile < 0)
	{
		printf("fail@open:%s\n", name);
		return -5;
	}

	//
	printf("%d	%x	%s\n", size, size, name);
	return 1;
}
int worker_stop()
{
	//close
	close(infile);
	return 0;
}
void worker_create()
{
	int t;
	char* j = (char*)wk;
	for(t=0;t<0x100*20;t++)j[t] = 0;

	//none_create(wk, j);	//how many file has been ignored
	//j += 0x100;

	//count_create(wk, j);	//how many bytes and lines in this file
	//j += 0x100;

	asm_create(wk, j);
	j += 0x100;

	c_create(wk, j);
	j += 0x100;

	//cpp_create(wk, j);
	//j += 0x100;

	java_create(wk, j);
	j += 0x100;

	cir_create(wk, j);
	j += 0x100;

	//dts_create(wk, j);
	//j += 0x100;

	//struct_create(wk, j);
	//j += 0x100;

	three_create(wk, j);
	j += 0x100;

	utf8_create(wk,j);
	j += 0x100;

	map_create(wk,j);
	j += 0x100;
	//worker_list();
}
void worker_delete()
{
}




int learn_one(char* p, int offs, int type)
{
	int k;
	if(offs){
	if('/' == p[offs]){
		//printf("path=%s\n", p);
		//printf("name=%s\n", p+offs+1);
		k = strlen(p+offs+1);
		connect_path_to_name(p, offs+1+k, p+offs+1, k);
	}
	}

	//check
	k = worker_start(p);
	if(k <= 0)return 0;;
	//printf("worker=%d\n",k);

	//parse
	k = worker_read();
	//worker_write(k);

	//close
	worker_stop();
	return 0;
}
int learn(int argc,char** argv)
{
	int j;
	int offs,type;
	char* p;
	readthemall(0);
	worker_create();
	worker_list();

	//example:	./a.out 1.c *.c /src/*.c */*.c
	for(j=1;j<argc;j++)
	{
		//do it
		traverse_start(argv[j]);
		while(1)
		{
			//get one(traverse.c)
			p = traverse_read(&offs, &type);
			if(p == 0)break;

			learn_one(p, offs, type);
		}
		traverse_stop();
	}//for

	worker_delete();
	writethemall(0);
	return 0;
}



