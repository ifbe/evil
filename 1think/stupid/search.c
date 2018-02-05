#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
u64 strhash_generate(void*, int);
int strhash_export(u64, void*, int);
void strhash_print(u64);
void* strhash_read(u64);
//
void* pin_read(int);
void* chipindex_read(int);
void* funcindex_read(int);
void* filemd5_read(int);
void* pointindex_read(int);
void* shapeindex_read(int);
//
void* samepinprevchip(void*);
void* samepinnextchip(void*);
void* samechipprevpin(void*);
void* samechipnextpin(void*);
void* relation_read(int);
//
void readthemall(int);
int hexstr2data(void*, void*);
int input(void*, int);
int output(void*, int);




struct pointindex
{
	u64 self;
	u64 haha;

	u64 irel;
	u64 orel;
};
struct shapeindex
{
	u64 self;
	u64 type;
	u64 irel;
	u64 orel;
};
struct chipindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct funcindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct wire
{
	u64 destchip;
	u64 destfoot;
	u32 desttype;		//eg: 'hash', 'dir', 'file', 'func'
	u32 destflag;
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selfchip;
	u64 selffoot;
	u32 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u32 selfflag;
	u32 samechipprevpin;
	u32 samechipnextpin;
};
//
static u8* sb = 0;
static int sl = 0;




void chipname(u64 addr)
{
	struct wire* orel;
	struct chipindex* chip;

	chip = chipindex_read(addr);
	if(chip == 0)goto error;

	orel = relation_read(chip->orel);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			break;
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?chipname?");
}
void filename(u64 addr)
{
	struct wire* orel;
	struct fileindex* file;

	file = filemd5_read(addr);
	if(file == 0)goto error;

	orel = relation_read(file->orel);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			break;
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?filename?");
}
void funcname(u64 addr)
{
	struct wire* orel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	orel = relation_read(func->orel);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			break;
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?funcname?");
}
void funcpath(u64 addr)
{
	struct wire* orel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	orel = relation_read(func->orel);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('f','i','l','e'))
		{
			filename(orel->destchip);
			sb += snprintf(sb, sl, ":%lld", orel->destfoot);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?funcpath?");
}




void searchpin(int offset)
{
	struct fileindex* pin;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: pin@%x",offset);

	pin = pin_read(offset);
	sb += snprintf(sb, sl, "pin@%08x	@%llx\n", offset, (u64)pin);

pinirel:
	irel = relation_read(pin->irel);
	if(irel == 0)goto pinorel;

	//input
	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "i:	");
			//strhash_print(irel->selfchip);
			sb += strhash_export(irel->selfchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(irel->selftype == hex32('c','h','i','p'))
		{
			sb += snprintf(sb, sl, "i:	");
			chipname(irel->selfchip);
			sb += snprintf(sb, sl,
				":%c\n",
				(u32)(irel->selffoot)
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"i:	%-s@%08llx	%08llx	(@%lld)\n",
				(char*)&(irel->selftype),
				irel->selfchip,
				irel->selffoot,
				irel->destfoot
			);
		}

		irel = samepinprevchip(irel);
		if(irel == 0)break;
	}

pinorel:
	orel = relation_read(pin->orel);
	if(orel == 0)return;

	//output(many)
	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "o:	");
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(orel->desttype == hex32('p','i','n',0))
		{
			sb += snprintf(sb, sl,
				"o:	%c@pin%llx\n",
				(u32)(orel->selffoot),
				(orel->destchip)/0x20
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->desttype),
				orel->destchip,
				orel->destfoot
			);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
}




void searchchip(int offset)
{
	struct wire* irel;
	struct wire* orel;
	struct chipindex* chip;
	if(offset%0x20 != 0)printf("notfound: chip@%x",offset);

	chip = chipindex_read(offset);
	sb += snprintf(sb, sl,
		"chip@%08x	@%llx\n",
		offset,
		(u64)chip
	);

chipirel:
	irel = relation_read(chip->irel);
	if(irel == 0)goto chiporel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "i:	");
			//strhash_print(irel->selfchip);
			sb += strhash_export(irel->selfchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else
		{
			sb += snprintf(sb, sl,
				"i:	%-s@%08llx	%08llx	(@%lld)\n",
				(char*)&(irel->selftype),
				irel->selfchip,
				irel->selffoot,
				irel->destfoot
			);
		}

		irel = samepinprevchip(irel);
		if(irel == 0)break;
	}

