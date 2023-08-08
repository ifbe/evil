#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<errno.h>
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
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#define _dir_ hex32('d','i','r', 0)
#define _file_ hex32('f','i','l','e')




struct stack
{
	DIR* folder;
	int namelen;
};
//
static struct stack stack[16];
static int rsp = 0;
//
static char path[0x1000];
static int len = 0;




char* traverse_read(int* offs, int* type)
{
	int j;
	struct dirent* ent;

	while(1){
		//no name, go back
		if(stack[rsp].namelen == 0){
			//empty name
			if(rsp == 0)return 0;

			//pop
			else{
				rsp--;
				continue;
			}
		}

		//have name, not opened
		if(stack[rsp].folder == 0){
			//try to open dir
			stack[rsp].folder = opendir(path);

			//open fail, is leaf
			if(0 == stack[rsp].folder){
				*type = _file_;
				stack[rsp].namelen = 0;
			}
			else{
				*type = _dir_;
			}

			if(0 == rsp){
				*offs = 0;
			}
			else{
				*offs = stack[rsp-1].namelen;
			}
/*
			if((rsp != 0)&&(stack[0].namelen == 1)&&(path[0] == '.')){
				return path+2;
			}
			else{
				*offs = stack[rsp].namelen;
				return path;
			}
*/
			return path;
		}

		//folder opened, take one
		ent = readdir(stack[rsp].folder);

		//failed to get
		if(ent == 0){
			closedir(stack[rsp].folder);
			stack[rsp].namelen = 0;

			if(rsp > 0)rsp--;
			continue;
		}

#ifdef DT_LNK
		//ignore nondir
		if(ent->d_type == DT_LNK){
			continue;
		}
#endif

		//ignore . .. .*
		if(ent->d_name[0] == '.'){
			continue;
		}

		//push
		if((rsp == 0)&&(len == 1)&&(path[0] == '/'))j = 0;
		else j = stack[rsp].namelen;

		rsp++;
		stack[rsp].folder = 0;
		stack[rsp].namelen = j + snprintf(
			path + j, 4096 - j,
			"/%s", ent->d_name
		);
//printf("3:%s\n", path);
	}
}
void traverse_write()
{
}
void traverse_start(char* name)
{
	//clear everything
	int j;
	rsp = 0;
	len = 0;
	memset(stack, 0, sizeof(struct stack));

	//copy name
	len = snprintf(path, 0x1000, "%s", name);
	if(len <= 0)return;

	//special:	"folder/" -> "folder"
	if((len > 1) && (path[len-1] =='/'))
	{
		len--;
		path[len] = 0;
	}

	stack[0].namelen = len;
}
void traverse_stop()
{
}
