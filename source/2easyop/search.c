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
int input(void*, int);
int output(void*, int);
int fixarg(void*, void*);
int hexstr2data(void*, void*);




static u8* outbuf = 0;
static int outlen = 0;
static int outnow = 0;




void chipname(u64 addr)
{
	struct relation* irel;
	struct chipindex* chip;

	chip = chip_read(addr);
	if(0 == chip)goto error;

	irel = relationread(chip->irel0);
	if(0 == irel)goto error;

	while(1)
	{
		if(_hash_ == irel->srcchiptype)
		{
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			break;
		}

		irel = samedstnextsrc(irel);
		if(0 == irel)break;
	}
normal:
	return;
error:
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "?chipname?");
}
void filename(u64 addr)
{
	struct relation* irel;
	struct fileindex* file;

	file = filemd5_read(addr);
	if(0 == file)goto error;

	irel = relationread(file->irel0);
	if(0 == irel)goto error;

	while(1)
	{
		if(_hash_ == irel->srcchiptype)
		{
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			break;
		}

		irel = samesrcnextdst(irel);
		if(0 == irel)break;
	}
normal:
	return;
error:
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "?filename?");
}
void funcname(u64 addr)
{
	struct relation* irel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	irel = relationread(func->irel0);
	if(irel == 0)goto error;

	while(1)
	{
		if(_hash_ == irel->srcchiptype)
		{
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			break;
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}
normal:
	return;
error:
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "?funcname?");
}
void funcpath(u64 addr)
{
	struct relation* irel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	irel = relationread(func->irel0);
	if(irel == 0)goto error;

	while(1)
	{
		if(_file_ == irel->srcchiptype)
		{
			filename(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, ":%lld", irel->srcfoot);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}
normal:
	return;
error:
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "?funcpath?");
}




void searchpin(int offset)
{
	struct fileindex* pin;
	struct relation* irel;
	struct relation* orel;
	if(offset%0x20 != 0)
	{
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound: pin@%x", offset);
		return;
	}

	pin = pin_read(offset);
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "pin@%08x	@%llx\n", offset, (u64)pin);

pinirel:
	irel = relationread(pin->irel0);
	if(irel == 0)goto pinorel;

	//input
	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "i:	");
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_chip_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "i:	");
			chipname(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				":%c\n",
				(u32)(irel->srcfoot)
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	%-s@%08llx	%08llx	(@%lld)\n",
				(char*)&(irel->srcchiptype),
				irel->srcchip,
				irel->srcfoot,
				irel->dstfoot
			);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

pinorel:
	orel = relationread(pin->orel0);
	if(orel == 0)return;

	//output(many)
	while(orel != 0)
	{
		if(_hash_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	");
			//strhash_print(orel->dstchip);
			outnow += strhash_export(orel->dstchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_pin_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%c@pin%llx\n",
				(u32)(orel->srcfoot),
				(orel->dstchip)/0x20
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->dstchiptype),
				orel->dstchip,
				orel->dstfoot
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
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound: chip@%x", offset);
		return;
	}

	chip = chip_read(offset);
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
		"chip@%08x	@%llx\n",
		offset,
		(u64)chip
	);

chipirel:
	irel = relationread(chip->irel0);
	if(irel == 0)goto chiporel;

	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "i:	");
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	%-s@%08llx	%08llx	(@%lld)\n",
				(char*)&(irel->srcchiptype),
				irel->srcchip,
				irel->srcfoot,
				irel->dstfoot
			);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

chiporel:
	orel = relationread(chip->orel0);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(_hash_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	");
			//strhash_print(orel->dstchip);
			outnow += strhash_export(orel->dstchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_pin_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%c@pin%llx\n",
				(u32)(orel->srcfoot),
				(orel->dstchip)/0x20
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->dstchiptype),
				orel->dstchip,
				orel->dstfoot
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
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound: shape@%x", offset);
		return;
	}

	shape = shapeindex_read(offset);
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
		"shape@%08x	@%llx\n",
		offset,
		(u64)shape
	);