chiporel:
	orel = relation_read(chip->orel);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "o:	");
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(orel->desttype == hex32('p','i','n',0))
		{
			sb += snprintf(sb, sl,
				"o:	%c@pin%llx\n",
				(u32)(orel->selffoot),
				(orel->destchip)/0x20
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->desttype),
				orel->destchip,
				orel->destfoot
			);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
}




void searchshape(int offset)
{
	struct shapeindex* shape;
	struct shapeindex* ss;
	struct pointindex* pp;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: shape@%x",offset);

	shape = shapeindex_read(offset);
	sb += snprintf(sb, sl,
		"shape@%08x	@%llx\n",
		offset,
		(u64)shape
	);

shapeirel:
	irel = relation_read(shape->irel);
	if(irel == 0)goto shapeorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "i:	");
			//strhash_print(irel->selfchip);
			sb += strhash_export(irel->selfchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(irel->selftype == hex32('s','h','a','p'))
		{
			ss = shapeindex_read(irel->selfchip);
			sb += snprintf(sb, sl,
				"i:	shap@%08llx	%.8s\n",
				irel->selfchip,
				(char*)&(ss->type)
			);
		}
		else if(irel->selftype == hex32('p','o','i','n'))
		{
			pp = pointindex_read(irel->selfchip);
			sb += snprintf(sb, sl,
				"i:	poin@%08llx	%llx\n",
				irel->selfchip,
				pp->haha
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"i:	%-s@%08llx	%08llx\n",
				(char*)&(irel->selftype),
				irel->selfchip,
				irel->selffoot
			);
		}

		irel = samepinprevchip(irel);
		if(irel == 0)break;
	}

shapeorel:
	orel = relation_read(shape->orel);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "o:	");
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(orel->desttype == hex32('s','h','a','p'))
		{
			ss = shapeindex_read(orel->destchip);
			sb += snprintf(sb, sl,
				"o:	shap@%08llx	%.8s\n",
				orel->destchip,
				(char*)&(ss->type)
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->desttype),
				orel->destchip,
				orel->destfoot
			);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
}




void searchfile(int offset)
{
	struct fileindex* file;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: file@%x",offset);

	file = filemd5_read(offset);
	sb += snprintf(sb, sl,
		"file@%08x	@%llx\n",
		offset,
		(u64)file
	);

fileirel:
	irel = relation_read(file->irel);
	if(irel == 0)goto fileorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl,
				"i:	str	:%lld	",
				irel->destfoot
			);
			//strhash_print(irel->selfchip);
			sb += strhash_export(irel->selfchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(irel->selftype == hex32('f','u','n','c'))
		{
			sb += snprintf(sb, sl,
				"i:	func@%08llx	:%lld	",
				irel->selfchip,
				irel->destfoot
			);
			funcname(irel->selfchip);
			sb += snprintf(sb, sl, "\n");
		}
		else
		{
			sb += snprintf(sb, sl,
				"i:	%-s@%08llx	%08llx	(@%lld)\n",
				(char*)&(irel->selftype),
				irel->selfchip,
				irel->selffoot,
				irel->destfoot
			);
		}

		irel = samepinprevchip(irel);
		if(irel == 0)break;
	}

fileorel:
	orel = relation_read(file->orel);
	if(orel == 0)return;

	//output(many)
	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "o:	");
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else
		{
			sb += snprintf(sb, sl,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->desttype),
				orel->destchip,
				orel->destfoot
			);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
}




void searchfunc(int offset)
{
	struct funcindex* f;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: func@%x",offset);

	f = funcindex_read(offset);
	sb += snprintf(sb, sl,
		"func@%08x	@%llx\n",
		offset,
		(u64)f
	);

funcirel:
	irel = relation_read(f->irel);
	if(irel == 0)goto funcorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			if(irel->selfflag == hex32('s','t','r',0))
			{
				sb += snprintf(sb, sl,
					"i:	str	:%lld	",
					irel->destfoot
				);
			}
			else
			{
				sb += snprintf(sb, sl,
					"i:	func	:%lld	",
					irel->destfoot
				);
			}
			//strhash_print(irel->selfchip);
			sb += strhash_export(irel->selfchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}

		irel = samepinprevchip(irel);
		if(irel == 0)break;
	}

