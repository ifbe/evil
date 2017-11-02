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
int worker_write(char* buf, int len, int type, int haha);




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




char* traverse_read()
{
	int j;
	struct dirent* ent;

	while(1)
	{
		//no name, go back
		if(stack[rsp].namelen == 0)
		{
			//empty name
			if(rsp == 0)return 0;

			//pop
			else
			{
				rsp--;
				continue;
			}
		}
//printf("1:%s\n", path);
		//have name, not opened
		if(stack[rsp].folder == 0)
		{
			//try to open dir
			stack[rsp].folder = opendir(path);

			//opened successfully
			if(stack[rsp].folder != 0)continue;

			//can not open, it is leaf !!!
			else
			{
				stack[rsp].namelen = 0;
//printf("%s\n",path);
				if((rsp != 0)&&(len == 1)&&(path[0] == '.'))
				{
					return path+2;
				}
				else return path;
			}
		}
//printf("2:%s\n", path);

		//folder opened, take one
		ent = readdir(stack[rsp].folder);

		//failed to get
		if(ent == 0)
		{
			closedir(stack[rsp].folder);
			stack[rsp].namelen = 0;

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