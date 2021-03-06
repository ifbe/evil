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




static u8* sb = 0;
static int sl = 0;




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
			sb += strhash_export(irel->srcchip, sb, 99);
			break;
		}

		irel = samedstnextsrc(irel);
		if(0 == irel)break;
	}
normal:
	return;
error:
	sb += snprintf((void*)sb, sl, "?chipname?");
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
			sb += strhash_export(irel->srcchip, sb, 99);
			break;
		}

		irel = samesrcnextdst(irel);
		if(0 == irel)break;
	}
normal:
	return;
error:
	sb += snprintf((void*)sb, sl, "?filename?");
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
			sb += strhash_export(irel->srcchip, sb, 99);
			break;
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf((void*)sb, sl, "?funcname?");
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
			sb += snprintf((void*)sb, sl, ":%lld", irel->srcfoot);
		}

		irel = samedstnextsrc(irel);
		if(irel == 0)break;
	}
normal:
	return;
error:
	sb += snprintf((void*)sb, sl, "?funcpath?");
}




void searchpin(int offset)
{
	struct fileindex* pin;
	struct relation* irel;
	struct relation* orel;
	if(offset%0x20 != 0)
	{
		sb += snprintf((void*)sb, sl, "notfound: pin@%x", offset);
		return;
	}

	pin = pin_read(offset);
	sb += snprintf((void*)sb, sl, "pin@%08x	@%llx\n", offset, (u64)pin);

pinirel:
	irel = relationread(pin->irel0);
	if(irel == 0)goto pinorel;

	//input
	while(irel != 0)
	{
		if(_hash_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl, "i:	");
			//strhash_print(irel->srcchip);
			sb += strhash_export(irel->srcchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_chip_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl, "i:	");
			chipname(irel->srcchip);
			sb += snprintf((void*)sb, sl,
				":%c\n",
				(u32)(irel->srcfoot)
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "o:	");
			//strhash_print(orel->dstchip);
			sb += strhash_export(orel->dstchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_pin_ == orel->dstchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"o:	%c@pin%llx\n",
				(u32)(orel->srcfoot),
				(orel->dstchip)/0x20
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
		sb += snprintf((void*)sb, sl, "notfound: chip@%x", offset);
		return;
	}

	chip = chip_read(offset);
	sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "i:	");
			//strhash_print(irel->srcchip);
			sb += strhash_export(irel->srcchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "o:	");
			//strhash_print(orel->dstchip);
			sb += strhash_export(orel->dstchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_pin_ == orel->dstchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"o:	%c@pin%llx\n",
				(u32)(orel->srcfoot),
				(orel->dstchip)/0x20
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
		sb += snprintf((void*)sb, sl, "notfound: shape@%x", offset);
		return;
	}

	shape = shapeindex_read(offset);
	sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "i:	");
			//strhash_print(irel->srcchip);
			sb += strhash_export(irel->srcchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_shap_ == irel->srcchiptype)
		{
			ss = shapeindex_read(irel->srcchip);
			sb += snprintf((void*)sb, sl,
				"i:	shap@%08llx	%.8s\n",
				irel->srcchip,
				(char*)&(ss->type)
			);
		}
		else if(_poin_ == irel->srcchiptype)
		{
			pp = pointindex_read(irel->srcchip);
			sb += snprintf((void*)sb, sl,
				"i:	poin@%08llx	%llx\n",
				irel->srcchip,
				pp->pppp
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "o:	");
			//strhash_print(orel->dstchip);
			sb += strhash_export(orel->dstchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_shap_ == orel->dstchiptype)
		{
			ss = shapeindex_read(orel->dstchip);
			sb += snprintf((void*)sb, sl,
				"o:	shap@%08llx	%.8s\n",
				orel->dstchip,
				(char*)&(ss->type)
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
		sb += snprintf((void*)sb, sl, "notfound: file@%x",offset);
		return;
	}

	file = filemd5_read(offset);
	sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl,
				"i:	str	:%lld	",
				irel->dstfoot
			);
			//strhash_print(irel->srcchip);
			sb += strhash_export(irel->srcchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_func_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"i:	func@%08llx	:%lld	",
				irel->srcchip,
				irel->dstfoot
			);
			funcname(irel->srcchip);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "o:	");
			//strhash_print(orel->dstchip);
			sb += strhash_export(orel->dstchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
		sb += snprintf((void*)sb, sl, "notfound: func@%x", offset);
		return;
	}

	f = funcindex_read(offset);
	sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "i:	name	");
			//strhash_print(irel->srcchip);

			sb += strhash_export(irel->srcchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_file_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"i:	file@%08llx	",
				irel->srcchip
			);
			filename(irel->srcchip);
			sb += snprintf((void*)sb, sl,
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
				sb += snprintf((void*)sb, sl, "o:	str	");
			}
			else
			{
				sb += snprintf((void*)sb, sl, "o:	func	");
			}
			//strhash_print(orel->dstchip);

			sb += strhash_export(orel->dstchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_file_ == orel->dstchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"o:	file@%08llx	",
				orel->dstchip
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
		sb += snprintf((void*)sb, sl, "notfound: %.*s\n", len, buf);
		return;
	}
	sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "i:	");
			//strhash_print(irel->srcchip);
			sb += strhash_export(irel->srcchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_func_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"i:	func@%08llx	",
				irel->srcchip
			);
			funcpath(irel->srcchip);
			sb += snprintf((void*)sb, sl, "	");

			funcname(irel->srcchip);
			sb += snprintf((void*)sb, sl,
				":%lld	\n",
				irel->srcfoot
			);
		}
		else if(_file_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"i:	file@%08llx\n",
				irel->srcchip
			);
		}
		else if(_chip_ == irel->srcchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"i:	chip@%08llx\n",
				irel->srcchip
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
			sb += snprintf((void*)sb, sl, "o:	");
			//strhash_print(orel->dstchip);
			sb += strhash_export(orel->dstchip, sb, 99);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_func_ == orel->dstchiptype)
		{
			sb += snprintf((void*)sb, sl,
				"o:	func@%08llx	",
				orel->dstchip
			);

			funcpath(orel->dstchip);
			sb += snprintf((void*)sb, sl, "\n");
		}
		else if(_file_ == orel->dstchiptype)
		{
			special = orel->dstchip;
			sb += snprintf((void*)sb, sl,
				"o:	file@%08llx	",
				orel->dstchip
			);
			filename(orel->dstchip);
			sb += snprintf((void*)sb, sl,
				":%lld\n",
				orel->dstfoot
			);
		}
		else
		{
			sb += snprintf((void*)sb, sl,
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
	sb = dbuf;
	sl = dlen;

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
	return sb-dbuf;
}




void search_print(struct halfrel* self, struct halfrel* peer)
{
	if(_hash_ == peer->chiptype){
		sb += snprintf((void*)sb, sl,
			"%llx,%llx,%.4s,%.4s -> %llx,%llx,%.4s,%.4s(",
			self->chip, self->foot, (void*)&self->chiptype, (void*)&self->foottype,
			peer->chip, peer->foot, (void*)&peer->chiptype, (void*)&peer->foottype
		);
		sb += strhash_export(peer->chip, sb, 99);
		sb += snprintf((void*)sb, sl, ")\n");
	}
	else{
		sb += snprintf((void*)sb, sl,
			"%llx,%llx,%.4s,%.4s -> %llx,%llx,%.4s,%.4s\n",
			self->chip, self->foot, (void*)&self->chiptype, (void*)&self->foottype,
			peer->chip, peer->foot, (void*)&peer->chiptype, (void*)&peer->foottype
		);
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

	sb = dbuf;
	sl = dlen;

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
	else{	//hash
		temp = strhash_generate(sbuf, slen);
		sb += snprintf((void*)sb, sl, "hash=%016llx\n", temp);
		item = strhash_read(temp);
	}

	if(0 == item){
		sb += snprintf((void*)sb, sl, "notfound\n");
		return sb-dbuf;
	}

	ret = search_item(item);
	if(0 == ret){
		sb += snprintf((void*)sb, sl, "lonely@%llx\n", (u64)item);
		return sb-dbuf;
	}
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
			search_one(obuf, 0x100000, (void*)argv[j], len);
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
