#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
int hexstr2data(void*, void*);
void strhash_print(u64);
u64 strhash_generate(void*, int);
void* strhash_read(u64);
//
void readall(int);
void* connect_read(int);
void* pin_read(int);
void* chipindex_read(int);
void* funcindex_read(int);
void* filemd5_read(int);
void* point_read(int);
void* shape_read(int);




struct pointindex
{
	u32 self;
	float x;
	float y;
	float z;

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




void chipname(u64 addr)
{
	u64 temp;
	struct wire* orel;
	struct chipindex* chip;

	chip = chipindex_read(addr);
	if(chip == 0)goto error;

	temp = chip->orel;
	if(temp == 0)goto error;

	orel = connect_read(temp);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			strhash_print(orel->destchip);
			break;
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;

		orel = connect_read(temp);
		if(orel == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}
void filename(u64 addr)
{
	u64 temp;
	struct wire* orel;
	struct fileindex* file;

	file = filemd5_read(addr);
	if(file == 0)goto error;

	temp = file->orel;
	if(temp == 0)goto error;

	orel = connect_read(temp);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			strhash_print(orel->destchip);
			break;
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;

		orel = connect_read(temp);
		if(orel == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}
void funcname(u64 addr)
{
	u64 temp;
	struct wire* orel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	temp = func->orel;
	if(temp == 0)goto error;

	orel = connect_read(temp);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			strhash_print(orel->destchip);
			break;
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;

		orel = connect_read(temp);
		if(orel == 0)break;
	}
normal:
	return;
error:
	printf("\n");
}
void funcpath(u64 addr)
{
	u64 temp;
	struct wire* orel;
	struct funcindex* func;

	func = funcindex_read(addr);
	if(func == 0)goto error;

	temp = func->orel;
	if(temp == 0)goto error;

	orel = connect_read(temp);
	if(orel == 0)goto error;

	while(1)
	{
		if(orel->desttype == hex32('f','i','l','e'))
		{
			filename(orel->destchip);
			printf(":%lld", orel->destfoot);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;

		orel = connect_read(temp);
		if(orel == 0)break;
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
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: pin@%x",offset);

	pin = pin_read(offset);
	printf("pin@%08x	@%llx\n", offset, (u64)pin);

pinirel:
	temp = pin->irel;
	if(temp == 0)goto pinorel;

	irel = connect_read(temp);
	if(irel == 0)goto pinorel;

	//input
	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			printf("i:	");
			strhash_print(irel->selfchip);
			printf("\n");
		}
		else if(irel->selftype == hex32('c','h','i','p'))
		{
			printf("i:	");
			chipname(irel->selfchip);
			printf(":%c\n", (u32)(irel->selffoot));
		}
		else
		{
			printf("i:	%-s@%08llx	%08llx	(@%lld)\n",
			(char*)&(irel->selftype),
			irel->selfchip, irel->selffoot, irel->destfoot);
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;
	}

pinorel:
	temp = pin->orel;
	if(temp == 0)return;

	orel = connect_read(temp);
	if(irel == 0)return;

	//output(many)
	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			printf("o:	");
			strhash_print(orel->destchip);
			printf("\n");
		}
		else if(orel->desttype == hex32('p','i','n',0))
		{
			printf("o:	%c@pin%llx\n",
				(u32)(orel->selffoot), (orel->destchip)/0x20);
		}
		else
		{
			printf("o:	%-s@%08llx	%08llx\n",
			(char*)&(orel->desttype),
			orel->destchip, orel->destfoot);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);
	}
}




void searchchip(int offset)
{
	u64 temp;
	struct chipindex* chip;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: chip@%x",offset);

	chip = chipindex_read(offset);
	printf("chip@%08x	@%llx\n", offset, (u64)chip);

chipirel:
	temp = chip->irel;
	if(temp == 0)goto chiporel;

	irel = connect_read(temp);
	if(irel == 0)goto chiporel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			printf("i:	");
			strhash_print(irel->selfchip);
			printf("\n");
		}
		else
		{
			printf("i:	%-s@%08llx	%08llx	(@%lld)\n",
			(char*)&(irel->selftype),
			irel->selfchip, irel->selffoot, irel->destfoot);
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;
	}

chiporel:
	temp = chip->orel;
	if(temp == 0)return;

