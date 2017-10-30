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
u64 strhash_generate(char*,int);
void strhash_print(u64);
void* strhash_read(u64);
//
void* pin_read(int);
void* chipindex_read(int);
void* funcindex_read(int);
void* filemd5_read(int);
//
void connect_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype);
void* connect_read(int);
//
void chipdata_start(int);
void chipindex_start(int);
void filedata_start(int);
void filemd5_start(int);
void funcdata_start(int);
void funcindex_start(int);
void pindata_start(int);
void pinindex_start(int);
void strdata_start(int);
void strhash_start(int);
void connect_start(int);




struct chipindex
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
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
struct hash
{
	u32 hash0;
	u32 hash1;
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




void chipname(u64 addr)
{
	u64 temp;
	struct wire* haha;
	struct fileindex* chip;

	chip = chipindex_read(addr);
	if(chip == 0)goto error;

	temp = chip->first;
	if(temp == 0)goto error;

	haha = connect_read(temp);
	if(haha == 0)goto error;

	if(haha->selftype == 0)
	{
		temp = haha->samechipnextpin;
		if(temp == 0)goto error;
		haha = connect_read(temp);
	}
	while(1)
	{
		if(haha->desttype == hex32('h','a','s','h'))
		{
			strhash_print(haha->destchip);
			break;
		}

		temp = haha->samechipnextpin;
		if(temp == 0)break;

		haha = connect_read(temp);
		if(haha == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}
void filename(u64 addr)
{
	u64 temp;
	struct wire* haha;
	struct fileindex* file;

	file = filemd5_read(addr);
	if(file == 0)goto error;

	temp = file->first;
	if(temp == 0)goto error;

	haha = connect_read(temp);
	if(haha == 0)goto error;

	if(haha->selftype == 0)
	{
		temp = haha->samechipnextpin;
		if(temp == 0)goto error;
		haha = connect_read(temp);
	}
	while(1)
	{
		if(haha->desttype == hex32('h','a','s','h'))
		{
			strhash_print(haha->destchip);
			break;
		}

		temp = haha->samechipnextpin;
		if(temp == 0)break;

		haha = connect_read(temp);
		if(haha == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}
void funcname(u64 addr)
{
	u64 temp;
	struct wire* haha;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	temp = func->first;
	if(temp == 0)goto error;

	haha = connect_read(temp);
	if(haha == 0)goto error;

	if(haha->selftype == 0)
	{
		temp = haha->samechipnextpin;
		if(temp == 0)goto error;
		haha = connect_read(temp);
	}
	while(1)
	{
		if(haha->desttype == hex32('h','a','s','h'))
		{
			strhash_print(haha->destchip);
			break;
		}

		temp = haha->samechipnextpin;
		if(temp == 0)break;

		haha = connect_read(temp);
		if(haha == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}
void funcpath(u64 addr)
{
	u64 temp;
	struct wire* haha;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	temp = func->first;
	if(temp == 0)goto error;

	haha = connect_read(temp);
	if(haha == 0)goto error;

	if(haha->selftype == 0)
	{
		temp = haha->samechipnextpin;
		if(temp == 0)goto error;
		haha = connect_read(temp);
	}
	while(1)
	{
		if(haha->desttype == hex32('f','i','l','e'))
		{
			filename(haha->destchip);
			printf(":%lld", haha->destfoot);
		}

		temp = haha->samechipnextpin;
		if(temp == 0)break;

		haha = connect_read(temp);
		if(haha == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}




void searchpin(int offset)
{
	u64 temp;
	struct fileindex* pin;
	struct wire* ipin;
	struct wire* opin;
	if(offset%0x20 != 0)printf("\nnotfound: pin@%x",offset);

	pin = pin_read(offset);
	printf("pin@%08x	@%llx\n", offset, (u64)pin);

	temp = pin->first;
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
	if(ipin != 0)printf("i:");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(ipin->selfchip);
			printf("\n");
		}
		else if(ipin->selftype == hex32('c','h','i','p'))
		{
			printf("	");
			chipname(ipin->selfchip);
			printf(":%c\n", (u32)(ipin->selffoot));
		}
		else if(ipin->selfchip != 0)
		{
			printf("	%-s@%08llx	%08llx	(@%lld)\n",
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
		printf("o:");
		//searchfile_printdest(opin);
		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(opin->destchip);
			printf("\n");
		}
		else if(opin->desttype == hex32('p','i','n',0))
		{
			printf("	%c@pin%llx\n",
				(u32)(opin->selffoot), (opin->destchip)/0x20);
		}
		else
		{
			printf("	%-s@%08llx	%08llx\n",
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




void searchchip(int offset)
{
	u64 temp;
	struct fileindex* chip;
	struct wire* ipin;
	struct wire* opin;
	if(offset%0x20 != 0)printf("\nnotfound: chip@%x",offset);

	chip = chipindex_read(offset);
	printf("chip@%08x	@%llx\n", offset, (u64)chip);

	temp = chip->first;
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
	if(ipin != 0)printf("i:");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(ipin->selfchip);
			printf("\n");
		}
		else if(ipin->selfchip != 0)
		{
			printf("	%-s@%08llx	%08llx	(@%lld)\n",
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
		printf("o:");
		//searchfile_printdest(opin);
		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(opin->destchip);
			printf("\n");
		}
		else if(opin->desttype == hex32('p','i','n',0))
		{
			printf("	%c@pin%llx\n",
				(u32)(opin->selffoot), (opin->destchip)/0x20);
		}
		else
		{
			printf("	%-s@%08llx	%08llx\n",
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




void searchfile(int offset)
{
	u64 temp;
	struct fileindex* file;
	struct wire* ipin;
	struct wire* opin;
	if(offset%0x20 != 0)printf("\nnotfound: file@%x",offset);

	file = filemd5_read(offset);
	printf("file@%08x	@%llx\n", offset, (u64)file);

	temp = file->first;
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
	if(ipin != 0)printf("i:");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(ipin->selfchip);
			printf("\n");
		}
		else if(ipin->selftype == hex32('f','u','n','c'))
		{
			printf("	func@%08llx	", ipin->selfchip);
			funcname(ipin->selfchip);
			printf("	");
			funcpath(ipin->selfchip);
			printf("\n");
		}
		else if(ipin->selfchip != 0)
		{
			printf("	%-s@%08llx	%08llx	(@%lld)\n",
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
		printf("o:");
		//searchfile_printdest(opin);
		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(opin->destchip);
			printf("\n");
		}
		else
		{
			printf("	%-s@%08llx	%08llx\n",
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

	f = funcindex_read(offset);
	printf("func@%08x	@%llx\n", offset, (u64)f);

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
	if(ipin != 0)printf("i:");
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(ipin->selfchip);
			printf("\n");
		}

		temp = ipin->samepinnextchip;
		if(temp == 0)break;

		ipin = connect_read(temp);
		if(ipin == 0)break;
	}

	//output(many)
	while(opin != 0)
	{
		printf("o:");
		//searchfunc_printdest(opin);
		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(opin->destchip);
			printf("\n");
		}
		else if(opin->desttype == hex32('f','i','l','e'))
		{
			printf("	file@%08llx	", opin->destchip);
			filename(opin->destchip);
			printf(":%lld\n", opin->destfoot);
		}
		else
		{
			printf("	func@%08llx	%08llx\n",
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

	haha = strhash_generate(buf, len);
	h = strhash_read(haha);
	if(h == 0)
	{
		printf("\nnotfound: (%llx)%s\n", haha, buf);
		return;
	}
	printf("hash: %08x%08x @0x%08llx(%s)\n",
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
	while(ipin != 0)
	{
		if(ipin->selftype == hex32('h','a','s','h'))
		{
			printf("i:	");
			strhash_print(ipin->selfchip);
			printf("\n");
		}
		else if(ipin->selftype == hex32('f','u','n','c'))
		{
			printf("i:	func@%08llx	", ipin->selfchip);
			funcpath(ipin->selfchip);
			printf("\n");
		}
		else if(ipin->selftype == hex32('f','i','l','e'))
		{
			searchfile(ipin->selfchip);
		}
		else if(ipin->selftype == hex32('c','h','i','p'))
		{
			printf("i:	chip@%08llx\n", ipin->selfchip);
		}
		else if(ipin->selftype != 0)
		{
			printf("i:	%llx@%llx\n", ipin->selftype, (u64)ipin);
		}

		temp = ipin->samepinnextchip;
		if(temp == 0)break;

		ipin = connect_read(temp);
		if(ipin == 0)break;
	}

	//output(many)
	while(opin != 0)
	{
		printf("o:");

		if(opin->desttype == hex32('h','a','s','h'))
		{
			printf("	");
			strhash_print(opin->destchip);
			printf("\n");
		}
		else if(opin->desttype == hex32('f','u','n','c'))
		{
			printf("	func@%08llx	", opin->destchip);
			funcname(opin->destchip);
			printf("	");
			funcpath(opin->destchip);
			printf("\n");
		}
		else if(opin->desttype == hex32('f','i','l','e'))
		{
			printf("	file@%08llx	", opin->destchip);
			filename(opin->destchip);
			printf(":%lld\n", opin->destfoot);
		}
		else
		{
			printf("	%llx@%llx\n", ipin->selftype, (u64)ipin);
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
	//chipdata_start(1);
	chipindex_start(1);
	filedata_start(1);
	filemd5_start(1);
	funcdata_start(1);
	funcindex_start(1);
	//pindata_start(1);
	pinindex_start(1);
	strdata_start(1);
	strhash_start(1);
	connect_start(1);

	for(j=1;j<argc;j++)
	{
		len = strlen(argv[j]);
		if(len > 5)
		{
			if(strncmp(argv[j], "pin@", 4)==0)
			{
				hexstr2data(argv[j] + 4, &temp);
				searchpin(temp);
				continue;
			}
			else if(strncmp(argv[j], "chip@", 5)==0)
			{
				hexstr2data(argv[j] + 5, &temp);
				searchchip(temp);
				continue;
			}
			else if(strncmp(argv[j], "file@", 5)==0)
			{
				hexstr2data(argv[j] + 5, &temp);
				searchfile(temp);
				continue;
			}
			else if(strncmp(argv[j], "func@", 5)==0)
			{
				hexstr2data(argv[j] + 5, &temp);
				searchfunc(temp);
				continue;
			}
		}
		searchhash(argv[j], len);
	}
}
