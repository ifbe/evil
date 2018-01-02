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
u32 bkdrhash(char* buf, int len);
u32 djb2hash(char* buf, int len);
int strdata_write(char*, int);
void* strdata_read(int);
void bplus_prepare(void*);
void bplus_debug(void*);
void* bplus_search(void*, u64);
void* bplus_insert(void*, void*);





struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
static u8 btnode[0x100000];
static int btlen = 0x100000;




u64 strhash_generate(char* buf, int len)
{
	int j;
	u32 this[2];
	u8* dst;

	if(len <= 8)
	{
		dst = (u8*)this;
		for(j=0;j<len;j++)dst[j] = buf[j];
		for(j=len;j<8;j++)dst[j] = 0;
	}
	else
	{
		this[1] = bkdrhash(buf, len);
		this[0] = djb2hash(buf, len);
	}
	//printf("%08x,%08x: %s\n", this[0], this[1], buf);

	return *(u64*)this;
}
/*
void* strhash_search(void* buf, int len, u64 hash)
{
	u64 xxx;
	int mid, left, right;

	left = 0;
	right = len/0x20;
	while(1)
	{
		//lastone
		if(left >= right)
		{
			return buf + (left*0x20);
		}

		//
		mid = (left+right)/2;
		xxx = *(u64*)(buf + (mid*0x20));
		if(xxx < hash)
		{
			left = mid+1;
		}
		else if(xxx > hash)
		{
			right = mid;
		}
		else
		{
			return buf + (mid*0x20);
		}
	}
}
*/
void strhash_print(u64 hash)
{
	int len;
	char* addr;
	struct hash* h;
//printf("fdas");
	h = bplus_search(btnode, hash);
	if(h == 0)return;

	if((h->len) <= 8)
	{
		addr = (char*)h;
		len = 8;
	}
	else
	{
		addr = strdata_read(h->off);
		len = h->len;
	}
	printf("%.*s", len, addr);
}
int strhash_export(u8* dst, u64* part)
{
/*
	int j,k,ret,len;
	u8* buf;
	u8* addr;
	struct tree* t;
	struct hash* h;
	t = tree_search(*part);
	if(t == 0)return 0;

	ret = 0;
	len = t->cur;
	buf = t->buf;
	for(j=0; j<len; j+=sizeof(struct hash))
	{
		h = (void*)(buf+j);
		//printf("%llx\n", *(u64*)h);

		if((h->len) <= 8)
		{
			addr = (u8*)h;
			k = 8;
		}
		else
		{
			addr = strdata_read(h->off);
			k = h->len;
		}
		ret += snprintf(
			(void*)dst+ret, 0xf0000-ret,
			"%.*s\n", k, addr
		);
	}

	*part = t->end;
	return ret;
*/
}




void* strhash_write(char* buf, int len)
{
	int j;
	u64 hash;
	void* addr;
	struct hash h;

	hash = strhash_generate(buf, len);

	//exist -> return
	addr = bplus_search(btnode, hash);
	if(addr != 0)return addr;

	if(len <= 8)j = 0;
	else j = strdata_write(buf, len);

	h.hash0 = hash & 0xffffffff;
	h.hash1 = hash >> 32;
	h.off = j;
	h.len = len;
	h.first = 0;
	h.last = 0;

	addr = bplus_insert(btnode, &h);
	bplus_debug(btnode);

	return bplus_search(btnode, hash);
}
void* strhash_read(u64 hash)
{
	return bplus_search(btnode, hash);
/*
	void* t;
	struct hash* h;
	t = bplus_search(btnode, hash);
	if(t == 0)return 0;

	h = strhash_search(t+0x20, 0xfe0, hash);
	if(h == 0)return 0;
	if(*(u64*)h != hash)return 0;
	return h;
*/
}
void strhash_start(int type)
{
/*
	int j, fd;
	char name[32];
	void* mem;
	struct tree* t;
	if(type == 0)
	{
		//alloc
		mem = malloc(maxlen);
		if(mem == 0)
		{
			printf("error@malloc: %s\n", name);
			return;
		}
		t = btnode;
		btlen = 1;

		t->begin = 0;
		t->end = 0xffffffffffffffff;
		t->cur = 0;
		t->len = maxlen;
		t->buf = mem;
		return;
	}
*/
	int j, fd;
	char name[32];
	snprintf(name, 32, ".42/str/index");

	if(type == 0)
	{
		bplus_prepare(btnode);
	}
	else
	{
		//open
		fd = open(name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read index
		btlen = read(fd, btnode, sizeof(btnode));
		printf("strhash:	%x\n", btlen);

		//close index
		close(fd);
	}
/*
	//
	for(j=0;j<btlen;j++)
	{
		//open
		snprintf(name, 32, ".42/str/%02x", j);
		fd = open(name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		if(fd <= 0)
		{
			printf("error@open: %s\n", name);
			break;
		}

		//alloc
		mem = malloc(maxlen);
		if(mem == 0)
		{
			printf("error@malloc: %s\n", name);
			break;
		}
		t = &btnode[j];
		t->len = maxlen;
		t->buf = mem;

		//read
		t->cur = read(fd, t->buf, t->len);
		if(t->cur == 0)
		{
			printf("error@read: %s\n", name);
			break;
		}

		//close
		close(fd);
	}
*/
}
void strhash_stop()
{
	int j;
	int fd;
	int ret;
	char name[32];
	struct tree* t;

	//open index
	snprintf(name, 32, ".42/str/index");
	fd = open(name,
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	if(fd <= 0)
	{
		printf("error@open: %s\n", name);
		return;
	}

	//write index
	ret = write(fd, btnode, 0x100000);
	if(ret < 0)
	{
		printf("error@write: %s\n", name);
		return;
	}

	//close index
	close(fd);
/*
	for(j=0;j<btlen;j++)
	{
		//open data
		snprintf(name, 32, ".42/str/%02x", j);
		fd = open(name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		if(fd <= 0)
		{
			printf("error@open: %s\n", name);
			break;
		}

		//write data
		t = &btnode[j];
		ret = write(fd, t->buf, t->cur);
		if(ret < 0)
		{
			printf("error@write: %s\n", name);
			break;
		}

		//close data
		close(fd);
	}
*/
}
void strhash_create()
{
}
void strhash_delete()
{
}