	orel = connect_read(temp);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			printf("o:	");
			strhash_print(orel->destchip);
			printf("\n");
		}
		else if(orel->desttype == hex32('p','i','n',0))
		{
			printf("o:	%c@pin%llx\n",
				(u32)(orel->selffoot), (orel->destchip)/0x20);
		}
		else
		{
			printf("o:	%-s@%08llx	%08llx\n",
			(char*)&(orel->desttype),
			orel->destchip, orel->destfoot);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);
	}
}




void searchshape(int offset)
{
	u64 temp;
	struct shapeindex* shape;
	struct shapeindex* ss;
	struct pointindex* pp;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: shape@%x",offset);

	shape = shape_read(offset);
	printf("shape@%08x	@%llx\n", offset, (u64)shape);

shapeirel:
	temp = shape->irel;
	if(temp == 0)goto shapeorel;

	irel = connect_read(temp);
	if(irel == 0)goto shapeorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			printf("i:	");
			strhash_print(irel->selfchip);
			printf("\n");
		}
		else if(irel->selftype == hex32('s','h','a','p'))
		{
			ss = shape_read(irel->selfchip);
			printf("i:	shap@%08llx	%.8s\n",
			irel->selfchip, (char*)&(ss->type));
		}
		else if(irel->selftype == hex32('p','o','i','n'))
		{
			pp = point_read(irel->selfchip);
			printf("i:	poin@%08llx	(%f, %f, %f)\n",
			irel->selfchip, pp->x, pp->y, pp->z);
		}
		else
		{
			printf("i:	%-s@%08llx	%08llx\n",
			(char*)&(irel->selftype), irel->selfchip, irel->selffoot);
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;
	}

shapeorel:
	temp = shape->orel;
	if(temp == 0)return;

	orel = connect_read(temp);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			printf("o:	");
			strhash_print(orel->destchip);
			printf("\n");
		}
		else if(orel->desttype == hex32('s','h','a','p'))
		{
			ss = shape_read(orel->destchip);
			printf("o:	shap@%08llx	%.8s\n",
			orel->destchip, (char*)&(ss->type));
		}
		else
		{
			printf("o:	%-s@%08llx	%08llx\n",
			(char*)&(orel->desttype), orel->destchip, orel->destfoot);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);
	}
}




void searchfile(int offset)
{
	u64 temp;
	struct fileindex* file;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: file@%x",offset);

	file = filemd5_read(offset);
	printf("file@%08x	@%llx\n", offset, (u64)file);

fileirel:
	temp = file->irel;
	if(temp == 0)goto fileorel;

	irel = connect_read(temp);
	if(irel == 0)goto fileorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			printf("i:	str	:%lld	", irel->destfoot);
			strhash_print(irel->selfchip);
			printf("\n");
		}
		else if(irel->selftype == hex32('f','u','n','c'))
		{
			printf("i:	func@%08llx	:%lld	", irel->selfchip, irel->destfoot);
			funcname(irel->selfchip);
			printf("\n");
		}
		else
		{
			printf("i:	%-s@%08llx	%08llx	(@%lld)\n",
			(char*)&(irel->selftype),
			irel->selfchip, irel->selffoot, irel->destfoot);
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;
	}

fileorel:
	temp = file->orel;
	if(temp == 0)return;

	orel = connect_read(temp);
	if(orel == 0)return;

	//output(many)
	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			printf("o:	");
			strhash_print(orel->destchip);
			printf("\n");
		}
		else
		{
			printf("o:	%-s@%08llx	%08llx\n",
			(char*)&(orel->desttype),
			orel->destchip, orel->destfoot);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);
	}
}




void searchfunc(int offset)
{
	u64 temp;
	struct funcindex* f;
	struct wire* irel;
	struct wire* orel;
	if(offset%0x20 != 0)printf("notfound: func@%x",offset);

	f = funcindex_read(offset);
	printf("func@%08x	@%llx\n", offset, (u64)f);

funcirel:
	temp = f->irel;
	if(temp == 0)goto funcorel;

	irel = connect_read(temp);
	if(irel == 0)goto funcorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			if(irel->selfflag == hex32('s','t','r',0))
			{
				printf("i:	str	:%lld	", irel->destfoot);
			}
			else
			{
				printf("i:	func	:%lld	", irel->destfoot);
			}
			strhash_print(irel->selfchip);
			printf("\n");
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;
	}

