#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef O_BINARY
	//mingw64 compatiable
	#define O_BINARY 0x0
#endif




//
static int dest=-1;

//
static unsigned char* datahome;		//4k+4k
static unsigned char strbuf[256];

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




int count_explain(int start,int end)
{
	int i=0;
	unsigned char ch=0;
	printf(
		"@%x@%d -> %d,%d,%d,%d\n",
		countbyte+start,
		countline+1,
		infunc,
		inmarco,
		innote,
		instr
	);

	//不用i<end防止交界麻烦,给足了整整0x800个机会自己决定滚不滚
	for(i=start;i<0x180000;i++)
	{
		//拿一个
		ch=datahome[i];

		//软退
		if(i>end)
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

	countbyte += 0x100000;
	return i-end;	//可能多分析了几十几百个字节
}
int count_start(char* thisfile,int size)
{
	int ret;

	//infomation
	ret=snprintf(datahome,256,"#name:	%s\n",thisfile);
	printf("%s",datahome);
	write(dest,datahome,ret);

	ret=snprintf(datahome,256,"#size:	%d(0x%x)\n",size,size);
	printf("%s",datahome);
	write(dest,datahome,ret);

	//init
	countbyte=countline=0;
	infunc = inmarco = innote = instr = 0;
}
int count_stop(int where)
{
	printf("@%x@%d -> %d,%d,%d,%d\n",
		where,
		countline,
		infunc,
		inmarco,
		innote,
		instr
	);
	printf("\n\n\n\n");
	write(dest,"\n\n\n\n",4);
}
int count_init(char* file,char* memory)
{
/*
	dest=open(
		file,
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
*/
	datahome=memory;
}
int count_kill()
{
	//close(dest);
}
