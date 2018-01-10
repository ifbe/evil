#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u64 unsigned long long
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))




void arm2ac_line(char* buf, int len)
{
	printf("%.*s\n",len,buf);
}
int arm2ac(int argc, char** argv)
{
	int j,k,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);

	k = 0;
	for(j=0;j<len;j++)
	{
		if((buf[j] == '/')&&(buf[j+1] == '*'))
		{
			for(;j<len;j++)
			{
				if((buf[j] == '*')&&(buf[j+1] == '/'))
				{
					j++;
					break;
				}
			}
		}
		else if(buf[j] == '@')
		{
			for(;j<len;j++)
			{
				if(buf[j] == '\n')break;
			}
			buf[k] = buf[j];
			k++;
		}
		else
		{
			buf[k] = buf[j];
			k++;
		}
	}
	len = k;

	k = 0;
	for(j=0;j<=len;j++)
	{
		if((j==len) | (buf[j]=='\n'))
		{
			arm2ac_line(buf+k, j-k);
			k=j+1;
		}
	}
}





void att2ac_fmt(char* dst, char* src, int len)
{
	int j;
	for(;len>0;len--)
	{
		if((src[len-1] != 0x9)&&(src[len-1] != 0x20))break;
	}
	for(j=0;j<len;j++)
	{
		if(	(src[j] != 0x9)&&(src[j] != 0x20) &&
			(src[j] != '%')&&(src[j] !=  '$') )
		{break;}
	}
	snprintf(dst, 32, "%.*s", len-j, src+j);
}
void att2ac_line(char* buf, int len)
{
	int j,k;
	int type = 0;
	char fmt1[32];
	char fmt2[32];
	for(j=0;j<len;j++)
	{
		if((buf[j] != 0x20)&&(buf[j] != 0x9))break;
	}

	if(strncmp(buf+j, "mov", 3) == 0)type = 1;
	else if(strncmp(buf+j, "lea", 3) == 0)type = 2;
	else if(strncmp(buf+j, "add", 3) == 0)type = 3;
	else if(strncmp(buf+j, "sub", 3) == 0)type = 4;
	else if(strncmp(buf+j, "xor", 3) == 0)type = 5;
	else if(strncmp(buf+j, "and", 3) == 0)type = 6;
	else if(strncmp(buf+j, "cmp", 3) == 0)type = 7;
	else if(strncmp(buf+j, "not", 3) == 0)type = 20;
	else if(strncmp(buf+j, "inc", 3) == 0)type = 21;
	else if(strncmp(buf+j, "dec", 3) == 0)type = 22;
	else if(strncmp(buf+j, "push", 4) == 0)type = 23;
	else if(strncmp(buf+j, "pop", 3) == 0)type = 24;
	else if(strncmp(buf+j, "call", 4) == 0)type = 25;
	else if(strncmp(buf+j, "jmp", 3) == 0)type = 26;

	if(type>0)
	{
		j+=3;
		for(;j<len;j++)
		{
			if((buf[j] < 'a')|(buf[j] > 'z'))break;
		}
		for(;j<len;j++)
		{
			if((buf[j] != ' ')&&(buf[j] != '	'))break;
		}

		if(type >= 20)
		{
			att2ac_fmt(fmt2, buf+j, len-j);
			if(type==20)printf("not %s\n", fmt2);
			else if(type == 21)printf("inc %s\n", fmt2);
			else if(type == 22)printf("dec %s\n", fmt2);
			else if(type == 23)printf("push %s\n", fmt2);
			else if(type == 24)printf("pop %s\n", fmt2);
			else if(type == 25)printf("call %s\n", fmt2);
			else if(type == 26)printf("jmp %s\n", fmt2);
		}
		else
		{
			for(k=j;k<len;k++)
			{
				if(buf[k] == ',')break;
			}

			att2ac_fmt(fmt1, buf+k+1, len-k-1);
			att2ac_fmt(fmt2, buf+j, k-j);

			if((type==1)|(type==2))printf("%s = %s\n",fmt1,fmt2);
			else if(type == 3)printf("%s + %s\n",fmt1,fmt2);
			else if(type == 4)printf("%s - %s\n",fmt1,fmt2);
			else if(type == 5)printf("%s ^ %s\n",fmt1,fmt2);
			else if(type == 6)printf("%s & %s\n",fmt1,fmt2);
			else if(type == 7)printf("%s ? %s\n",fmt1,fmt2);
		}
	}
	else
	{
		printf("%.*s\n",len-j,buf+j);
	}
}
int att2ac(int argc, char** argv)
{
	int j,k,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);

	k = 0;
	for(j=0;j<len;j++)
	{
		if((buf[j] == '/')&&(buf[j+1] == '*'))
		{
			for(;j<len;j++)
			{
				if((buf[j] == '*')&&(buf[j+1] == '/'))
				{
					j++;
					break;
				}
			}
		}
		else
		{
			buf[k] = buf[j];
			k++;
		}
	}
	len = k;

	k = 0;
	for(j=0;j<=len;j++)
	{
		if((j==len) | (buf[j]=='\n'))
		{
			att2ac_line(buf+k, j-k);
			k=j+1;
		}
	}
}
int intel2ac(int argc, char** argv)
{
	int j,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);
}




