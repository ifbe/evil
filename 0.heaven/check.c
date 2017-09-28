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
	u32 desttype;		//eg: 'hash', 'dir', 'file', 'func'
	u32 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u32 samepinprevchip;
	u32 samepinnextchip;

	u32 chipinfo;
	u32 footinfo;
	u32 samechipprevpin;
	u32 samechipnextpin;
};




void checkfile_printpin(struct wire* w)
{
	u64 t1;
	u64 t2;
	u64 temp;
	u64 temp2 = 0;
	struct hash* h;
	while(1)
	{
		t1 = w->desttype;
		t2 = w->selftype;

		if(t2 != 0)
		{
			temp = *(u64*)&(w->chipinfo);
			printf("	%-8s %-8s %x	%x\n",
				&t1, &t2, w->chipinfo, w->footinfo);
		}

		temp = w->samepinnextchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
}
void checkfile_printdest(struct wire* base)
{
	u64 t1;
	u64 t2;
	u64 temp;
	struct wire* w = base;
	while(1)
	{
		if(w == 0)break;
		if(w->selftype == 0)
		{
			t1 = w->desttype;
			t2 = w->selftype;

			if(t1 == hex32('h','a','s','h'))
			{
				printf("	%-8s %-8s ", (void*)&t1, (void*)&t2);
				stringhash_print(*(u64*)&(w->chipinfo));
			}
			else
			{
				printf("	%-8s %-8s %08x	%08x\n",
				(void*)&t1, (void*)&t2, w->chipinfo, w->footinfo);
			}
		}

		temp = w->samepinprevchip;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		w = connect_read(temp);
		if(w == 0)break;
		//printf("w=%x\n",w);
	}
/*
	t1 = base->desttype;
	t2 = base->selftype;
	printf("	%-8s %-8s %08x	%08x\n",
		&t1, &t2, w->chipinfo, w->footinfo);
*/
}
void checkfile(int offset)
{
	u64 temp;
	struct fileindex* f;
	struct wire* ipin;
	struct wire* opin;

	f = filetrav_read(offset);
	printf("\nfile :%x @%llx\n", offset, (u64)f);

	temp = f->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	//input
	if(ipin->selftype == 0)
	{
		printf("i:\n");
		checkfile_printpin(ipin);

		temp = ipin->samechipnextpin;
		if(temp == 0)return;

		opin = connect_read(temp);
	}
	else opin = ipin;

	//output(many)
	while(1)
	{
		printf("o:\n");
		checkfile_printdest(opin);

		temp = opin->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		opin = connect_read(temp);
		if(opin == 0)break;
		//printf("opin=%x\n",temp);
	}
}




void checkfunc_printpin(struct wire* w)
{
	u64 t1;
	u64 t2;
	u64 temp;
	u64 temp2 = 0;
	struct hash* h;
	while(1)
	{
		t1 = w->desttype;
		t2 = w->selftype;

		//printf("%x %x\n", w->chipinfo, w->footinfo);
		if(t2 != 0)
		{
			temp = *(u64*)&(w->chipinfo);
			printf("	");
			stringhash_print(temp);
		}

		temp = w->samepinnextchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
}
void checkfunc_printdest(struct wire* base)
{
	u64 t1;
	u64 t2;
	u64 temp;
	struct wire* w = base;
	while(1)
	{
		if(w == 0)break;
		if(w->selftype == 0)
		{
			t1 = w->desttype;
			t2 = w->selftype;

			if(t1 == hex32('h','a','s','h'))
			{
				printf("	%-8s %-8s ", (void*)&t1, (void*)&t2);
				stringhash_print(*(u64*)&(w->chipinfo));
			}
			else
			{
				printf("	%-8s %-8s %08x	%08x\n",
				(void*)&t1, (void*)&t2, w->chipinfo, w->footinfo);
			}
		}

		temp = w->samepinprevchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
/*
	t1 = base->desttype;
	t2 = base->selftype;
	printf("	%-8s %-8s %08x	%08x\n",
		&t1, &t2, w->chipinfo, w->footinfo);
*/
}
void checkfunc(int offset)
{
	u64 temp;
	struct funcindex* f;
	struct wire* ipin;
	struct wire* opin;

	f = funcindx_read(offset);
	printf("\nfunc :%x @%llx\n", offset, (u64)f);

	temp = f->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	//input
	if(ipin->selftype == 0)
	{
		printf("i:\n");
		checkfunc_printpin(ipin);

		temp = ipin->samechipnextpin;
		if(temp == 0)return;

		opin = connect_read(temp);
	}
	else opin = ipin;

	//output(many)
	while(1)
	{
		printf("o:\n");
		checkfunc_printdest(opin);

		temp = opin->samechipnextpin;
		if(temp == 0)break;

		opin = connect_read(temp);
		if(opin == 0)break;
	}
}




void checkhash_printpin(struct wire* w)
{
	u64 t1;
	u64 t2;
	u64 temp;
	while(1)
	{
		t1 = w->desttype;
		t2 = w->selftype;

		if(t2 != 0)printf("	%-8s %-8s %08x	%08x\n",
			(void*)&t1, (void*)&t2, w->chipinfo, w->footinfo);

		temp = w->samepinnextchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
}
void checkhash_printdest(struct wire* base)
{
	u64 t1;
	u64 t2;
	u64 temp;
	struct wire* w = base;
	while(1)
	{
		if(w == 0)break;
		if(w->selftype == 0)
		{
			t1 = w->desttype;
			t2 = w->selftype;

			printf("	%-8s %-8s %08x	%08x\n",
			(void*)&t1, (void*)&t2, w->chipinfo, w->footinfo);
		}

		temp = w->samepinprevchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
/*
	t1 = base->desttype;
	t2 = base->selftype;
	printf("	%-8s %-8s %08x	%08x\n",
		&t1, &t2, w->chipinfo, w->footinfo);
*/
}
void checkhash(char* buf, int len)
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
		printf("notfound: %s\n", buf);
		return;
	}
	printf("\nhash: %08x%08x @0x%08llx(%s)\n",
		h->hash1, h->hash0, (u64)h, buf);


	//samepin
	temp = h->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	//input(only one)
	if(ipin->selftype == 0)
	{
		printf("i:\n");
		checkhash_printpin(ipin);

		temp = ipin->samechipnextpin;
		if(temp == 0)return;

		opin = connect_read(temp);
	}
	else opin = ipin;

	//output(many)
	while(1)
	{
		printf("o:\n");
		checkhash_printdest(opin);

		temp = opin->samechipnextpin;
		if(temp == 0)break;

		opin = connect_read(temp);
		if(opin == 0)break;
	}
}
void check(int argc, char** argv)
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
				checkfile(temp);
				continue;
			}
			if(strncmp(argv[j], "func@", 5)==0)
			{
				hexstr2data(argv[j] + 5, &temp);
				checkfunc(temp);
				continue;
			}
		}
		checkhash(argv[j], len);
	}
}
