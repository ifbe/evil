#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define s8 signed char
#define s16 signed short
#define s32 signed int
#define s64 signed long long
int hexstr2u32(void* str, void* dat);
int disasm_x8664_all(void* buf, int len, u64 rip);




struct object
{
	u16 machine;	//64,86 or 4c,01
	u16 numberofsection;
	u32 timedatestamp;
	u32 pointertosymboltable;
	u32 numberofsymbol;
	u16 sizeofoptionalheader;
	u16 characteristics;
};




int disasm_object(void* pe, int len, char* section)
{
	return 0;
}
int check_object(struct object* obj)
{
	u32 temp;

	//[0,1]:0x8664 or 0x014c
	if(0x8664 != obj->machine)return 0;

	printf("numberofsection=%d\n",obj->numberofsection);
	printf("timedatestamp=%d\n",obj->timedatestamp);

	int ptr = obj->pointertosymboltable;
	u8* sym = (void*)obj+ptr;
	printf("pointertosymboltable=%x, data=%x,%x,%x,%x\n",ptr,sym[0],sym[1],sym[2],sym[3]);
	printf("numberofsymbol=%d\n",obj->numberofsymbol);

	printf("sizeofoptionalheader=%d\n",obj->sizeofoptionalheader);
	printf("characteristics=%d\n",obj->characteristics);
	return 1;
}
