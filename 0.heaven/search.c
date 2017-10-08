#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
u32 bkdrhash(char*, int);
u32 djb2hash(char*, int);
int hexstr2data(void* src, void* data);
//
u64 stringhash_generate(char*,int);
void* stringhash_read(u64);
void stringhash_print(u64);
void* funcindx_read(int);
void* filetrav_read(int);
void connect_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype);
void* connect_read(int);
//
void filedata_start(int);
void filetrav_start(int);
void funcdata_start(int);
void funcindx_start(int);
void stringdata_start(int);
void stringhash_start(int);
void connect_start(int);




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
struct funcindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
struct wire
{
	u64 desttype;		//eg: 'hash', 'dir', 'file', 'func'
	u64 destchip;
	u64 destfoot;
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u64 selfchip;
	u64 selffoot;
	u32 samechipprevpin;
	u32 samechipnextpin;
};




void searchfile(int offset)
{
	u64 temp;
	struct fileindex* f;
	struct wire* ipin;
	struct wire* opin;
	if(offset%0x20 != 0)printf("\nnotfound: file@%x",offset);

	f = filetrav_read(offset);
	printf("\nfile :%x @%llx\n", offset, (u64)f);

	temp = f->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	if(ipin->selftype == 0)
	{
		temp = ipin->samechipnextpin;
		if(temp == 0)opin = 0;
		else opin = connect_read(temp);
	}
	else
	{
		opin = ipin;
		ipin = 0;
	}

	//input
	if(ipin != 0)printf("i:\n");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	hash	");
			stringhash_print(ipin->selfchip);
		}
		else if(ipin->selfchip != 0)
		{
			printf("	%-8s %08llx	%08llx	(@%lld)\n",
			(char*)&(ipin->selftype),
			ipin->selfchip, ipin->selffoot, ipin->destfoot);
		}

		temp = ipin->samepinnextchip;
		if(temp == 0)break;

		ipin = connect_read(temp);
		if(ipin == 0)break;
	}

	//output(many)
	while(opin != 0)
	{
		printf("o:\n");
		//searchfile_printdest(opin);
		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	hash	");
			stringhash_print(opin->destchip);
		}
		else
		{
			printf("	%-s %08llx	%08llx\n",
			(char*)&(opin->desttype),
			opin->destchip, opin->destfoot);
		}

		temp = opin->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		opin = connect_read(temp);
		if(opin == 0)break;
		//printf("opin=%x\n",temp);
	}
}




void searchfunc(int offset)
{
	u64 temp;
	struct funcindex* f;
	struct wire* ipin;
	struct wire* opin;
	if(offset%0x20 != 0)printf("\nnotfound: func@%x",offset);

	f = funcindx_read(offset);
	printf("\nfunc :%x @%llx\n", offset, (u64)f);

	temp = f->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	if(ipin->selftype == 0)
	{
		temp = ipin->samechipnextpin;
		if(temp == 0)opin = 0;
		else opin = connect_read(temp);
	}
	else
	{
		opin = ipin;
		ipin = 0;
	}

	//input
	if(ipin != 0)printf("i:\n");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	");
			stringhash_print(ipin->selfchip);
		}

		temp = ipin->samepinnextchip;
		if(temp == 0)break;

		ipin = connect_read(temp);
		if(ipin == 0)break;
	}

	//output(many)
	while(opin != 0)
	{
		printf("o:\n");
		//searchfunc_printdest(opin);
		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	hash	");
			stringhash_print(opin->destchip);
		}
		else if(opin->desttype == hex32('f','i','l','e'))
		{
			printf("	file	%08llx	%lld\n",
			opin->destchip, opin->destfoot);
		}
		else
		{
			printf("	%-s	%08llx	%08llx\n",
			(char*)&(opin->desttype),
			opin->destchip, opin->destfoot);
		}


		temp = opin->samechipnextpin;
		if(temp == 0)break;

		opin = connect_read(temp);
		if(opin == 0)break;
	}
}




void searchhash(char* buf, int len)
{
	u64 haha;
	u64 temp;
	struct hash* h;
	struct wire* ipin;
	struct wire* opin;

	haha = stringhash_generate(buf, len);
	h = stringhash_read(haha);
	if(h == 0)
	{
		printf("\nnotfound: (%llx)%s\n", haha, buf);
		return;
	}
	printf("\nhash: %08x%08x @0x%08llx(%s)\n",
		h->hash1, h->hash0, (u64)h, buf);


	//samepin
	temp = h->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	if(ipin->selftype == 0)
	{
		temp = ipin->samechipnextpin;
		if(temp == 0)opin = 0;
		else opin = connect_read(temp);
	}
	else
	{
		opin = ipin;
		ipin = 0;
	}
	//printf("temp=%llx,ipin=%llx,opin=%llx\n",temp,ipin,opin);

	//input(only one)
	if(ipin != 0)printf("i:\n");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	hash	");
			stringhash_print(ipin->selfchip);
		}
		else if(ipin->selftype != 0)
		{
			printf("	%-s	%08llx	%08llx\n",
			(char*)&(ipin->selftype),
			ipin->selfchip, ipin->selffoot);
		}

		temp = ipin->samepinnextchip;
		if(temp == 0)break;

		ipin = connect_read(temp);
		if(ipin == 0)break;
	}

	//output(many)
	while(opin != 0)
	{
		printf("o:\n");

		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	hash	");
			stringhash_print(opin->destchip);
		}
		else
		{
			printf("	%-s	%08llx	%08llx\n",
			(char*)&(opin->desttype),
			opin->destchip, opin->destfoot);
		}

		temp = opin->samechipnextpin;
		if(temp == 0)break;

		opin = connect_read(temp);
		if(opin == 0)break;
	}
}
void search(int argc, char** argv)
{
	int j,len;
	u64 temp;
	filedata_start(1);
	filetrav_start(1);
	funcdata_start(1);
	funcindx_start(1);
	stringdata_start(1);
	stringhash_start(1);
	connect_start(1);

	for(j=1;j<argc;j++)
	{
		len = strlen(argv[j]);
		if(len > 5)
		{
			if(strncmp(argv[j], "file@", 5)==0)
			{
				hexstr2data(argv[j] + 5, &temp);
				searchfile(temp);
				continue;
			}
			if(strncmp(argv[j], "func@", 5)==0)
			{
				hexstr2data(argv[j] + 5, &temp);
				searchfunc(temp);
				continue;
			}
		}
		searchhash(argv[j], len);
	}
}
