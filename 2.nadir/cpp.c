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




//fp
static int dest=-1;

//destination,source,datahome
static unsigned char* datahome;	//4k+4k
static unsigned char strbuf[256];
static unsigned char backup1[256];
static unsigned char backup2[256];

//the prophets who guide me
static unsigned char* prophet=0;	//后面可能要用的函数名字
static unsigned char* prophetinsist=0;	//在函数外面碰到了左括号:
static int doubt=0;		//"疑虑"(想更细致就出错):	else myfunc ()
static int chance=0;

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




int cpp_copyname(unsigned char* p,unsigned char* q)
{
	int i=0;
	unsigned long long temp;

	//2byte:	if
	if(p[2]==' ' | p[2]=='(' | p[2]==0x9)
	{
		temp=*(unsigned short*)p;
		if(temp==0x6669)
		{
			i=2;
			goto decide;
		}
	}

	//3byte:	for
	if(p[3]==' ' | p[3]=='(' | p[3]==0x9)
	{
		temp=0xffffff & (*(unsigned int*)p);
		if(temp==0x726f66)
		{
			i=3;
			goto decide;
		}
	}

	//4byte:	else
	if(p[4]==' ' | p[4]=='(' | p[4]==0x9)
	{
		temp=*(unsigned int*)p;
		if(temp==0x65736c65)
		{
			i=4;
			goto decide;
		}
	}

	//5byte:	while
	if(p[5]==' ' | p[5]=='(' | p[5]==0x9)
	{
		temp=0xffffffffff & (*(unsigned long long*)p);
		if(temp==0x656c696877)
		{
			i=5;
			goto decide;
		}
	}

	//6byte:	printf,return,sizeof,switch
	if(p[6]==' ' | p[6]=='(' | p[6]==0x9)
	{
		temp=0xffffffffffff & (*(unsigned long long*)p);
/*
		//printf
		if(temp==0x66746e697270)
		{
			i=6;
			goto decide;
		}
*/
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
	}
decide:
	if(i!=0)
	{
		if(infunc != 0)return 0;

		*(unsigned long long*)q=temp;
		*(unsigned int*)(q+i)=0x3f3f3f3f;
		return i+4;
	}

forcecopy:
	for(i=0;i<80;i++)
	{
		if(	((p[i]>='a')&&(p[i]<='z')) |
			((p[i]>='A')&&(p[i]<='Z')) |
			((p[i]>='0')&&(p[i]<='9')) |
			(p[i]=='_') |
			(p[i]=='.') |
			(p[i]=='-') |
			(p[i]=='>') )
		{
			q[i]=p[i];
		}
		else break;
	}

	//0
	q[i]=0;
	return i;

}
void cpp_printprophet(unsigned char* p)
{
	int count=0;

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
		count=cpp_copyname(p , strbuf);
		count+=snprintf(
			strbuf+count,
			0x80,
			"	@%d\n{\n",
			countline
		);
	}
	else
	{
		strbuf[0]=0x9;
		count++;

		count += cpp_copyname(p , strbuf+1);
		if(count==1)return;

		strbuf[count]='\n';
		strbuf[count+1]=0;
		count++;
	}

