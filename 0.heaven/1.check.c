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
u64 stringhash_generate(char*,int);
void* stringhash_read(u64);
void connect_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype);
void* connect_read(int);




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




void checkfunc(int offset)
{
	printf("func%x\n", offset);
}
void checkfile(int offset)
{
	printf("file%x\n", offset);
}




void printpin(struct wire* w)
{
	u64 t1;
	u64 t2;
	u64 temp;
	while(1)
	{
		t1 = w->desttype;
		t2 = w->selftype;

		if(t2 != 0)printf("		%-8s %-8s %08x	%08x\n",
			&t1, &t2, w->chipinfo, w->footinfo);

		temp = w->samepinnextchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
}
void printdest(struct wire* base)
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

			printf("		%-8s %-8s %08x	%08x\n",
				&t1, &t2, w->chipinfo, w->footinfo);
		}

		temp = w->samepinprevchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
/*
	t1 = base->desttype;
	t2 = base->selftype;
	printf("		%-8s %-8s %08x	%08x\n",
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
	printf("%s\n", buf);
	printf("	(@0x%08x), (=0x%08x%08x)\n", h, h->hash1, h->hash0);


	//samepin
	temp = h->first;
	if(temp == 0)return;

	ipin = connect_read(temp);
	if(ipin == 0)return;

	//input(only one)
	if(ipin->selftype == 0)
	{
		printf("	input:\n");
		printpin(ipin);

		temp = ipin->samechipnextpin;
		if(temp == 0)return;

		opin = connect_read(temp);
	}
	else opin = ipin;

	//output(many)
	while(1)
	{
		printf("	output:\n");
		printdest(opin);

		temp = opin->samechipnextpin;
		if(temp == 0)break;

		opin = connect_read(temp);
		if(opin == 0)break;
	}
}
void check(int argc, char** argv)
{
	int j,len,temp;
	for(j=1;j<argc;j++)
	{
		len = strlen(argv[j]);
		if(len > 5)
		{
			if(strncmp(argv[j], "file@", 5)==0)
			{
				checkfile(atoi(argv[j] + 5));
				continue;
			}
			if(strncmp(argv[j], "func@", 5)==0)
			{
				checkfunc(atoi(argv[j] + 5));
				continue;
			}
		}
		checkhash(argv[j], len);
	}
}