shapeirel:
	irel = relationread(shape->irel0);
	if(irel == 0)goto shapeorel;

	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "i:	");
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_shap_ == irel->srcchiptype)
		{
			ss = shapeindex_read(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	shap@%08llx	%.8s\n",
				irel->srcchip,
				(char*)&(ss->type)
			);
		}
		else if(_poin_ == irel->srcchiptype)
		{
			pp = pointindex_read(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	poin@%08llx	%llx\n",
				irel->srcchip,
				pp->pppp
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	%-s@%08llx	%08llx\n",
				(char*)&(irel->srcchiptype),
				irel->srcchip,
				irel->srcfoot
			);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

shapeorel:
	orel = relationread(shape->orel0);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(_hash_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	");
			//strhash_print(orel->dstchip);
			outnow += strhash_export(orel->dstchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_shap_ == orel->dstchiptype)
		{
			ss = shapeindex_read(orel->dstchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	shap@%08llx	%.8s\n",
				orel->dstchip,
				(char*)&(ss->type)
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->dstchiptype),
				orel->dstchip,
				orel->dstfoot
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
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound: file@%x",offset);
		return;
	}

	file = filemd5_read(offset);
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
		"file@%08x	@%llx\n",
		offset,
		(u64)file
	);

fileirel:
	irel = relationread(file->irel0);
	if(irel == 0)goto fileorel;

	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	str	:%lld	",
				irel->dstfoot
			);
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_func_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	func@%08llx	:%lld	",
				irel->srcchip,
				irel->dstfoot
			);
			funcname(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	%-s@%08llx	%08llx	(@%lld)\n",
				(char*)&(irel->srcchiptype),
				irel->srcchip,
				irel->srcfoot,
				irel->dstfoot
			);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

fileorel:
	orel = relationread(file->orel0);
	if(orel == 0)return;

	//output(many)
	while(orel != 0)
	{
		if(_hash_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	");
			//strhash_print(orel->dstchip);
			outnow += strhash_export(orel->dstchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%-s@%08llx	%08llx\n",
				(char*)&(orel->dstchiptype),
				orel->dstchip,
				orel->dstfoot
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
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound: func@%x", offset);
		return;
	}

	f = funcindex_read(offset);
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
		"func@%08x	@%llx\n",
		offset,
		(u64)f
	);

funcirel:
	irel = relationread(f->irel0);
	if(irel == 0)goto funcorel;

	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "i:	name	");
			//strhash_print(irel->srcchip);

			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_file_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	file@%08llx	",
				irel->srcchip
			);
			filename(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				":%lld\n",
				irel->srcfoot
			);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

funcorel:
	orel = relationread(f->orel0);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(_hash_ == orel->dstchiptype)
		{
			if(hex32('s','t','r',0) == orel->dstfoottype)
			{
				outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	str	");
			}
			else
			{
				outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	func	");
			}
			//strhash_print(orel->dstchip);

			outnow += strhash_export(orel->dstchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_file_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	file@%08llx	",
				orel->dstchip
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	func@%08llx	%08llx\n",
				orel->dstchip,
				orel->dstfoot
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
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound: %.*s\n", len, buf);
		return;
	}
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
		"hash: %08x%08x(%.*s)\n",
		h->hash1, h->hash0,
		len, buf
	);

hashirel:
	irel = relationread(h->irel0);
	if(irel == 0)goto hashorel;

	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "i:	");
			//strhash_print(irel->srcchip);
			outnow += strhash_export(irel->srcchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_func_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	func@%08llx	",
				irel->srcchip
			);
			funcpath(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "	");

			funcname(irel->srcchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				":%lld	\n",
				irel->srcfoot
			);
		}
		else if(_file_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	file@%08llx\n",
				irel->srcchip
			);
		}
		else if(_chip_ == irel->srcchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	chip@%08llx\n",
				irel->srcchip
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"i:	%.8s@%llx\n",
				(char*)&irel->srcchiptype,
				irel->srcchip
			);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}

hashorel:
	orel = relationread(h->orel0);
	if(orel == 0)goto theend;

	while(orel != 0)
	{
		if(_hash_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "o:	");
			//strhash_print(orel->dstchip);
			outnow += strhash_export(orel->dstchip, outbuf+outnow, outlen-outnow);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_func_ == orel->dstchiptype)
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	func@%08llx	",
				orel->dstchip
			);

			funcpath(orel->dstchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
		}
		else if(_file_ == orel->dstchiptype)
		{
			special = orel->dstchip;
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	file@%08llx	",
				orel->dstchip
			);
			filename(orel->dstchip);
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				":%lld\n",
				orel->dstfoot
			);
		}
		else
		{
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
				"o:	%.8s@%llx\n",
				(char*)&orel->dstchiptype,
				orel->dstchip
			);
		}

		orel = samesrcnextdst(orel);
		if(orel == 0)break;
	}

