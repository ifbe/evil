#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evil.h"
u64 strhash_generate(void*, int);
int strhash_export(u64, void*, int);
void strhash_print(u64);
void* strhash_read(u64);
//
void* pin_read(int);
void* chip_read(int);
void* funcindex_read(int);
void* filemd5_read(int);
void* pointindex_read(int);
void* shapeindex_read(int);
//
void* samedstprevsrc(void*);
void* samedstnextsrc(void*);
void* samesrcprevdst(void*);
void* samesrcnextdst(void*);
void* relation_read(int);
//
void readthemall(int);
void writethemall(int);
int input(void*, int);
int output(void*, int);
int fixarg(void*, void*);
int hexstr2data(void*, void*);




static u8* sb = 0;
static int sl = 0;




void chipname(u64 addr)
{
	struct relation* orel;
	struct chipindex* chip;

	chip = chip_read(addr);
	if(chip == 0)goto error;

	orel = relation_read(chip->orel0);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			break;
		}

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?chipname?");
}
void filename(u64 addr)
{
	struct relation* orel;
	struct fileindex* file;

	file = filemd5_read(addr);
	if(file == 0)goto error;

	orel = relation_read(file->orel0);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			break;
		}

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?filename?");
}
void funcname(u64 addr)
{
	struct relation* orel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	orel = relation_read(func->orel0);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			//strhash_print(orel->destchip);
			sb += strhash_export(orel->destchip, sb, 99);
			break;
		}

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf(sb, sl, "?funcname?");
}
void funcpath(u64 addr)
{
	struct relation* orel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	orel = relation_read(func->orel0);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('f','i','l','e'))
		{
			filename(orel->destchip);
			sb += snprintf(sb, sl, ":%lld", orel->destfoot);
		}

		orel = samesrcnextdst(orel);
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
	struct relation* irel;
	struct relation* orel;
	if(offset%0x20 != 0)
	{
		sb += snprintf(sb, sl,
			"notfound: pin@%x", offset
		);
		return;
	}

	pin = pin_read(offset);
	sb += snprintf(sb, sl, "pin@%08x	@%llx\n", offset, (u64)pin);

pinirel:
	irel = relation_read(pin->irel0);
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

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

pinorel:
	orel = relation_read(pin->orel0);
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

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
}




void searchchip(int offset)
{
	struct relation* irel;
	struct relation* orel;
	struct chipindex* chip;
	if(offset%0x20 != 0)
	{
		sb += snprintf(sb, sl,
			"notfound: chip@%x", offset
		);
		return;
	}

	chip = chip_read(offset);
	sb += snprintf(sb, sl,
		"chip@%08x	@%llx\n",
		offset,
		(u64)chip
	);

chipirel:
	irel = relation_read(chip->irel0);
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

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

chiporel:
	orel = relation_read(chip->orel0);
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

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
}




void searchshape(int offset)
{
	struct shapeindex* shape;
	struct shapeindex* ss;
	struct pointindex* pp;
	struct relation* irel;
	struct relation* orel;
	if(offset%0x20 != 0)
	{
		sb += snprintf(sb, sl,
			"notfound: shape@%x", offset
		);
		return;
	}

	shape = shapeindex_read(offset);
	sb += snprintf(sb, sl,
		"shape@%08x	@%llx\n",
		offset,
		(u64)shape
	);

shapeirel:
	irel = relation_read(shape->irel0);
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
				pp->pppp
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

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

shapeorel:
	orel = relation_read(shape->orel0);
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

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
}




void searchfile(int offset)
{
	struct fileindex* file;
	struct relation* irel;
	struct relation* orel;
	if(offset%0x20 != 0)
	{
		sb += snprintf(sb, sl,
			"notfound: file@%x",offset
		);
		return;
	}

	file = filemd5_read(offset);
	sb += snprintf(sb, sl,
		"file@%08x	@%llx\n",
		offset,
		(u64)file
	);

fileirel:
	irel = relation_read(file->irel0);
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

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

fileorel:
	orel = relation_read(file->orel0);
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

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
}




void searchfunc(int offset)
{
	struct funcindex* f;
	struct relation* irel;
	struct relation* orel;
	if(offset%0x20 != 0)
	{
		sb += snprintf(sb, sl,
			"notfound: func@%x", offset
		);
		return;
	}

	f = funcindex_read(offset);
	sb += snprintf(sb, sl,
		"func@%08x	@%llx\n",
		offset,
		(u64)f
	);

funcirel:
	irel = relation_read(f->irel0);
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

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

funcorel:
	orel = relation_read(f->orel0);
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

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}
}




void searchhash(char* buf, int len)
{
	u64 temp;
	u64 special = 0;
	struct hash* h;
	struct relation* irel;
	struct relation* orel;

	temp = strhash_generate(buf, len);
	h = strhash_read(temp);
	if(h == 0)
	{
		sb += snprintf(sb, sl, "notfound: %.*s\n", len, buf);
		return;
	}
	sb += snprintf(sb, sl,
		"hash: %08x%08x(%.*s)\n",
		h->hash1, h->hash0,
		len, buf
	);

hashirel:
	irel = relation_read(h->irel0);
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

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

hashorel:
	orel = relation_read(h->orel0);
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
				"o:	%.8s@%llx\n",
				(char*)&orel->desttype,
				orel->destchip
			);
		}

		orel = samesrcnextdst(orel);
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
	u8* ibuf;
	u8* obuf;

	ibuf = malloc(0x1000);
	obuf = malloc(0x100000);
	readthemall(1);

	if(argc > 1)
	{
#if (defined(_WIN32) || defined(__WIN32__))
		for(j=1;j<argc;j++)
		{
			fixarg(ibuf, argv[j]);
			search_one(obuf, 0x100000, ibuf, strlen(ibuf));
			output(obuf, sb-obuf);
		}
#else
		for(j=1;j<argc;j++)
		{
			len = strlen(argv[j]);
			search_one(obuf, 0x100000, argv[j], len);
			output(obuf, sb-obuf);
		}
#endif
		return;
	}

	while(1)
	{
		printf("->");
		len = input(ibuf, 0x1000);
		if((ibuf[0] == 'q')&&(ibuf[1] == 0))break;

		j = search_one(obuf, 0x100000, ibuf, len);
		output(obuf, j);
	}
}