funcorel:
	orel = relation_read(f->orel);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "o:	");
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(orel->desttype == hex32('f','i','l','e'))
		{
			sb += snprintf(sb, sl,
				"o:	file@%08llx	",
				orel->destchip
			);
			filename(orel->destchip);
			sb += snprintf(sb, sl,
				":%lld\n",
				orel->destfoot
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"o:	func@%08llx	%08llx\n",
				orel->destchip,
				orel->destfoot
			);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}
}




void searchhash(char* buf, int len)
{
	u64 temp;
	u64 special = 0;
	struct hash* h;
	struct wire* irel;
	struct wire* orel;

	temp = strhash_generate(buf, len);
	h = strhash_read(temp);
	if(h == 0)
	{
		printf("notfound: %s\n", buf);
		return;
	}
	sb += snprintf(sb, sl,
		"hash: %08x%08x(%.*s)\n",
		h->hash1, h->hash0,
		len, buf
	);

hashirel:
	irel = relation_read(h->irel);
	if(irel == 0)goto hashorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "i:	");
			//strhash_print(irel->selfchip);
			sb += strhash_export(irel->selfchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(irel->selftype == hex32('f','u','n','c'))
		{
			sb += snprintf(sb, sl,
				"i:	func@%08llx	",
				irel->selfchip
			);
			funcpath(irel->selfchip);
			sb += snprintf(sb, sl, "\n");
		}
		else if(irel->selftype == hex32('f','i','l','e'))
		{
			special = irel->selfchip;
			sb += snprintf(sb, sl,
				"i:	file@%08llx\n",
				irel->selfchip
			);
		}
		else if(irel->selftype == hex32('c','h','i','p'))
		{
			sb += snprintf(sb, sl,
				"i:	chip@%08llx\n",
				irel->selfchip
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"i:	%.8s@%llx\n",
				(char*)&irel->selftype,
				irel->selfchip
			);
		}

		irel = samepinprevchip(irel);
		if(irel == 0)break;
	}

hashorel:
	orel = relation_read(h->orel);
	if(orel == 0)goto theend;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			sb += snprintf(sb, sl, "o:	");
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			sb += snprintf(sb, sl, "\n");
		}
		else if(orel->desttype == hex32('f','u','n','c'))
		{
			sb += snprintf(sb, sl,
				"o:	func@%08llx	",
				orel->destchip
			);

			funcpath(orel->destchip);
			sb += snprintf(sb, sl, "	");

			funcname(orel->destchip);
			sb += snprintf(sb, sl,
				":%lld	\n",
				orel->destfoot
			);
		}
		else if(orel->desttype == hex32('f','i','l','e'))
		{
			sb += snprintf(sb, sl,
				"o:	file@%08llx	",
				orel->destchip
			);
			filename(orel->destchip);
			sb += snprintf(sb, sl,
				":%lld\n",
				orel->destfoot
			);
		}
		else
		{
			sb += snprintf(sb, sl,
				"i:	%.8s@%llx\n",
				(char*)&orel->selftype,
				orel->selfchip
			);
		}

		orel = samechipprevpin(orel);
		if(orel == 0)break;
	}

theend:
	//thisis filename, search filename
	if(special != 0)searchfile(special);
}




int search_one(u8* dbuf, int dlen, u8* sbuf, int slen)
{
	u64 temp;
	sb = dbuf;
	sl = dlen;

	if(slen > 5)
	{
		if(strncmp(sbuf, "pin@", 4)==0)
		{
			hexstr2data(sbuf + 4, &temp);
			searchpin(temp);
			goto byebye;
		}
		else if(strncmp(sbuf, "chip@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchchip(temp);
			goto byebye;
		}
		else if(strncmp(sbuf, "file@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchfile(temp);
			goto byebye;
		}
		else if(strncmp(sbuf, "func@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchfunc(temp);
			goto byebye;
		}
		else if(strncmp(sbuf, "shap@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchshape(temp);
			goto byebye;
		}
	}
	searchhash(sbuf, slen);
byebye:
	return sb-dbuf;
}
void search(int argc, char** argv)
{
	int j,len;
	u8* buf;
	readthemall(1);
	buf = malloc(0x100000);

	if(argc > 1)
	{
		for(j=1;j<argc;j++)
		{
			len = strlen(argv[j]);
			search_one(buf, 0x100000, argv[j], len);
			printf("%.*s\n", sb-buf, buf);
		}
		return;
	}

	while(1)
	{
		printf("->");
		len = input(buf, 0x1000);
		if((buf[0] == 'q')&&(buf[1] == 0))break;

		j = search_one(buf, 0x100000, buf, len);
		printf("%.*s\n", j, buf);
	}
}