void ac2arm_line(char* buf, int len)
{
	printf("%.*s\n",len,buf);
}
int ac2arm(int argc, char** argv)
{
	int j,k,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);

	k = 0;
	for(j=0;j<=len;j++)
	{
		if((j==len) | (buf[j]=='\n'))
		{
			ac2arm_line(buf+k, j-k);
			k=j+1;
		}
	}
}




void ac2att_fmt(char* dst, int brack, char* src, int len)
{
	int j,k,t;
	int bl,il,ol;
	char* base=0;
	char* index=0;
	char* scale="1";
	char* offset=0;

	if(brack == 0)
	{
		if((src[0] >= '0')&&(src[0] <= '9'))dst[0] = '$';
		else dst[0] = '%';
		snprintf(dst+1, 32, "%.*s", len, src);
	}
	else
	{
		//printf("<%.*s>",len,src);
		k = 0;
		for(j=0;j<=len;j++)
		{
			if((j==len)|(src[j] == '+')|(src[j] == '-'))
			{
				if((src[k]>='a')&&(src[k]<='z'))
				{
					base = src+k;
					bl = j-k;
				}
				else if((src[k]=='-')|((src[k]>='0')&&(src[k]<='9')))
				{
					offset = src+k;
					ol = j-k;
				}

				if(src[j] == '-')k = j;
				else k = j+1;
			}
			else if(src[j] == '*')
			{
				for(t=j+1;t<=len;t++)
				{
					if((src[t] == '+')|(src[t] == '-'))break;
				}

				if((src[k]>='a')&&(src[k]<='z'))
				{
					index = src+k;
					il = j-k;
					scale = src+j+1;
				}
				else if((src[k]>='0')&&(src[k]<='9'))
				{
					scale = src+k;
					index = src+j+1;
					il = t-j-1;
				}

				j = t;
				if(src[t] == '-')k = t;
				else k=t+1;
			}
		}

		j=0;
		if(offset != 0)j = snprintf(dst,32,"%.*s",ol,offset);
		if((base==0)&&(index==0))return;

		j += snprintf(dst+j, 32-j, "(");
		if(base != 0)
		{
			j += snprintf(dst+j, 32-j,
				"%%%.*s", bl, base
			);
		}
		if(index != 0)
		{
			j += snprintf(dst+j, 32-j,
				",%%%.*s,%.*s",
				il, index,
				1, scale
			);
		}
		j += snprintf(dst+j, 32-j, ")");
	}
	//printf("{%s}",dst);
}
void ac2att_line(char* buf, int len)
{
	int j,k;
	char* opcode = 0;
	char fmt1[32];
	char fmt2[32];
	for(j=0;j<len;j++)
	{
		if(	(buf[j] == '=') | (buf[j] == '?') |
			(buf[j] == '<') | (buf[j] == '>') |
			(buf[j] == '+') | (buf[j] == '-') |
			(buf[j] == '*') )
		{
			break;
		}
	}
	if(j >= len)
	{
		printf("%.*s\n",len,buf);
		return;
	}

	if(buf[j] == '=')
	{
		for(k=j+1;k<len;k++)
		{
			if(	(buf[k] == '[') |
				(buf[k] == '*') |
				(buf[k] == '+') |
				(buf[k] == '-') )
			{break;}
		}
		if(k >= len)
		{
			opcode = "mov";
			ac2att_fmt(fmt1, 0, buf+j+1, len-j-1);
		}
		else
		{
			if(buf[k] == '[')
			{
				opcode = "mov";
				ac2att_fmt(fmt1, 1, buf+k+1, len-k-2);
			}
			else
			{
				opcode = "lea";
				ac2att_fmt(fmt1, 1, buf+j+1, len-j-1);
			}
		}
		ac2att_fmt(fmt2, 0, buf, j);
	}
	else
	{
		if(buf[j] == '?')opcode = "cmp";
		else if(buf[j] == '<')opcode = "shl";
		else if(buf[j] == '>')opcode = "shr";
		else if(buf[j] == '+')opcode = "add";
		else if(buf[j] == '-')opcode = "sub";
		else if(buf[j] == '*')opcode = "imul";

		ac2att_fmt(fmt1, 0, buf+j+1, len-j-1);
		ac2att_fmt(fmt2, 0, buf, j);
	}

	printf("%s ", opcode);
	printf("%s,%s\n", fmt1, fmt2);
}
int ac2att(int argc, char** argv)
{
	int j,k,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);

	k = 0;
	for(j=0;j<=len;j++)
	{
		if((j==len) | (buf[j]=='\n'))
		{
			ac2att_line(buf+k, j-k);
			k=j+1;
		}
	}
}




