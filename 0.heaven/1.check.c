#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u32 unsigned int
#define u64 unsigned long long
u32 bkdrhash(char*, int);
u32 djb2hash(char*, int);
u64 stringhash_generate(char*,int);
void* stringhash_read(u64);
struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};




void checkone(char* buf, int len)
{
	struct hash* addr;
	u64 haha;

	haha = stringhash_generate(buf, len);
	addr = stringhash_read(haha);
	if(addr == 0)
	{
		printf("notfound: %s\n", buf);
		return;
	}

	printf("%s\n", buf);
	printf("	(@0x%08x), (=0x%08x%08x)\n", addr, addr->hash1, addr->hash0);
	printf("	(%x,%x)\n", addr->first, addr->last);
}
void check(int argc, char** argv)
{
	int j;
	for(j=1;j<argc;j++)
	{
		checkone( argv[j], strlen(argv[j]) );
	}
}