theend:
	//thisis filename, search filename
	if(special != 0)searchfile(special);
}




int search_one_origin(u8* dbuf, int dlen, u8* sbuf, int slen)
{
	u64 temp;
	outbuf = dbuf;
	outlen = dlen;
	outnow = 0;

	if(slen > 5)
	{
		if(strncmp((void*)sbuf, "pin@", 4)==0)
		{
			hexstr2data(sbuf + 4, &temp);
			searchpin(temp);
			goto byebye;
		}
		else if(strncmp((void*)sbuf, "chip@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchchip(temp);
			goto byebye;
		}
		else if(strncmp((void*)sbuf, "file@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchfile(temp);
			goto byebye;
		}
		else if(strncmp((void*)sbuf, "func@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchfunc(temp);
			goto byebye;
		}
		else if(strncmp((void*)sbuf, "shap@", 5)==0)
		{
			hexstr2data(sbuf + 5, &temp);
			searchshape(temp);
			goto byebye;
		}
	}
	searchhash((void*)sbuf, slen);
byebye:
	return outnow;
}




void search_print(struct halfrel* self, struct halfrel* peer)
{
	outnow += snprintf((void*)outbuf+outnow, outlen-outnow,
		"%.4s@%llx,%.4s@%llx -> %.4s@%llx,%.4s@%llx",
		(char*)&self->chiptype, self->chip,
		(char*)&self->foottype, self->foot,
		(char*)&peer->chiptype, peer->chip,
		(char*)&peer->foottype, peer->foot
	);

	if(_hash_ == peer->chiptype){
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "(");
		outnow += strhash_export(peer->chip, outbuf+outnow, outlen-outnow);
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, ")\n");
	}
	else{
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
	}
}
int search_item(struct hash* item)
{
	int cnt = 0;
	struct relation* rel;

	rel = relationread(item->irel0);
	while(1){
		if(0 == rel)break;

		search_print((void*)&rel->dstchip, (void*)&rel->srcchip);
		cnt += 1;

		rel = samedstnextsrc(rel);
	}

	rel = relationread(item->orel0);
	while(1){
		if(0 == rel)break;

		search_print((void*)&rel->srcchip, (void*)&rel->dstchip);
		cnt += 1;

		rel = samesrcnextdst(rel);
	}

	return cnt;
}
int search_one(u8* dbuf, int dlen, u8* sbuf, int slen)
{
	int ret;
	u64 temp;
	struct hash* item = 0;

	outbuf = dbuf;
	outlen = dlen;
	outnow = 0;

	if(strncmp((void*)sbuf, "pin@", 4)==0){
		hexstr2data(sbuf + 4, &temp);
		item = pin_read(temp);
	}
	else if(strncmp((void*)sbuf, "chip@", 5)==0){
		hexstr2data(sbuf + 5, &temp);
		item = chip_read(temp);
	}
	else if(strncmp((void*)sbuf, "file@", 5)==0){
		hexstr2data(sbuf + 5, &temp);
		item = filemd5_read(temp);
	}
	else if(strncmp((void*)sbuf, "func@", 5)==0){
		hexstr2data(sbuf + 5, &temp);
		item = funcindex_read(temp);
	}
	else if(strncmp((void*)sbuf, "shap@", 5)==0){
		hexstr2data(sbuf + 5, &temp);
		item = shapeindex_read(temp);
	}
	else if(strncmp((void*)sbuf, "hash@", 5)==0){
		hexstr2data(sbuf + 5, &temp);
		item = strhash_read(temp);
		if(item){
			//printf("@@@@1111\n");
			outnow += strhash_export(temp, outbuf+outnow, outlen-outnow);
			//printf("@@@@2222\n");
			outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "\n");
			//printf("@@@@3333\n");
		}
	}
	else{	//hash
		temp = strhash_generate(sbuf, slen);
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "hash=%016llx\n", temp);
		item = strhash_read(temp);
	}

	if(0 == item){
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "notfound\n");
		return outnow;
	}

	ret = search_item(item);
	if(0 == ret){
		outnow += snprintf((void*)outbuf+outnow, outlen-outnow, "lonely@%llx\n", (u64)item);
		return outnow;
	}
	return outnow;
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
			output(obuf, outnow);
		}
#else
		for(j=1;j<argc;j++)
		{
			len = strlen(argv[j]);
			search_one(obuf, 0x100000, (void*)argv[j], len);
			output(obuf, outnow);
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
