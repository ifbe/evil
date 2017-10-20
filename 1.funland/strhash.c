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
void strdata_read(int, int);
int strdata_write(char*, int);




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
struct tree
{
	//hash
	u64 begin;
	u64 end;

	//length
	u64 cur;
	u64 len;

	//child
	u64 left;
	u64 right;

	//memory
	u64 type;
	union{
		void* buf;
		u64 pad;
	};
};
static struct tree btnode[0x100];
static int btlen = 0;
//
#define maxlen 0x100000
static u8* hashbuf[256];
static int hashfd[256];
static int hashlen[256];




u32 bkdrhash(char* buf, int len)
{
	int j;
	u32 hash = 0;
	for(j=0;j<len;j++)
	{
		hash = hash * 131 + buf[j];
	}
	return hash;
}
u32 djb2hash(char* buf, int len)
{
	int j;
	u32 hash=5381;
	for(j=0;j<len;j++)
	{
		hash=(hash<<5) + hash + buf[j];
	}
	return hash;
}




void* tree_split(struct tree* orig)
{
	int j;
	int begin;
	u8* src;
	u8* dst;
	void* mem;
	struct tree* this;

	//check
	if(btlen >= 0x100)
	{
		printf("error@btlen\n");
		return 0;
	}

	//alloc
	mem = malloc(0x100000);
	if(mem == 0)
	{
		printf("error@malloc\n");
		return 0;
	}

	//move node
	dst = (void*)btnode + (btlen+1)*sizeof(struct tree) - 1;
	src = dst - sizeof(struct tree);
	this = (void*)orig + sizeof(struct tree);
	btlen++;
	while(1)
	{
		*dst = *src;

		dst--;
		src--;
		if(src < (u8*)this)break;
	}

	//write
	this->len = 0x100000;
	this->buf = mem;

	//move hash
	dst = mem;
	src = orig->buf;
	begin = ((orig->cur)/2)&0xffffffffffffff20;
	for(j=0;j<begin;j++)dst[j] = src[begin+j];

	//change
	this->cur = (orig->cur) - begin;
	this->begin = *(u64*)dst;
	this->end = orig->end;
	//printf("%016llx,%016llx	%llx\n", this->begin, this->end, this->cur);

	orig->cur = begin;
	orig->end = this->begin - 1;
	//printf("%016llx,%016llx	%llx\n", orig->begin, orig->end, orig->cur);
}
void* tree_search(u64 hash)
{
	int j;
	for(j=0;j<btlen;j++)
	{
		if( (hash >= btnode[j].begin) && (hash <= btnode[j].end) )
		{
			return &btnode[j];
		}
	}
	printf("impossiable\n");
	return 0;
}




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
void strhash_print(u64 hash)
{
	struct tree* t;
	struct hash* h;
	t = tree_search(hash);
	if(t == 0)return;

	h = strhash_search(t->buf, t->cur, hash);
	if(h == 0)return;
	if(*(u64*)h != hash)return;

	if((h->len) > 8)
	{
		strdata_read(h->off, h->len);
	}
	else
	{
		printf("%-8.8s\n", (char*)h);
	}
}




void* strhash_write(char* buf, int len)
{
	int j;
	u64 temp;
	u8* src;
	u8* dst;
	struct tree* t;
	struct hash* h;
	temp = strhash_generate(buf, len);

	//which
	t = tree_search(temp);
	if(t == 0)return 0;
	if((t->cur) >= (t->len))
	{
		printf("tree split: %llx,%llx\n", t->cur, t->len);
		tree_split(t);
		t = tree_search(temp);
	}

	//where
	h = strhash_search(t->buf, t->cur, temp);
	if(h == 0)return 0;
	if(*(u64*)h == temp)return h;

	//move 
	dst = (t->buf) + (t->cur) + 0x1f;
	src = dst - 0x20;
	t->cur += 0x20;
	while(1)
	{
		*dst = *src;

		dst--;
		src--;
		if(src < (u8*)h)break;
	}

	//insert string
	if(len <= 8)j = 0;
	else j = strdata_write(buf, len);

	//insert hash
	h->hash0 = temp & 0xffffffff;
	h->hash1 = temp >> 32;
	h->off = j;
	h->len = len;
	h->first = 0;
	h->last = 0;

	//printf("%d	%s\n", j, buf);
	return h;
}
void* strhash_read(u64 hash)
{
	struct tree* t;
	struct hash* h;
	t = tree_search(hash);
	if(t == 0)return 0;

	h = strhash_search(t->buf, t->cur, hash);
	if(h == 0)return 0;
	if(*(u64*)h != hash)return 0;
	return h;
}
void strhash_list()
{
}
void strhash_choose()
{
}
void strhash_start(int type)
{
	int j, fd;
	char name[32];
	void* mem;
	struct tree* t;
	if(type == 0)
	{
		//alloc
		mem = malloc(0x100000);
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
		t->len = 0x100000;
		t->buf = mem;
		return;
	}

	//open index
	snprintf(name, 32, ".42/strhash/index");
	fd = open(name,
		O_CREAT|O_RDWR|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//read index
	j = read(fd, btnode, sizeof(btnode));
	btlen = j / sizeof(struct tree);
	printf("strhash:	%x\n", btlen);

	//close index
	close(fd);

	//
	for(j=0;j<btlen;j++)
	{
		//open
		snprintf(name, 32, ".42/strhash/%02x", j);
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
		mem = malloc(0x100000);
		if(mem == 0)
		{
			printf("error@malloc: %s\n", name);
			break;
		}
		t = &btnode[j];
		t->len = 0x100000;
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
}
void strhash_stop()
{
	int j;
	int fd;
	int ret;
	char name[32];
	struct tree* t;

	//open index
	snprintf(name, 32, ".42/strhash/index");
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
	ret = write(fd, btnode, btlen*sizeof(struct tree));
	if(ret < 0)
	{
		printf("error@write: %s\n", name);
		return;
	}

	//close index
	close(fd);

	for(j=0;j<btlen;j++)
	{
		//open data
		snprintf(name, 32, ".42/strhash/%02x", j);
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
}
void strhash_create()
{
}
void strhash_delete()
{
}
