#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evil.h"
u64 strhash_generate(void*, int);
void* strhash_read(u64);
void* strhash_write(void*, int);
//
void* shapeindex_read(int);
void* shapeindex_write(void*, int);
void* pointindex_read(int);
void* pointindex_write(int num, float x, float y, float z, float w);
//
int decstr2data(void*, void*);
int hexstr2data(void*, void*);




static int innote = 0;
static int inname = 0;
static int inpoint = 0;
static int inshape = 0;
//
static int countline = 0;
static int rsp = 0;
static void* stack[16];




void three_point(char* buf, int len, int num)
{
	float x,y,z;
	sscanf(buf, "%f, %f, %f", &x, &y, &z);
	//printf("%lf, %lf, %lf\n", x, y, z);

	pointindex_write(num, x, y, z, 0.0);
	//printf("%d	%lf, %lf, %lf\n", num, x, y, z);
}
void three_shape(char* buf, int len)
{
	int j;
	void* shap;
	void* hash;
	for(j=0;j<len;j++)
	{
		if(buf[j] <= 0x20)break;
		if(buf[j] == '{')break;
	}
	//printf("%.*s\n", len, buf);

	//
	shap = shapeindex_write(buf, j);
	if(shap == 0)
	{
		printf("error@shapeindex_write\n");
		return;
	}

	//
	if(rsp <= 0)
	{
		hash = strhash_write(buf, j);
		relationcreate(
			hash, 0, hex32('h','a','s','h'),
			shap, 0, hex32('s','h','a','p')
		);
	}
	else
	{
		relationcreate(
			stack[rsp-1], 0, hex32('s','h','a','p'),
			shap,         0, hex32('s','h','a','p')
		);
	}

	//
	stack[rsp] = shap;
	rsp++;
}
void three_foot(char* buf ,int len)
{
	int j;
	u64 num;
	void* point;
	if(buf[0] < 0x30)return;
	if(buf[0] > 0x39)return;

	j = 0;
	while(1)
	{
		j += decstr2data(buf+j, &num);
		//printf("%d	", num);

		point = pointindex_read(num*0x20);
		relationcreate(
			stack[rsp-1], 0, hex32('s', 'h', 'a', 'p'),
			point, 0, hex32('p','o','i','n')
		);

		while(1)
		{
			if(j >= len)break;
			if((buf[j] >= 0x30) && (buf[j] <= 0x39))break;
			j++;
		}
		if(j >= len)break;
	}
	//printf("\n");
}
void three_call(u8* buf, int len)
{
	u64 temp;
	u64 haha;
	void* shap;
	struct hash* h;
	struct relation* w;
	//printf("%.*s\n", len, buf);

	temp = strhash_generate(buf, len);
	h = strhash_read(temp);
	if(h == 0)
	{
		printf("no str: %.*s", len, buf);
		return;
	}

	temp = h->irel0;
	w = relationread(temp);
	if((temp == 0) | (w == 0))
	{
		printf("no rel\n");
		return;
	}

	haha = 0;
	while(1)
	{
		if(w->selftype == hex32('s','h','a','p'))
		{
			if(haha != 0)
			{
				printf("more than one shap\n");
				return;
			}
			else haha = w->selfchip;
			//printf("shap@%llx\n", w->selfchip);
		}
		else
		{
			printf("%llx,%llx,%x\n", w->selfchip, w->selffoot, w->selftype);
		}

		temp = w->samedstnextsrc;
		if(temp == 0)break;

		w = relationread(temp);
		if(w == 0)break;
	}
	if(haha == 0)
	{
		printf("no shap from hash\n");
		return;
	}

	shap = shapeindex_read(haha);
	relationcreate(
		stack[rsp-1], 0, hex32('s','h','a','p'),
		shap, 0, hex32('s','h','a','p')
	);
}




void three_read(u8* buf, int len)
{
	u8 ch;
	int j, num;
	//printf("%.*s", len, buf);

	j=0;
	for(j=0;j<=len;j++)
	{
		ch = buf[j];
		if((ch == 0xa) | (ch == 0xd) )
		{
			if(inname != 0)
			{
				three_call(buf+inname, j-inname);
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
			sscanf((void*)buf+inname, "%d", &num);
			//printf("%d:	", num);

			inpoint = j+1;
			inname = 0;
		}
		else if(ch == ')')
		{
			three_point((void*)buf+inpoint, j-inpoint, num);
			inpoint = 0;
			inname = 0;
		}
		else if(ch == '{')
		{
			three_shape((void*)buf+inname, j-inname);
			inshape = 2;
			inname = 0;
		}
		else if(ch == '}')
		{
			if(inname != 0)three_foot((void*)buf+inname, j-inname);
			rsp--;

			inshape = 0;
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