funcorel:
	temp = f->orel;
	if(temp == 0)return;

	orel = connect_read(temp);
	if(orel == 0)return;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			printf("o:	");
			strhash_print(orel->destchip);
			printf("\n");
		}
		else if(orel->desttype == hex32('f','i','l','e'))
		{
			printf("o:	file@%08llx	", orel->destchip);
			filename(orel->destchip);
			printf(":%lld\n", orel->destfoot);
		}
		else
		{
			printf("o:	func@%08llx	%08llx\n",
			orel->destchip, orel->destfoot);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;

		orel = connect_read(temp);
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
	printf("hash: %08x%08x @0x%08llx(%s)\n", h->hash1, h->hash0, (u64)h, buf);

hashirel:
	temp = h->irel;
	if(temp == 0)goto hashorel;

	irel = connect_read(temp);
	if(irel == 0)goto hashorel;

	while(irel != 0)
	{
		if(irel->selftype == hex32('h','a','s','h'))
		{
			printf("i:	");
			strhash_print(irel->selfchip);
			printf("\n");
		}
		else if(irel->selftype == hex32('f','u','n','c'))
		{
			printf("i:	func@%08llx	", irel->selfchip);
			funcpath(irel->selfchip);
			printf("\n");
		}
		else if(irel->selftype == hex32('f','i','l','e'))
		{
			special = irel->selfchip;
			printf("i:	file@%08llx\n", irel->selfchip);
		}
		else if(irel->selftype == hex32('c','h','i','p'))
		{
			printf("i:	chip@%08llx\n", irel->selfchip);
		}
		else
		{
			printf("i:	%.8s@%llx\n", (char*)&irel->selftype, irel->selfchip);
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;
	}

hashorel:
	temp = h->orel;
	if(temp == 0)goto theend;

	orel = connect_read(temp);
	if(orel == 0)goto theend;

	while(orel != 0)
	{
		if(orel->desttype == hex32('h','a','s','h'))
		{
			printf("o:	");
			strhash_print(orel->destchip);
			printf("\n");
		}
		else if(orel->desttype == hex32('f','u','n','c'))
		{
			printf("o:	func@%08llx	", orel->destchip);
			funcpath(orel->destchip);
			printf("	");
			funcname(orel->destchip);
			printf(":%lld	\n", orel->destfoot);
		}
		else if(orel->desttype == hex32('f','i','l','e'))
		{
			printf("o:	file@%08llx	", orel->destchip);
			filename(orel->destchip);
			printf(":%lld\n", orel->destfoot);
		}
		else
		{
			printf("i:	%.8s@%llx\n", (char*)&orel->selftype, orel->selfchip);
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;

		orel = connect_read(temp);
		if(orel == 0)break;
	}

theend:
	//thisis filename, search filename
	if(special != 0)searchfile(special);
}




void search_one(char* buf, int len)
{
	u64 temp;
	if(len > 5)
	{
		if(strncmp(buf, "pin@", 4)==0)
		{
			hexstr2data(buf + 4, &temp);
			searchpin(temp);
			return;
		}
		else if(strncmp(buf, "chip@", 5)==0)
		{
			hexstr2data(buf + 5, &temp);
			searchchip(temp);
			return;
		}
		else if(strncmp(buf, "file@", 5)==0)
		{
			hexstr2data(buf + 5, &temp);
			searchfile(temp);
			return;
		}
		else if(strncmp(buf, "func@", 5)==0)
		{
			hexstr2data(buf + 5, &temp);
			searchfunc(temp);
			return;
		}
		else if(strncmp(buf, "shap@", 5)==0)
		{
			hexstr2data(buf + 5, &temp);
			searchshape(temp);
			return;
		}
	}
	searchhash(buf, len);
}
void search(int argc, char** argv)
{
	int j;
	readall(1);

	for(j=1;j<argc;j++)
	{
		search_one(argv[j], strlen(argv[j]));
		printf("\n");
	}
}
