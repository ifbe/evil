#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#ifndef O_BINARY
	//mingw64 compatiable
	#define O_BINARY 0x0
#endif




//
static int dest=-1;

//count
static int countbyte=0;		//统计字节数
static int countline=0;		//统计行数

//status
static int infunc=0;
	//0:	不在函数里
	//1:	在函数内
	//?:	被包在第几个括号里
static int inmarco=0;
	//0:	不在宏里
	//1:	在普通宏内
	//'d':	#define
	//'e':	#else
static int innote=0;
	//0:	不在注释里
	//1:	//
	//9:	/**/
static int instr=0;
	//0:	不在字符串里
	//1:	在字符串内




static int count_read(char* datahome, int len)
{
	int i=0;
	unsigned char ch=0;
	printf(
		"@%x@%d -> %d,%d,%d,%d\n",
		countbyte,
		countline+1,
		infunc,
		inmarco,
		innote,
		instr
	);

	//不用i<end防止交界麻烦,给足了整整0x800个机会自己决定滚不滚
	for(i=0;i<0x180000;i++)
	{
		//拿一个
		ch=datahome[i];

		//软退
		if(i>len)
		{
			if(ch==' ')break;
			else if(ch==0x9)break;
			else if(ch==0xa)break;
			else if(ch==0xd)break;
		}

		//强退(代码里绝不会有真正的0，都是ascii的0x30)
		if(ch==0)
		{
			//printf("@%x\n",i);
			break;
		}

		//0xa:		linux的换行符
		else if(ch==0xa)
		{
			//linux的换行符
			countline++;

			//define宏，换行清零
			if(inmarco=='d')inmarco=0;

			//单行注释，换行清零
			if(innote==1)innote=0;

			//字符串，换行清零
			if(instr==1)instr=0;
		}

		//0xd:		mac或是windows的换行符
		else if(ch==0xd)
		{
			//如果是windows的换行符，吃掉后面的0xa
			countline++;
			if(datahome[i+1]==0xa)i++;

			//define宏，换行清零
			if(inmarco=='d')inmarco=0;

			//单行注释，换行清零
			if(innote==1)innote=0;

			//字符串，换行清零
			if(instr==1)instr=0;
		}

		//.....................
		else if(ch=='\\')
		{
			//linux的换行
			if(datahome[i+1]==0xa)
			{
				countline++;
			}

			//mac或者windows的换行
			else if(datahome[i+1]==0xd)
			{
				//windows的换行多吃掉一个
				if(datahome[i+2]==0xa)i++;
				countline++;
			}

			//吃一个，然后换行
			i++;
			continue;
		}
	}//for

	countbyte += len;
	return i-len;	//可能多分析了几十几百个字节
}
static void count_write()
{
}
static void count_stop()
{
	printf("@%x@%d -> %d,%d,%d,%d\n",
		countbyte,
		countline,
		infunc,
		inmarco,
		innote,
		instr
	);
	printf("\n\n\n\n");
}
static void count_start()
{
	countbyte=countline=0;
	infunc = inmarco = innote = instr = 0;
}




void count_delete()
{
}
void count_create(u64* file, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = hex32('c','n','t',0);
	this[4] = (u64)count_start;
	this[5] = (u64)count_stop;
	this[6] = (u64)count_read;
	this[7] = (u64)count_write;
}