finalprint:
	write(dest,strbuf,count);
	//printf("%s",dest);
}
int cpp_explain(int start,int end)
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
		//printf("%c",ch);

		//软退
		if( (i>end) && (prophet==0) && (prophetinsist==0))
		{
			if(ch==' ')break;
			else if(ch==0x9)break;
			else if(ch==0xa)break;
			else if(ch==0xd)break;
		}

		//强退(代码里绝不会有真正的0，都是ascii的0x30)
		if(ch==0)
		{
			//保存一下上次的名字
			if(prophet!=0)
			{
				cpp_copyname(prophet,backup1);
				prophet=backup1;
			}
			if(prophetinsist!=0)
			{
				cpp_copyname(prophetinsist,backup2);
				prophetinsist=backup2;
			}

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

			//换行了，可能函数名不对了
			if(prophet != 0)doubt=1;
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

			//换行了，可能函数名不对了
			if(prophet != 0)doubt=1;
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
			if(prophet==0)prophet=datahome+i;
			else
			{
				if(doubt==1)
				{
					doubt=0;
					prophet=datahome+i;
				}
			}
		}

		//prophets' doubt
		else if( (ch==' ')|(ch==0x9) )
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			if(prophet != 0)doubt=1;
		}

		//prophets' fable right or wrong
		else if(ch=='(')
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			if(prophet!=0)
			{
				//somthing like:    what=func();
				if(infunc > 0)
				{
					cpp_printprophet(prophet);
				}

				//在函数外面碰到了左括号
				else
				{
					prophetinsist=prophet;
				}

				prophet=0;
				doubt=0;
			}
		}

		else if(ch==')')
		{
			if(inmarco>=2|innote>0|instr>0)continue;
			prophet=0;
			doubt=0;

			if(infunc==0)chance=1;
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
				if( (chance>0) && (prophetinsist!=0) )
				{
					cpp_printprophet(prophetinsist);

					infunc++;
					prophet=prophetinsist=0;
					doubt=chance=0;
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
				if(infunc==0)cpp_printprophet(0);
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

			if(datahome[i+2]=='\'')
			{
				i+=2;
			}
		}

		else if(datahome[i]=='/')
		{
			//在这三种情况下什么都不能干
			if(innote>0|instr>0)continue;

			//单行注释很好解决
			if(datahome[i+1]=='/')	//    //
			{
				innote=1;
				i++;
			}

			//多行注释
			else if(datahome[i+1]=='*')	//    /*
			{
				innote=9;
				i++;
			}
		}

		else if(datahome[i]=='*')
		{
			if((innote==1)|(instr>0))continue;

			if(datahome[i+1]=='/')
			{
				if(innote==9)
				{
					innote=0;
					i++;
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
				prophetinsist=0;
			}
		}

		else if( (ch=='=') | (ch==';') | (ch=='|') ) 
		{
			if(inmarco>=2|innote>0|instr>0)continue;

			chance=0;
			doubt=0;
			prophet=0;
			prophetinsist=0;
		}

		else if(ch=='#')
		{
			//不在注释里面,也不在字符串里的时候
			if(innote>0|instr>0)continue;

			//吃掉所有空格和tab
			while(1)
			{
				if( (datahome[i+1]==' ') | (datahome[i+1]==0x9) )i++;
				else break;
			}

			//宏外面碰到#号
			if(inmarco==0)
			{
				//#define
				if( (*(unsigned short*)(datahome+i+1) )==0x6564 )
				{
					if( (*(unsigned int*)(datahome+i+3) )==0x656e6966 )
					{
						i+=6;
						inmarco='d';
					}
				}

				//#else 这里是为了暂时不管宏嵌套的问题...
				else if( (*(unsigned int*)(datahome+i+1) )==0x65736c65 )
				{

					inmarco='e';
					i+=4;
				}

				//#if
				else if( (*(unsigned short*)(datahome+i+1) )==0x6669 )
				{
					inmarco=1;
					i+=2;
				}
			}

			//普通宏里又碰到了#号
			else if(inmarco==1)
			{
/*
				//嵌套在#if里面的#define,这种解法不对
				if( (*(unsigned short*)(datahome+i+1) )==0x6564 )
				{
					if( (*(unsigned int*)(datahome+i+3) )==0x656e6966 )
					{
						i+=6;
						inmarco='d';
					}
				}
*/
				//#else -> 升级
				if( (*(unsigned int*)(datahome+i+1) )==0x65736c65 )
				{

					inmarco='e';
					i+=4;
				}

				//#endif -> 降级
				else if( (datahome[i+1]=='e') &&
				    (datahome[i+2]=='n') &&
				    (datahome[i+3]=='d') &&
				    (datahome[i+4]=='i') &&
				    (datahome[i+5]=='f') )
				{
					inmarco=0;
					i+=5;
				}
			}

			//else里面碰到endif号
			else if(inmarco=='e')
			{
				if( (datahome[i+1]=='e') &&
				    (datahome[i+2]=='n') &&
				    (datahome[i+3]=='d') &&
				    (datahome[i+4]=='i') &&
				    (datahome[i+5]=='f') )
				{
					inmarco=0;
					i+=5;
				}
			}
		}//#marco

	}//for

	countbyte += 0x100000;
	return i-end;	//可能多分析了几十几百个字节
}
int cpp_start(char* thisfile,int size)
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
	chance=doubt=0;
	countbyte=countline=0;
	prophet=prophetinsist=0;
	infunc = inmarco = innote = instr = 0;
}
int cpp_stop(int where)
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
int cpp_init(char* file,char* memory)
{
	//
	dest=open(
		file,
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	datahome=memory;
}
int cpp_kill()
{
	close(dest);
}