void ac2intel_line(char* buf, int len)
{
	int j,k;
	for(j=0;j<len;j++)
	{
		if(	(buf[j] == '=') | (buf[j] == '?') |
			(buf[j] == '<') | (buf[j] == '>') |
			(buf[j] == '+') | (buf[j] == '-') |
			(buf[j] == '*') )
		{
			break;
		}
	}
	if(j >= len)
	{
		printf("%.*s\n",len,buf);
	}
	else if(buf[j] == '=')
	{
		for(k=j+1;k<len;k++)
		{
			if(	(buf[k] == '[') |
				(buf[k] == '*') |
				(buf[k] == '+') |
				(buf[k] == '-') )
			{break;}
		}
		if((k >= len)|(buf[k] == '['))
		{
			printf("mov %.*s,%.*s\n",
				j,buf,
				len-j-1,buf+j+1
			);
		}
		else
		{
			printf("lea %.*s,[%.*s]\n",
				j,buf,
				len-j-1,buf+j+1
			);
		}
	}
	else if(buf[j] == '?')
	{
		printf("cmp %.*s,%.*s\n",
			j,buf,
			len-j-1,buf+j+1
		);
	}
	else if(buf[j] == '<')
	{
		printf("shl %.*s,%.*s\n",
			j,buf,
			len-j-1,buf+j+1
		);
	}
	else if(buf[j] == '>')
	{
		printf("shr %.*s,%.*s\n",
			j,buf,
			len-j-1,buf+j+1
		);
	}
	else if(buf[j] == '+')
	{
		printf("add %.*s,%.*s\n",
			j,buf,
			len-j-1,buf+j+1
		);
	}
	else if(buf[j] == '-')
	{
		printf("sub %.*s,%.*s\n",
			j,buf,
			len-j-1,buf+j+1
		);
	}
	else if(buf[j] == '*')
	{
		for(k=j+1;k<len;k++)
		{
			if(buf[k] == '*')break;
		}
		if(k < len)
		{
			printf("imul %.*s,%.*s,%.*s\n",
				j,buf,
				k-j-1,buf+j+1,
				len-k-1,buf+k+1
			);
		}
		else
		{
			printf("imul %.*s,%.*s\n",
				j,buf,
				len-j-1,buf+j+1
			);
		}
	}
	else if(buf[j] == '/')
	{
		printf("idiv %.*s,%.*s\n",j,buf,len-j-1,buf+j+1);
	}
}
int ac2intel(int argc, char** argv)
{
	int j,k,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);

	printf("[bits 64]\n");
	k = 0;
	for(j=0;j<=len;j++)
	{
		if((j==len) | (buf[j]=='\n'))
		{
			ac2intel_line(buf+k, j-k);
			k=j+1;
		}
	}
}




int conv(int argc, char** argv)
{
	if(argc <= 2)return 0;

	if(strcmp(argv[1], "arm2ac") == 0)
	{
		arm2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "att2ac") == 0)
	{
		att2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "intel2ac") == 0)
	{
		intel2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2arm") == 0)
	{
		ac2arm(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2att") == 0)
	{
		ac2att(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2intel") == 0)
	{
		ac2intel(argc-1, argv+1);
	}
}
