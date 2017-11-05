#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
void* shape_write();
void* strhash_write(void*, int);
void* point_write(int num, double x, double y, double z);
void* point_read(int);
void connect_write(
	void* uchip, u64 ufoot, u64 utype,
	void* bchip, u64 bfoot, u64 btype);




static int countline = 0;
static int innote = 0;
static int inname = 0;
static int inpoint = 0;
static int inshape = 0;




static void* shape = 0;
void three_point(int num, double x, double y, double z)
{
	void* point = point_write(num, x, y, z);
	//printf("%d	%lf, %lf, %lf\n", num, x, y, z);
}
void three_shape(char* buf, int len)
{
	void* hash = strhash_write(buf, len);
	shape = shape_write();
	connect_write(
		hash, 0, hex32('h','a','s','h'),
		shape, 0, hex32('s', 'h', 'a', 'p')
	);
	//printf("%.*s\n", len, buf);
}
void three_foot(int foot, int num)
{
	//printf("%c@%d\n", foot, which);
	void* point = point_read(num*0x20);
	connect_write(
		point, 0, hex32('p','o','i','n'),
		shape, foot, hex32('s', 'h', 'a', 'p')
	);
}




void three_read(u8* buf, int len)
{
	u8 ch;
	int j,k;
	int num, foot;
	double x,y,z;
	//printf("%.*s", len, buf);

	j=0;
	for(j=0;j<=len;j++)
	{
		ch = buf[j];
		if((ch == 0xa) | (ch == 0xd) )
		{
			if((inshape != 0) && (inname != 0))
			{
				sscanf(buf+inname, "%d", &num);
				//printf("point=%d\n", num);
				three_foot(foot, num);

				inname = 0;
			}

			if((ch == 0xd)&&(buf[j+1] == 0xa))j++;
			countline++;
			continue;
		}
		else if((ch == '/')&&(buf[j+1] == '*'))
		{
			innote = 1;
			j++;
			continue;
		}
		else if((ch == '*')&&(buf[j+1] == '/'))
		{
			innote = 0;
			j++;
			continue;
		}
		if(innote != 0)continue;

		else if(ch == '(')
		{
			sscanf(buf+inname, "%d", &num);
			//printf("%d:	", num);

			inpoint = j+1;
			inname = 0;
		}
		else if(ch == ')')
		{
			//printf("%.*s\n", j-inpoint, buf+inpoint);
			sscanf(buf+inpoint, "%lf, %lf, %lf", &x, &y, &z);
			//printf("%lf, %lf, %lf\n", x, y, z);

			three_point(num, x, y, z);
			inpoint = 0;
			inname = 0;
		}
		else if(ch == '{')
		{
			//printf("inname=%d\n", inname);
			for(k=inname;k<j;k++)
			{
				if(buf[k] == '{')break;
				if(buf[k] == '\r')break;
				if(buf[k] == '\n')break;
			}
			three_shape(buf+inname, k-inname);

			inshape = 2;
			inname = 0;
		}
		else if(ch == '}')
		{
			inshape = 0;
			inname = 0;
		}
		else if(ch == '@')
		{
			//printf("foot = %c	", buf[inname]);
			foot = buf[inname];
			inname = 0;
		}
		else if(
			((ch >= '0')&&(ch <= '9')) |
			((ch >= 'A')&&(ch <= 'Z')) |
			((ch >= 'a')&&(ch <= 'z')) )
		{
			if(inname == 0)inname = j;
			//printf("%c",ch);
		}
	}
}
void three_write()
{
}
void three_list()
{
}
void three_choose()
{
}
void three_start()
{
}
void three_stop()
{
}
void three_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x6433;
	this[2] = (u64)three_start;
	this[3] = (u64)three_stop;
	this[4] = (u64)three_list;
	this[5] = (u64)three_choose;
	this[6] = (u64)three_read;
	this[7] = (u64)three_write;
}
