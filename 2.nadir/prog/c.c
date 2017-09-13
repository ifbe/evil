#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#ifndef O_BINARY
	//mingw64 compatiable
	#define O_BINARY 0x0
#endif




//count
static int countbyte=0;		//统计字节数
static int countline=0;		//统计行数

//函数名字
static unsigned char* prophet=0;	//后面可能要用的函数名字
static unsigned char* insist=0;		//在函数外面碰到了左括号:
static unsigned char* doubt=0;		//有些人写代码else myfunc ()

//这到底是不是个函数
static int chance=0;
static int roundbracket=0;

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




static int copyname(u8* src, u8* dst)
{
	int i=0;
	u64 temp;

	//2byte:	if
	if(src[2]==' ' | src[2]=='(' | src[2]==0x9)
	{
		temp=*(u16*)src;
		if(temp==0x6669)
		{
			i=2;
			goto decide;
		}
	}

	//3byte:	for
	if(src[3]==' ' | src[3]=='(' | src[3]==0x9)
	{
		temp = *(u32*)src;
		temp &= 0xffffff;
		if(temp==0x726f66)
		{
			i=3;
			goto decide;
		}
	}

	//4byte:	else,free
	if(src[4]==' ' | src[4]=='(' | src[4]==0x9)
	{
		temp=*(u32*)src;
		if(temp==0x65736c65)
		{
			i=4;
			goto decide;
		}
		else if(temp==0x65657266)
		{
			i=4;
			goto decide;
		}
	}

	//5byte:	while
	if(src[5]==' ' | src[5]=='(' | src[5]==0x9)
	{
		temp = *(u64*)src;
		temp &= 0xffffffffff;
		if(temp==0x656c696877)
		{
			i=5;
			goto decide;
		}
	}

	//6byte:	return,sizeof,switch,printf,malloc
	if(src[6]==' ' | src[6]=='(' | src[6]==0x9)
	{
		temp =  *(u64*)src;
		temp &= 0xffffffffffff;

		//return
		if(temp==0x6e7275746572)
		{
			i=6;
			goto decide;
		}

		//sizeof
		else if(temp==0x666f657a6973)
		{
			i=6;
			goto decide;
		}

		//switch
		else if(temp==0x686374697773)
		{
			i=6;
			goto decide;
		}

		//printf
		else if(temp==0x66746e697270)
		{
			i=6;
			goto decide;
		}

		//malloc
		else if(temp==0x636f6c6c616d)
		{
			i=6;
			goto decide;
		}

	}
decide:
	if(i!=0)
	{
		if(infunc != 0)return 0;

		*(u64*)dst=temp;
		*(u32*)(dst+i)=0x3f3f3f3f;
		return i+4;
	}

forcecopy:
	for(i=0;i<80;i++)
	{
		if(	((src[i]>='a')&&(src[i]<='z')) |
			((src[i]>='A')&&(src[i]<='Z')) |
			((src[i]>='0')&&(src[i]<='9')) |
			(src[i]=='_') |
			(src[i]=='.') |
			(src[i]=='-') |
			(src[i]=='>') )
		{
			dst[i]=src[i];
		}
		else break;
	}

	//0
	dst[i]=0;
	return i;

}
static void c_write(u8* p)
{
	int count=0;
	u8 strbuf[256];

	//函数结束
	if(p==0)
	{
		strbuf[0]='}';
		strbuf[1]='\n';
		strbuf[2]=0;
		count=2;
		goto finalprint;
	}

	//在函数外
	if(infunc==0)
	{
		count = copyname(p , strbuf);
		worker_write(p, count-1, 1);
		count += snprintf(
			strbuf+count,	0x80,
			"	@%d\n{\n",	countline
		);
	}
	else
	{
		strbuf[0]=0x9;
		count++;

		count += copyname(p , strbuf+1);
		if(count==1)return;

		worker_write(p, count-1, 2);

		strbuf[count]='\n';
		strbuf[count+1]=0;
		count++;
	}

finalprint:
	//write(outfile,strbuf,count);
	printf("%s",strbuf);
	return;
}
static int c_read(char* src, int count)
{
	unsigned char ch=0;
	countbyte = 0;

/*
	printf(
		"@%x@%d -> %d,%d,%d,%d\n",
		countbyte,
		countline+1,
		infunc,
		inmarco,
		innote,
		instr
	);
*/
	//
	for(;countbyte<0x100000;countbyte++)
	{
		//拿一个
		ch=src[countbyte];

		//软退
		if( (countbyte>count) && (prophet==0) && (insist==0))
		{
			if(ch==' ')break;
			else if(ch==0x9)break;
			else if(ch==0xa)break;
			else if(ch==0xd)break;
		}

		//强退(代码里绝不会有真正的0，都是ascii的0x30)
		if(ch==0)
		{
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

			//换行了，可能函数名不对了
			if(prophet != 0)doubt=src+countbyte;
		}

		//0xd:		mac或是windows的换行符
		else if(ch==0xd)
		{
			//如果是windows的换行符，吃掉后面的0xa
			countline++;
			if(src[countbyte+1]==0xa)countbyte++;

			//define宏，换行清零
			if(inmarco=='d')inmarco=0;

			//单行注释，换行清零
			if(innote==1)innote=0;

			//字符串，换行清零
			if(instr==1)instr=0;

			//换行了，可能函数名不对了
			if(prophet != 0)doubt = src + countbyte;
		}

		//.....................
		else if(ch=='\\')
		{
			//linux的换行
			if(src[countbyte+1] == 0xa)
			{
				countline++;
			}

			//mac或者windows的换行
			else if(src[countbyte+1] == 0xd)
			{
				//windows的换行多吃掉一个
				if(src[countbyte+2] == 0xa)countbyte++;
				countline++;
			}

			//吃一个，然后换行
			countbyte++;
			continue;
		}

		//prophets' guess
		else if(
			(ch>='a' && ch<='z') |
			(ch>='A' && ch<='Z') |
			(ch>='0' && ch<='9') |
			ch=='_' )
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			chance=0;

			//
			if(prophet == 0)prophet = src + countbyte;
			else
			{
				if(doubt!=0)
				{
					doubt=0;
					prophet = src + countbyte;
				}
			}
		}

		//prophets' doubt
		else if( (ch==' ')|(ch==0x9) )
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			if(prophet != 0)doubt = src + countbyte;
		}

		//prophets' fable right or wrong
		else if(ch=='(')
		{
			if(inmarco>=2|innote>0|instr>0)continue;

			//somthing like:    what=func();
			if(infunc > 0)
			{
				if(prophet!=0)
				{
					c_write(prophet);
					prophet=0;
					doubt=0;
				}
			}

			//在函数外面碰到了左括号
			else
			{
				if(prophet!=0)
				{
					if(roundbracket==0)insist=prophet;
					prophet=0;
					doubt=0;
				}
				roundbracket++;
			}
		}
		else if(ch==')')
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			prophet=0;
			doubt=0;

			if(infunc==0)
			{
				chance=1;
				roundbracket--;
			}
		}

		else if(ch=='{')
		{
			if(inmarco>=2|innote>0|instr>0)continue;

			//已经在函数里
			if(infunc!=0)infunc++;

			//确认这即将是个函数
			else
			{
				//消灭aaa=(struct){int a,int b}这种
				if( (chance>0) && (insist!=0) )
				{
					c_write(insist);

					infunc++;
					chance=0;
					prophet=insist=doubt=0;
				}//chance && insist!=0
			}//infunc
		}

		else if(ch=='}')
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			chance=0;

			if(infunc>0)
			{
				infunc--;
				if(infunc==0)c_write(0);
			}
		}

		else if(ch=='\"')
		{
			if(innote>0)continue;
			if( instr==0 )
			{
				instr=1;
			}
			else if(instr==1)
			{
				instr=0;
			}
		}

		//这里有bug，必须干掉单引号括一个字符的情况
		else if(ch=='\'')
		{
			if(innote>0|instr>0)continue;

			if(src[countbyte+2] == '\'')
			{
				countbyte += 2;
			}
		}

		else if(ch=='/')
		{
			//在这三种情况下什么都不能干
			if(innote>0|instr>0)continue;

			//单行注释很好解决
			if(src[countbyte+1] == '/')
			{
				innote=1;
				countbyte++;
			}

			//多行注释
			else if(src[countbyte+1]=='*')
			{
				innote=9;
				countbyte++;
			}
		}

		else if(ch=='*')
		{
			if((innote==1)|(instr>0))continue;

			if(src[countbyte+1]=='/')
			{
				if(innote==9)
				{
					innote=0;
					countbyte++;
				}
			}
			prophet=0;
			doubt=0;
		}

		else if(ch==',')
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			chance=0;
			doubt=0;
			prophet=0;
		}

		else if(ch==':')
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			prophet=0;
		}

		else if( (ch=='&') | (ch=='+') | (ch=='-') | (ch=='>') | (ch=='<') )
		{
			if(inmarco>=2|innote>0|instr>0)continue;

			if(infunc>0)
			{
				doubt=0;
				prophet=0;
				insist=0;
			}
		}

		else if( (ch=='=') | (ch==';') | (ch=='|') ) 
		{
			if(inmarco>=2|innote>0|instr>0)continue;

			chance=0;
			prophet=doubt=insist=0;
		}

		else if(ch=='#')
		{
			//不在注释里面,也不在字符串里的时候
			if(innote>0|instr>0)continue;

			//吃掉所有空格和tab
			while(1)
			{
				if(src[countbyte+1]==' ')countbyte++;
				else if(src[countbyte+1]==0x9)countbyte++;
				else break;
			}

			//宏外面碰到#号
			if(inmarco==0)
			{
				//#define
				if( (*(u16*)(src+countbyte+1) )==0x6564 )
				{
					if( (*(u32*)(src+countbyte+3) )==0x656e6966 )
					{
						inmarco = 'd';
						countbyte += 6;
					}
				}

				//#else 这里是为了暂时不管宏嵌套的问题...
				else if( (*(u32*)(src+countbyte+1) )==0x65736c65 )
				{

					inmarco = 'e';
					countbyte += 4;
				}

				//#if
				else if( (*(u16*)(src+countbyte+1) )==0x6669 )
				{
					inmarco = 1;
					countbyte += 2;
				}
			}

			//普通宏里又碰到了#号
			else if(inmarco==1)
			{
/*
				//嵌套在#if里面的#define,这种解法不对
				if( (*(unsigned short*)(src+i+1) )==0x6564 )
				{
					if( (*(unsigned int*)(src+i+3) )==0x656e6966 )
					{
						i+=6;
						inmarco='d';
					}
				}
*/
				//#else -> 升级
				if( (*(u32*)(src+countbyte+1) )==0x65736c65 )
				{
					inmarco = 'e';
					countbyte += 4;
				}

				//#endif -> 降级
				else if((src[countbyte+1]=='e') &&
					(src[countbyte+2]=='n') &&
					(src[countbyte+3]=='d') &&
					(src[countbyte+4]=='i') &&
					(src[countbyte+5]=='f') )
				{
					inmarco = 0;
					countbyte += 5;
				}
			}

			//else里面碰到endif
			else if(inmarco=='e')
			{
				if( (src[countbyte+1]=='e') &&
				    (src[countbyte+2]=='n') &&
				    (src[countbyte+3]=='d') &&
				    (src[countbyte+4]=='i') &&
				    (src[countbyte+5]=='f') )
				{
					inmarco = 0;
					countbyte += 5;
				}
			}
		}//#marco

	}//for

	return countbyte;
}
static int c_list()
{
}
static int c_choose()
{
}
static int c_stop()
{
/*
	printf("@%x@%d -> %d,%d,%d,%d\n\n\n\n\n",
		countbyte,
		countline,
		infunc,
		inmarco,
		innote,
		instr
	);
*/
	//write(outfile,"\n\n\n\n",4);
}
static int c_start()
{
	chance=roundbracket=0;
	countbyte=countline=0;
	infunc = inmarco = innote = instr = 0;
	prophet=insist=doubt=0;
}
int c_delete()
{
}
int c_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x63;
	this[2] = (u64)c_start;
	this[3] = (u64)c_stop;
	this[4] = (u64)c_list;
	this[5] = (u64)c_choose;
	this[6] = (u64)c_read;
	this[7] = (u64)c_write;
}
