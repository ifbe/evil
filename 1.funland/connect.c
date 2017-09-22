#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
//void* stringhash_read(u64 hash);




struct hash
{
	u32 hash0;
	u32 hash1;
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
static u8 wirebuf[0x100000];
static int wirefd;
static int wirelen;




//hashinfo, hashfoot, 'hash', fileinfo, 0, 'file'
//fileinfo, fileline, 'file', funcinfo, 0, 'func'
//funcinfo, funcofst, 'func', hashinfo, 0, 'hash'
//wininfo,  position, 'win',  actor,    0, 'act'
//actinfo,  which,    'act',  userinfo, what,     'user'
void connect_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype)
{
	struct hash* h1;
	struct hash* h2;
	struct wire* w1;
	struct wire* w2;
	struct wire* wc;




	//dest wire
	h1 = uchip;
	if(h1->first != 0)
	{
		//not necessary, because it should be first pin
		//check for (desttype == utype) && (selftype == 0)
		w1 = (void*)wirebuf + (h1->first);
	}
	else
	{
		if(wirelen > 0xfffe0)return;

		w1 = (void*)wirebuf + wirelen;
		wirelen += 0x20;

		w1->desttype = utype;
		w1->selftype = 0;

		if(utype == hex32('h','a','s','h'))
		{
			w1->chipinfo = h1->hash0;
			w1->footinfo = h1->hash1;
		}
		else
		{
			w1->chipinfo = *(u32*)h1;
			w1->footinfo = ufoot;
		}

		h1->first = (void*)w1 - (void*)wirebuf;
		h1->last = 0;
		//printf("first=%llx\n",h1->first);
	}




	//src wire
	h2 = bchip;
	if(wirelen >= 0xfffc0)return;

	w2 = (void*)wirebuf + wirelen;
	wirelen += 0x20;

	w2->desttype = utype;
	w2->selftype = btype;

	if(btype == hex32('h','a','s','h'))
	{
		w2->chipinfo = h2->hash0;
		w2->footinfo = h2->hash1;
	}
	else
	{
		w2->chipinfo = *(u32*)h2;
		w2->footinfo = ufoot;
	}

	if(h2->first == 0)
	{
		h2->first = (void*)w2 - (void*)wirebuf;
		h2->last = 0;
	}




	//w1->samechipprevpin = ?;		//unchanged
	//w1->samechipnextpin = ?;		//unchanged

	//w1->samepinprevchip = 0;		//certainly
	wc = w1;
	while(wc->samepinnextchip != 0)
	{
		wc = (void*)wirebuf + (wc->samepinnextchip);
	}
	wc->samepinnextchip = (void*)w2 - (void*)wirebuf;
	w2->samepinprevchip = (void*)wc - (void*)wirebuf;
	w2->samepinnextchip = 0;		//certainly

	wc = (void*)wirebuf + (h2->first);
	while(wc->samechipnextpin != 0)
	{
		wc = (void*)wirebuf + (wc->samechipnextpin);
	}
	if(wc != w2)
	{
		wc->samechipnextpin = (void*)w2 - (void*)wirebuf;
		w2->samechipprevpin = (void*)wc - (void*)wirebuf;
		w2->samechipnextpin = 0;
	}
}
void connect_read()
{
}
void connect_list()
{
}
void connect_choose()
{
}
void connect_start()
{
	wirelen = 0x20;
}
void connect_stop()
{
}
void connect_create()
{
	int j;
	char* buf;

	buf = (void*)wirebuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//wire
	wirefd = open(
		".42/42.wire",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	wirelen = read(wirefd, wirebuf, 0x100000);
	printf("connect :	%x\n", wirelen);
}
void connect_delete()
{
	lseek(wirefd, 0, SEEK_SET);
	write(wirefd, wirebuf, wirelen);
	close(wirefd);
}