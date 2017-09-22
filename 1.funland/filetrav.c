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




struct stack
{
	DIR* folder;
	unsigned char name[512 - sizeof(char*)];
};
static struct stack stack[16];
static struct stat statbuf;
static char path[512];
static int rsp=0;




//
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
static u8 travbuf[0x100000];
static int travfd;
static int travlen;




char* traverse_read()
{
	struct dirent* ent;

	while(1)
	{
		//empty name
		if(stack[rsp].name[0] == 0)
		{
			//empty stack
			if(rsp == 0)return 0;

			//pop
			else
			{
				rsp--;
				continue;
			}
		}

		//have name, not opened
		if(stack[rsp].folder == 0)
		{
			//try to open dir
			stack[rsp].folder = opendir(stack[rsp].name);

			//opened successfully
			if(stack[rsp].folder != 0)continue;

			//can not open, it is leaf !!!
			else
			{
				strncpy(path,stack[rsp].name,500);
				stack[rsp].name[0]=0;
				return path;
			}
		}

		//folder opened, take one
		ent=readdir(stack[rsp].folder);

		//failed to get
		if(ent == 0)
		{
			closedir(stack[rsp].folder);
			stack[rsp].name[0]=0;

			if(rsp > 0)rsp--;
			continue;
		}

#ifdef DT_LNK
		//ignore nondir
		if(ent->d_type == DT_LNK)
		{
			continue;
		}
#endif

		//ignore . .. .*
		if(ent->d_name[0] == '.')
		{
			continue;
		}

		//push
		if(	(rsp==0) &&
			(stack[0].name[0]=='/') &&
			(stack[0].name[1]==0) )
		{
			snprintf(
				stack[1].name,
				500,
				"/%s",
				ent->d_name
			);
		}
		else
		{
			snprintf(
				stack[rsp+1].name,
				500,
				"%s/%s",
				stack[rsp].name,
				ent->d_name
			);
		}
		stack[rsp+1].folder = 0;
		rsp++;
	}
}
int traverse_write()
{
}
void traverse_start(char* p)
{
	int j;

	//clear everything
	rsp=0;
	memset(stack,0,sizeof(struct stack));
	strncpy(stack[0].name , p , 256);

	//convert "/some/dir/" to "/some/dir"
	j=strlen(p);
	if(j > 1)
	{
		if(p[j-1]=='/')stack[0].name[j-1]=0;
	}
}
void traverse_stop()
{
}




void* filetrav_read()
{
}
void* filetrav_write(void* name, u64 size)
{
	struct fileindex* addr;
	printf("%d(0x%x)	%s\n", size, size, name);

	addr = (void*)travbuf + travlen;
	addr->self = travlen;
	addr->what = size;

	travlen += 0x20;
	return addr;
}
void filetrav_start()
{
	travlen = 0x20;
}
void filetrav_stop()
{
}
void filetrav_create()
{
	int j;
	char* buf;

	buf = (void*)travbuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//trav
	travfd = open(
		".42/file.index",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	travlen = read(travfd, travbuf, 0x100000);
	printf("filetrav:	%x\n", travlen);
}
void filetrav_delete()
{
	lseek(travfd, 0, SEEK_SET);
	write(travfd, travbuf, travlen);
	close(travfd);
}
