#include<stdio.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
void* chip_write();
void* pin_write(void*, int);
//
void* strhash_write(void*, int);
u64 strhash_generate(void*, int);
//
void relation_write(
	void* uchip, u64 ufoot, u64 utype,
	void* bchip, u64 bfoot, u64 btype);




struct matchtable
{
	u64 name;
	void* body;
};
static struct matchtable mt[10];
static int tablen = 0;
//
struct chipindex
{
	u32 self;
	u32 what;
	u32 type;
	float data;

	u64 first;
	u64 last;
};
void* pinbody;
//
static int countline = 0;
static int innote = 0;
static int inchip = 0;
static int inname = 0;




void* cir_read_chip(u8* buf, int len)
{
	int j;
	u64 name;
	void* addr1;
	void* addr2;

	//check
	name = strhash_generate(buf, len);
	for(j=0;j<tablen;j++)
	{
		if(mt[j].name == name)return mt[j].body;
	}

	//create
	addr1 = strhash_write(buf, len);
	addr2 = chip_write();
	relation_write(
		addr1, 0, hex32('h','a','s','h'),
		addr2, 0, hex32('c','h','i','p')
	);

	mt[tablen].name = name;
	mt[tablen].body = addr2;
	tablen += 1;

	return addr2;
}
void cir_read_line(u8* buf, int len)
{
	int j,k;
	float f;
	void* addr;
	struct chipindex* chipbody;
	if(0 == buf)return;
	if(0 == len)return;
	//printf("%d	%.*s\n",countline, len, buf);

	for(j=0;j<len;j++)
	{
		if('}' == buf[j])return;
		if('{' == buf[j])
		{
			//printf("{%.*s}\n", j, buf);
			addr = strhash_write(buf, j);
			pinbody = pin_write(buf, len);
			relation_write(
				addr, 0, hex32('h','a','s','h'),
				pinbody, 0, hex32('p','i','n',0)
			);
			return;
		}
		if('(' == buf[j])
		{
			//printf("(%.*s, %.*s)\n", j, buf, len-j-2, buf+j+1);
			chipbody = cir_read_chip(buf, j);
			if(0 == chipbody->type)chipbody->type = buf[0];
			relation_write(
				chipbody, buf[j+1], hex32('c','h','i','p'),
				pinbody, 0, hex32('p','i','n',0)
			);
			return;
		}
		if('=' == buf[j])
		{
			for(k=j+1;k<len;k++)
			{
				if(buf[k] > 0x20)
				{
					sscanf(buf+k, "%f", &f);
					break;
				}
			}
			for(k=j-1;k>=0;k--)
			{
				if(buf[k] > 0x20)
				{
					k = k+1;
					break;
				}
			}
			//printf("%.*s = %f\n", k, buf, f);
			chipbody = cir_read_chip(buf, k);
			chipbody->type = buf[0];
			chipbody->data = f;
			return;
		}
	}
	printf("????%.*s\n", len, buf);
}
void cir_read(u8* buf, int len)
{
	int j,k,m,n;

	k = 0;
	for(j=0;j<len;j++)
	{
		if(('/' == buf[j])&&('*' == buf[j+1]))
		{
			for(;j<len;j++)
			{
				if(('*' == buf[j])&&('/' == buf[j+1]))
				{
					j += 1;
					break;
				}
				else if(0xa == buf[j])countline++;
			}
			k = j+1;
		}
		else if(0xa == buf[j])
		{
			for(m=k;m<j;m++)if(buf[m] > 0x20)break;
			for(n=j-1;n>=m;n--)if(buf[n] > 0x20)break;
			if(n-m > 0)
			{
				//printf("%d	%.*s\n", countline, n-m+1, buf+m);
				cir_read_line(buf+m, n-m+1);
			}

			k = j+1;
			countline++;
		}
	}
}




/*
static void* chip = 0;
static u64 foot = 0;
static u64 type = 0;
void throwall(u8* buf, int len)
{
	void* pin;
	void* hash;
	if(inname != 0)
	{
		if(inchip == 0)
		{
			//printf("chip:%.*s\n", len, buf);
			hash = strhash_write(buf, len);
			chip = chip_write(buf, len);
			if(buf[0] == 'N')type = hex64('c','h','i','p','N','M','O','S');
			else if(buf[0] == 'R')type = hex64('c','h','i','p','R',0,0,0);
			else if(buf[0] == 'V')type = hex64('c','h','i','p','V',0,0,0);
			else type = hex32('c','h','i','p');
			relation_write(
				hash, 0, hex32('h','a','s','h'),
				chip, 0, type);
		}
		else
		{
			if(inchip%2 == 0)
			{
				//printf("foot:%.*s\n", len, buf);
				foot = buf[0];
			}
			else
			{
				//printf("pin:%.*s\n", len, buf);
				pin = pin_write(buf, len);
				relation_write(
					pin, 0, hex32('p','i','n',0),
					chip, foot, type);
			}
			inchip++;
		}
	}
	inname = 0;
}




void cir_read(u8* buf, int len)
{
	u8 ch;
	int j;
	//printf("%.*s", len, buf);

	j=0;
	for(j=0;j<=len;j++)
	{
		ch = buf[j];
		if((ch == 0xa) | (ch == 0xd) )
		{
			throwall(buf+inname, j-inname);
			countline++;
			innote = 0;
			inname = 0;

			if((ch == 0xd)&&(buf[j+1] == 0xa))j++;
			continue;
		}
		if(innote != 0)continue;

		else if((ch == '/')&&(buf[j+1] == '/'))
		{
			innote = 1;
			j++;
			continue;
		}
		else if(ch == '{')
		{
			throwall(buf+inname, j-inname);
			inchip = 2;
			inname = 0;
		}
		else if(ch == '}')
		{
			throwall(buf+inname, j-inname);
			inchip = 0;
			inname = 0;
		}
		else if(ch == '@')
		{
			throwall(buf+inname, j-inname);
			inname = 0;
		}
		else if(
			((ch >= '0')&&(ch <= '9')) |
			((ch >= 'A')&&(ch <= 'Z')) |
			((ch >= 'a')&&(ch <= 'z')) |
			(ch=='+')|(ch=='-')|(ch=='@'))
		{
			if(inname == 0)inname = j;
		}
		else
		{
			throwall(buf+inname, j-inname);
		}
	}
}
*/
void cir_write()
{
}
void cir_list()
{
}
void cir_choose()
{
}
void cir_stop()
{
}
void cir_start()
{
	countline = 1;
	innote = 0;
	inchip = 0;
	inname = 0;
	tablen = 0;
}
void cir_delete()
{
}
void cir_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x726963;
	this[2] = (u64)cir_start;
	this[3] = (u64)cir_stop;
	this[4] = (u64)cir_list;
	this[5] = (u64)cir_choose;
	this[6] = (u64)cir_read;
	this[7] = (u64)cir_write;
}
