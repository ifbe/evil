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
void worker_write(void*, int, int, int);




//count
static int countbyte=0;		//统计字节数
static int countline=0;		//统计行数

//函数名字
static char* prophet = 0;		//后面可能要用的函数名字
static char* insist = 0;		//在函数外面碰到了左括号:
static char* doubt = 0;		//有些人写代码else myfunc ()

//这到底是不是个函数
static int chance = 0;
static int roundbracket = 0;

//status
static int infunc = 0;
	//0:	不在函数里
	//1:	在函数内
	//?:	被包在第几个括号里
static int inmarco = 0;
	//0:	不在宏里
	//1:	在普通宏内
	//'d':	#define
	//'e':	#else
	//'i':	#include
static int innote = 0;
	//0:	不在注释里
	//1:	//
	//9:	/**/
static int instr = 0;
	//0:	不在字符串里
	//1:	在字符串内




static void purec_func(char* buf)
{
	int len;
	for(len=0;len<256;len++)
	{
		if(	((buf[len]>='a')&&(buf[len]<='z')) |
			((buf[len]>='A')&&(buf[len]<='Z')) |
			((buf[len]>='0')&&(buf[len]<='9')) |
			(buf[len]=='_') )continue;
		else break;
	}
	if(len == 1)return;
	if((len == 2)&&(strncmp(buf, "if", 2) == 0))return;
	if((len == 3)&&(strncmp(buf, "for", 3) == 0))return;
	if((len == 5)&&(strncmp(buf, "while", 5) == 0))return;

	if(infunc == 0)worker_write(buf, len, 1, countline);
	else worker_write(buf, len, 2, countline);
}
static void purec_str(char* buf, int len)
{
	int j;
	int score = 0;
	for(j=0;j<len;j++)
	{
		if(buf[j] == '%')score -= 2;
		else if(buf[j] >= 0x80)score++;
		else if((buf[j] >= 'a')&&(buf[j] <= 'z'))score++;
		else if((buf[j] >= 'A')&&(buf[j] <= 'Z'))score++;
		else score--;
	}
	if(score <= 0)return;

	worker_write(buf, len, 5, countline+1);
}




static int c_read(char* buf, int len)
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
		ch=buf[countbyte];

		//软退
		if( (countbyte>len) && (prophet==0) && (insist==0))
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

		//换行符
		else if((ch==0xa) | (ch == 0xd) )
		{
			//如果是windows的换行符，吃掉后面的0xa
			countline++;
			if((ch == 0xd) && (buf[countbyte+1] == 0xa))countbyte++;

			//define宏，换行清零
			if(inmarco=='d')inmarco=0;

			//单行注释，换行清零
			if(innote==1)innote=0;

			//字符串，换行清零
			if(instr != 0)instr=0;

			//换行了，可能函数名不对了
			if(prophet != 0)doubt = buf + countbyte;
		}

		//.....................
		else if(ch=='\\')
		{
			//linux的换行
			if(buf[countbyte+1] == 0xa)
			{
				countline++;
			}

			//mac或者windows的换行
			else if(buf[countbyte+1] == 0xd)
			{
				//windows的换行多吃掉一个
				if(buf[countbyte+2] == 0xa)countbyte++;
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
			if(prophet == 0)prophet = buf + countbyte;
			else
			{
				if(doubt!=0)
				{
					doubt=0;
					prophet = buf + countbyte;
				}
			}
		}

		//prophets' doubt
		else if( (ch==' ')|(ch==0x9) )
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			if(prophet != 0)doubt = buf + countbyte;
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
					purec_func(prophet);
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
					purec_func(insist);

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
				//if(infunc==0)printf("}\n");
			}
		}

		else if(ch=='\"')
		{
			if(innote>0)continue;
			if(instr == 0)
			{
				instr = countbyte+1;
			}
			else
			{
				//printf(">>>%.*s<<<\n", countbyte-instr, buf+instr);
				purec_str(buf+instr, countbyte-instr);
				instr=0;
			}
		}

		//这里有bug，必须干掉单引号括一个字符的情况
		else if(ch=='\'')
		{
			if(innote>0|instr>0)continue;

			if(buf[countbyte+2] == '\'')
			{
				countbyte += 2;
			}
		}

		else if(ch=='/')
		{
			//在这三种情况下什么都不能干
			if(innote>0|instr>0)continue;

			//单行注释很好解决
			if(buf[countbyte+1] == '/')
			{
				innote=1;
				countbyte++;
			}

			//多行注释
			else if(buf[countbyte+1]=='*')
			{
				innote=9;
				countbyte++;
			}
		}

		else if(ch=='*')
		{
			if((innote==1)|(instr>0))continue;

			if(buf[countbyte+1]=='/')
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
				if(buf[countbyte+1]==' ')countbyte++;
				else if(buf[countbyte+1]==0x9)countbyte++;
				else break;
			}

			//#if
			if(strncmp(buf+countbyte+1, "if", 2) == 0)
			{
				if(inmarco==0)
				{
					inmarco = 1;
					countbyte += 2;
				}
			}

			//#else 暂时不管宏嵌套...
			else if(strncmp(buf+countbyte+1, "else", 4) == 0)
			{
				if(inmarco==0)
				{
					inmarco = 'e';
					countbyte += 4;
				}
				else if(inmarco==1)
				{
					inmarco = 'e';
					countbyte += 4;
				}
			}

			//#endif
			else if(strncmp(buf+countbyte+1, "endif", 5) == 0)
			{
				inmarco = 0;
				countbyte += 5;
			}
			//#define
			else if(strncmp(buf+countbyte+1, "define", 6) == 0)
			{
				if(inmarco==0)
				{
					inmarco = 'd';
					countbyte += 6;
				}
			}

			//#include
			if(strncmp(buf+countbyte+1, "include", 7) == 0)
			{
				if(inmarco==0)
				{
					inmarco == 'i';
					countbyte += 7;
				}
			}
		}//#

	}//for

	return countbyte;
}
static void c_write()
{
}
static void c_list()
{
}
static void c_choose()
{
}
static void c_stop()
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
static void c_start()
{
	chance = roundbracket = 0;
	countbyte = countline = 0;
	infunc = inmarco = innote = instr = 0;
	prophet = insist = doubt = 0;
}
void c_delete()
{
}
void c_create(u64* that, u64* this)
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