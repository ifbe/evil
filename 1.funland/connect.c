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
	u64 destchip;
	u64 destfoot;
	u64 desttype;		//eg: 'hash', 'dir', 'file', 'func'
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selfchip;
	u64 selffoot;
	u64 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u32 samechipprevpin;
	u32 samechipnextpin;
};
#define maxlen 0x1000000
static u8 wirebuf[maxlen];
static int wirefd;
static int wirelen;




void* connect_write_new(struct hash* uchip, u64 ufoot, u64 utype, struct hash* bchip, u64 bfoot, u64 btype)
{
	struct wire* w;
	if(wirelen > maxlen-0x40)return 0;

	w = (void*)wirebuf + wirelen;
	wirelen += sizeof(struct wire);

	//1.dest
	w->desttype = utype;
	w->destfoot = ufoot;
	if(utype == hex32('h','a','s','h'))
	{
		w->destchip = *(u64*)uchip;
	}
	else
	{
		w->destchip = *(u32*)uchip;
	}

	//2.self
	w->selftype = btype;
	w->selffoot = bfoot;
	if(btype == 0)
	{
		w->selfchip = 0;
	}
	else if(btype == hex32('h','a','s','h'))
	{
		w->selfchip = *(u64*)bchip;
	}
	else
	{
		w->selfchip = *(u32*)bchip;
	}

	return w;
}
//hashinfo, hashfoot, 'hash', fileinfo, 0, 'file'
//fileinfo, fileline, 'file', funcinfo, 0, 'func'
//funcinfo, funcofst, 'func', hashinfo, 0, 'hash'
//wininfo,  position, 'win',  actor,    0, 'act'
//actinfo,  which,    'act',  userinfo, what,     'user'
int connect_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype)
{
	struct hash* h1;
	struct hash* h2;
	struct wire* w1;
	struct wire* w2;
	struct wire* wc;




	//dest wire
	h1 = uchip;
	if(h1->first == 0)
	{
		w1 = connect_write_new(uchip, ufoot, utype, 0, 0, 0);
		h1->first = (void*)w1 - (void*)wirebuf;
		//h1->last = 0;
	}
	else
	{
		w1 = (void*)wirebuf + (h1->first);
		if( (w1->desttype != utype) | (w1->selftype != 0) )
		{
			wc = w1;
			w1 = connect_write_new(h1, ufoot, utype, 0, 0, 0);
			h1->first = (void*)w1 - (void*)wirebuf;

			wc->samechipprevpin = (void*)w1 - (void*)wirebuf;
			w1->samechipnextpin = (void*)wc - (void*)wirebuf;
		}
	}




	//src wire
	h2 = bchip;
	w2 = connect_write_new(uchip, ufoot, utype, bchip, bfoot, btype);
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

	return 1;
}
void* connect_read(int offset)
{
	return (void*)wirebuf + offset;
}
void connect_list()
{
}
void connect_choose()
{
}
void connect_start(int flag)
{
	int j;
	char* name = ".42/42.wire";

	if(flag == 0)
	{
		wirefd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		wirelen = sizeof(struct wire);
	}
	else
	{
		//wire
		wirefd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//
		wirelen = read(wirefd, wirebuf, maxlen);
		printf("connect :	%x\n", wirelen);

		for(j=wirelen;j<maxlen;j++)wirebuf[j] = 0;
	}
}
void connect_stop()
{
}
void connect_create()
{
}
void connect_delete()
{
	lseek(wirefd, 0, SEEK_SET);
	write(wirefd, wirebuf, wirelen);
	close(wirefd);
}
