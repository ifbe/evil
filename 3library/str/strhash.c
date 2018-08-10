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
#define pagesize 0x1000
#define indxkeys ((pagesize/0x10) - 2)
#define leafstep ((pagesize/0x20) - 1)
u32 bkdrhash(char* buf, int len);
u32 djb2hash(char* buf, int len);
int strdata_write(char*, int);
void* strdata_read(int);




struct bplushead
{
	u64 left:56;
	u64 type:8;
	u64 right:56;
	u64 len:8;
	u64 parent:56;
	u64 lock:8;
	u64 child:56;
	u64 flag:8;
};
struct indexdata
{
	u64 hash;
	u64 buf;
};
struct bplusindex
{
	struct bplushead head;
	struct indexdata node[indxkeys];
};
struct leafdata
{
	u64 hash;
	u32 off;
	u32 len;
	u64 irel;
	u64 orel;
};
struct bplusleaf
{
	struct bplushead head;
	struct leafdata node[leafstep];
};
static u8 btnode[0x4000000];
static int btlen = 0x4000000;




u64 bplus_memory2logic(struct bplushead* head, void* addr)
{
	return addr - (void*)head;
}
void* bplus_logic2memory(struct bplushead* head, u64 addr)
{
	return ((void*)head) + addr;
}




void bplus_prepare(struct bplushead* head, int len)
{
	int j;
	char* buf;
	if(head == 0)return;

	buf = (void*)head;
	for(j=0;j<len;j++)buf[j] = 0;

	head->type = '@';
	head->len = 0;
	head->lock = 0;

	head->left = 0;
	head->right = 0;
	head->parent = 0;
	head->child = pagesize;
}
void bplus_alldone(struct bplushead* head)
{
}
void* bplus_grow(struct bplushead* head)
{
	int j;
	if(head == 0)return 0;

	j = head->child;
	head->child += pagesize;

	return bplus_logic2memory(head, j);
}
void* bplus_recycle(struct bplushead* head, struct bplusleaf* this)
{
	if(head == 0)return 0;
	return 0;
}




void* bplus_getleft(struct bplushead* head, struct bplushead* this)
{
	if(head == 0)return 0;
	if(this == 0)return 0;
	if(this->left == 0)return 0;
	return bplus_logic2memory(head, this->left);
}
void* bplus_getright(struct bplushead* head, struct bplushead* this)
{
	if(head == 0)return 0;
	if(this == 0)return 0;
	if(this->right == 0)return 0;
	return bplus_logic2memory(head, this->right);
}
void* bplus_getparent(struct bplushead* head, struct bplushead* this)
{
	if(head == 0)return 0;
	if(this == 0)return 0;
	if(this->parent == 0)return 0;
	return bplus_logic2memory(head, this->parent);
}
void* bplus_getchild(struct bplushead* head, struct bplusindex* this, int j)
{
	if(head == 0)return 0;
	if(this == 0)return 0;
	if('!' == this->head.type)return 0;

	if( (j < 0) | (j >= this->head.len) )
	{
		if(this->head.child == 0)return 0;
		return bplus_logic2memory(head, this->head.child);
	}
	else
	{
		if(this->node[j].buf == 0)return 0;
		return bplus_logic2memory(head, this->node[j].buf);
	}
}
void* bplus_findleaf(struct bplushead* head, u64 hash)
{
	int j,k;
	struct bplusindex* this;

	if(head == 0)return 0;
	this = bplus_getparent(head, head);
	if(this == 0)return 0;

	while(1)
	{
		if(this->head.type == '!')return this;

		k = this->head.len;
		for(j=k-1;j>=0;j--)
		{
			if(hash >= (this->node[j].hash))break;
		}

		this = bplus_getchild(head, this, j);
		if(this == 0)return 0;
	}
	return 0;
}




void bplus_debug_leftright(struct bplushead* head, u64 addr)
{
	int j;
	struct bplusleaf* leaf;
	struct bplusindex* index;
	if(addr == 0)return;

	index = bplus_logic2memory(head, addr);
	while(1)
	{
		printf("%08llx->%08llx<-%08llx:\n",
			index->head.left, (u64)addr, index->head.right);

		if(index->head.type == '?')
		{
			for(j=0;j<index->head.len;j++)
			{
				printf("%016llx ", index->node[j].hash);
			}
			printf("\n");
		}
		else
		{
			leaf = (void*)index;
			for(j=0;j<leaf->head.len;j++)
			{
				printf("%016llx ", leaf->node[j].hash);
			}
			printf("\n");
		}

		addr = index->head.right;
		index = bplus_getright(head, &index->head);
		if(index == 0)break;
	}
}
void bplus_debug_traverse(struct bplushead* head, u64 addr)
{
	int j;
	struct bplusleaf* leaf;
	struct bplusindex* index;
	if(addr == 0)return;

	index = bplus_logic2memory(head, addr);
	if(index->head.type == '!')
	{
		leaf = (void*)index;
		printf("!%04llx: ", (u64)addr);
		for(j=0;j<leaf->head.len;j++)
		{
			printf(" %016llx", leaf->node[j].hash);
		}
		printf("\n");
		return;
	}

	printf("?%08llx: ", (u64)addr);
	for(j=0;j<index->head.len;j++)
	{
		printf(" %016llx", index->node[j].hash);
	}
	printf("\n");

	bplus_debug_traverse(head, index->head.child);

	for(j=0;j<index->head.len;j++)
	{
		printf("%d,%d,%llx\n", index->head.len, j, index->node[j].buf);
		bplus_debug_traverse(head, index->node[j].buf);
	}
}
void bplus_debug(struct bplushead* head)
{
	int depth = 0;
	u64 addr = head->parent;
	struct bplusindex* here = bplus_getparent(head, head);
	while(1)
	{
		if(addr == 0)break;

		printf("depth=%d\n", depth);
		bplus_debug_leftright(head, addr);

		if(here->head.type != '?')break;

		depth++;
		addr = here->head.child;
		here = bplus_getchild(head, here, 10000);
	}
	//bplus_debug_traverse(head, head->parent);
}








void bplus_indexcopy(struct indexdata* a, struct indexdata* b, int len)
{
	int j;
	u8* src;
	u8* dst;
	if(len <= 0)return;
	if(a == 0)return;
	if(b == 0)return;
	if(a == b)return;

	len *= sizeof(struct indexdata);
	src = (void*)a;
	dst = (void*)b;
	for(j=len-1;j>=0;j--)dst[j] = src[j];
}
void* bplus_indexadd(struct bplusindex* this, struct indexdata* data)
{
	int j,k;
	u64 hash;

	//vars
	k = this->head.len;
	hash = data->hash;

	//where
	for(j=0;j<k;j++)
	{
		if(hash < this->node[j].hash)break;
	}
	//printf("j=%d,k=%d,hash=%c\n", j, k, hash);

	//move
	bplus_indexcopy(&this->node[j], &this->node[j+1], k-j);
	bplus_indexcopy(data, &this->node[j], 1);

	this->head.len += 1;
	return &this->node[j];
}
void* bplus_indexsplit(struct bplushead* head,
	struct bplusindex* left, struct indexdata* data)
{
	int j,k;
	u64 hash;
	struct bplusindex* right;
	struct bplusindex* temp;
	struct indexdata haha;


	//step1: left right
	right = bplus_grow(head);
	right->head.type = '?';
	right->head.len = (indxkeys+1)/2;
	right->head.left = bplus_memory2logic(head, left);
	right->head.right = left->head.right;

	left->head.len = (indxkeys/2);
	left->head.right = bplus_memory2logic(head, right);

	temp = bplus_getright(head, &right->head);
	if(temp != 0)temp->head.left = bplus_memory2logic(head, right);


	//step2: inner data
	hash = data->hash;
	for(j=0;j<indxkeys;j++)
	{
		if(hash < left->node[j].hash)break;
	}

	haha.buf = bplus_memory2logic(head, right);
	if(j == (indxkeys+1)/2)
	{
		//3: l(2) >>1>> r(0)
		//4: l(2) >>2>> r(0)
		//5: l(3) >>2>> r(0)
		//6: l(3) >>3>> r(0)
		//7: l(4) >>3>> r(0)
		//8: l(4) >>4>> r(0)
		//9: l(5) >>4>> r(0)
		//a: l(5) >>5>> r(0)
		right->head.child = data->buf;
		haha.hash = data->hash;

		bplus_indexcopy(
			&left->node[(indxkeys+1)/2],
			&right->node[0],
			indxkeys/2
		);
	}
	else if(j > (indxkeys+1)/2)
	{
		//3: 2, l(3) >>j-3>> r(0), l(j) >>3-j>> r(j-2), r(j-3)
		//4: 2, l(3) >>j-3>> r(0), l(j) >>4-j>> r(j-2), r(j-3)
		//5: 3, l(4) >>j-4>> r(0), l(j) >>5-j>> r(j-3), r(j-4)
		//6: 3, l(4) >>j-4>> r(0), l(j) >>6-j>> r(j-3), r(j-4)
		//7: 4, l(5) >>j-5>> r(0), l(j) >>7-j>> r(j-4), r(j-5)
		//8: 4, l(5) >>j-5>> r(0), l(j) >>8-j>> r(j-4), r(j-5)
		//9: 5, l(6) >>j-6>> r(0), l(j) >>9-j>> r(j-5), r(j-6)
		//a: 5, l(6) >>j-6>> r(0), l(j) >>a-j>> r(j-5), r(j-6)
		right->head.child = left->node[(indxkeys+1)/2].buf;
		haha.hash = left->node[(indxkeys+1)/2].hash;

		bplus_indexcopy(
			&left->node[((indxkeys+1)/2)+1],
			&right->node[0],
			j-1 - ((indxkeys+1)/2)
		);
		bplus_indexcopy(
			&left->node[j],
			&right->node[j - ((indxkeys+1)/2)],
			indxkeys-j
		);
		bplus_indexcopy(
			data,
			&right->node[j-1 - ((indxkeys+1)/2)],
			1
		);
	}
	else
	{
		//3: 1, l(2) >>1>> r(0), l(j) >>1-j>> l(j+1), l(j)
		//4: 1, l(2) >>2>> r(0), l(j) >>1-j>> l(j+1), l(j)
		//5: 2, l(3) >>2>> r(0), l(j) >>2-j>> l(j+1), l(j)
		//6: 2, l(3) >>3>> r(0), l(j) >>2-j>> l(j+1), l(j)
		//7: 3, l(4) >>3>> r(0), l(j) >>3-j>> l(j+1), l(j)
		//8: 3, l(4) >>4>> r(0), l(j) >>3-j>> l(j+1), l(j)
		//9: 4, l(5) >>4>> r(0), l(j) >>4-j>> l(j+1), l(j)
		//a: 4, l(5) >>5>> r(0), l(j) >>4-j>> l(j+1), l(j)
		haha.buf = bplus_memory2logic(head, right);
		haha.hash = left->node[(indxkeys-1)/2].hash;
		right->head.child = left->node[(indxkeys-1)/2].buf;

		bplus_indexcopy(
			&left->node[(indxkeys+1)/2],
			&right->node[0],
			indxkeys/2
		);
		bplus_indexcopy(
			&left->node[j],
			&left->node[j+1],
			((indxkeys-1)/2)-j
		);
		bplus_indexcopy(
			data,
			&left->node[j],
			1
		);
	}
/*
	if(j == 0)
	{
		bplus_indexcopy(&left->node[2], &right->node[0]);

		right->head.child = left->node[1].buf;
		haha.hash = left->node[1].hash;

		bplus_indexcopy(&left->node[0], &left->node[1]);
		bplus_indexcopy(data, &left->node[0]);
	}
	else if(j == 1)
	{
		bplus_indexcopy(&left->node[2], &right->node[0]);

		right->head.child = left->node[1].buf;
		haha.hash = left->node[1].hash;

		bplus_indexcopy(data, &left->node[1]);
	}
	else if(j == 2)
	{
		bplus_indexcopy(&left->node[2], &right->node[0]);

		right->head.child = data->buf;
		haha.hash = data->hash;
	}
	else if(j == 3)
	{
		bplus_indexcopy(data, &right->node[0]);

		right->head.child = left->node[2].buf;
		haha.hash = left->node[2].hash;
	}
*/
	for(j=-1;j<right->head.len;j++)
	{
		temp = bplus_getchild(head, right, j);
		if(temp == 0)break;

		temp->head.parent = bplus_memory2logic(head, right);
	}


	//step3: parent
	temp = bplus_getparent(head, &left->head);
	if(temp == 0)
	{
		temp = bplus_grow(head);

		temp->head.type = '?';
		temp->head.len = 1;

		temp->head.left = 0;
		temp->head.right = 0;
		temp->head.parent = 0;
		temp->head.child = bplus_memory2logic(head, left);

		temp->node[0].hash = haha.hash;
		temp->node[0].buf = bplus_memory2logic(head, right);

		head->parent = left->head.parent = right->head.parent = 
			bplus_memory2logic(head, temp);
	}
	else
	{
		right->head.parent = left->head.parent;

		if(temp->head.len < indxkeys)
		{
			bplus_indexadd(temp, &haha);
		}
		else
		{
			bplus_indexsplit(head, temp, &haha);
			//bplus_debug(head);
		}
	}
	return 0;
}




void bplus_leafcopy(struct leafdata* a, struct leafdata* b, int len)
{
	int j;
	u8* src;
	u8* dst;
	if(len <= 0)return;
	if(a == 0)return;
	if(b == 0)return;
	if(a == b)return;

	len *= sizeof(struct leafdata);
	src = (void*)a;
	dst = (void*)b;
	for(j=len-1;j>=0;j--)dst[j] = src[j];
}
void* bplus_leafadd(struct bplusleaf* this, struct leafdata* data)
{
	int j,k;
	u64 hash;

	//vars
	k = this->head.len;
	hash = data->hash;

	//where
	for(j=0;j<k;j++)
	{
		if(hash < this->node[j].hash)break;
	}

	//move
	bplus_leafcopy(&this->node[j], &this->node[j+1], k-j);
	bplus_leafcopy(data, &this->node[j], 1);

	this->head.len += 1;
	return &this->node[j];
}
void* bplus_leafsplit(struct bplushead* head,
	struct bplusleaf* left, struct leafdata* data)
{
	int j,k;
	u64 hash;
	struct bplusindex* temp;
	struct bplusleaf* right;
	struct indexdata haha;


	//step1: left right
	right = bplus_grow(head);
	right->head.type = '!';
	right->head.len = (leafstep/2) + 1;
	right->head.left = bplus_memory2logic(head, left);
	right->head.right = left->head.right;

	left->head.len = (leafstep+1) / 2;
	left->head.right = bplus_memory2logic(head, right);

	temp = bplus_getright(head, &right->head);
	if(temp == 0)head->right = bplus_memory2logic(head, right);
	else temp->head.left = bplus_memory2logic(head, right);


	//step2: inner data
	hash = data->hash;
	for(j=0;j<leafstep;j++)
	{
		if(hash < left->node[j].hash)break;
	}
	if(j <= leafstep/2)
	{
		//3: l(1) >>2>> r(0), l(j) >>1-j>> l(j+1), l(j)
		//4: l(2) >>2>> r(0), l(j) >>2-j>> l(j+1), l(j)
		//5: l(2) >>3>> r(0), l(j) >>2-j>> l(j+1), l(j)
		//6: l(3) >>3>> r(0), l(j) >>3-j>> l(j+1), l(j)
		bplus_leafcopy(
			&left->node[leafstep/2],
			&right->node[0],
			(leafstep+1)/2
		);
		bplus_leafcopy(
			&left->node[j],
			&left->node[j+1],
			(leafstep/2)-j
		);
		bplus_leafcopy(
			data,
			&left->node[j],
			1
		);
	}
	else
	{
		//3: l(2) >>j-2>> r(0), l(j) >>3-j>> r(j-1), r(j-2)
		//4: l(3) >>j-3>> r(0), l(j) >>4-j>> r(j-2), r(j-3)
		//5: l(3) >>j-3>> r(0), l(j) >>5-j>> r(j-2), r(j-3)
		//6: l(4) >>j-4>> r(0), l(j) >>6-j>> r(j-3), r(j-4)
		//7: l(4) >>j-4>> r(0), l(j) >>7-j>> r(j-3), r(j-4)
		//8: l(5) >>j-5>> r(0), l(j) >>8-j>> r(j-4), r(j-5)
		//9: l(5) >>j-5>> r(0), l(j) >>9-j>> r(j-4), r(j-5)
		//a: l(6) >>j-6>> r(0), l(j) >>a-j>> r(j-5), r(j-6)
		bplus_leafcopy(
			&left->node[(leafstep/2)+1],
			&right->node[0],
			j-1 - (leafstep/2)
		);
		bplus_leafcopy(
			&left->node[j],
			&right->node[j - (leafstep/2)],
			leafstep-j
		);
		bplus_leafcopy(
			data,
			&right->node[j-1 - (leafstep/2)],
			1
		);
	}
/*
	if(j == 0)
	{
		bplus_leafcopy(&left->node[1], &right->node[0]);
		bplus_leafcopy(&left->node[2], &right->node[1]);

		bplus_leafcopy(&left->node[0], &left->node[1]);
		bplus_leafcopy(data, &left->node[0]);
	}
	else if(j == 1)
	{
		bplus_leafcopy(&left->node[1], &right->node[0]);
		bplus_leafcopy(&left->node[2], &right->node[1]);

		bplus_leafcopy(data, &left->node[1]);
	}
	else if(j == 2)
	{
		bplus_leafcopy(data, &right->node[0]);
		bplus_leafcopy(&left->node[2], &right->node[1]);
	}
	else if(j == 3)
	{
		bplus_leafcopy(&left->node[2], &right->node[0]);
		bplus_leafcopy(data, &right->node[1]);
	}
*/

	//step3: parent
	temp = bplus_getparent(head, &left->head);
	if(temp == 0)
	{
		temp = bplus_grow(head);

		temp->head.type = '?';
		temp->head.len = 1;

		temp->head.left = 0;
		temp->head.right = 0;
		temp->head.parent = 0;
		temp->head.child = bplus_memory2logic(head, left);

		temp->node[0].hash = right->node[0].hash;
		temp->node[0].buf = bplus_memory2logic(head, right);

		head->parent = left->head.parent = right->head.parent =
			bplus_memory2logic(head, temp);
	}
	else
	{
		right->head.parent = left->head.parent;

		haha.hash = right->node[0].hash;
		haha.buf = bplus_memory2logic(head, right);

		if(temp->head.len < indxkeys)bplus_indexadd(temp, &haha);
		else
		{
			bplus_indexsplit(head, temp, &haha);
			//bplus_debug(head);
		}
	}

	if(j <= leafstep/2)return &left->node[j];
	else return &right->node[j - leafstep/2];
}




void* bplus_search(struct bplushead* head, u64 hash)
{
	int mid, left, right;
	struct bplusleaf* leaf;
//printf("search:%llx\n", hash);

	leaf = bplus_findleaf(head, hash);
	if(leaf == 0)return 0;
//printf("fuck1\n");

	left = 0;
	right = leaf->head.len;
	while(1)
	{
		mid = (left+right)/2;
		if(left >= right)break;

		if(hash > leaf->node[mid].hash)left = mid+1;
		else if(hash < leaf->node[mid].hash)right = mid;
		else break;
	}

//printf("fuck2:%d, %llx\n",mid, leaf->node[mid].hash);
	if(hash != leaf->node[mid].hash)return 0;
	return &leaf->node[mid];
}
void* bplus_change(struct bplushead* head)
{
	return 0;
}
void* bplus_insert(struct bplushead* head, struct leafdata* data)
{
	u64 hash;
	struct bplusleaf* this;
	struct bplusleaf* right;
	struct bplusleaf* parent;
	if(head == 0)return 0;

	hash = data->hash;
//printf("insert:%llx\n", hash);

	//empty?
	this = bplus_getparent(head, head);
	if(this == 0)
	{
		//root
		this = bplus_grow(head);
		head->parent = head->left = head->right =
			bplus_memory2logic(head, this);

		//leaf
		this->head.type = '!';
		this->head.len = 0;
		this->head.left = 0;
		this->head.right = 0;
		this->head.parent = 0;
		this->head.child = 0;

		//first
		return bplus_leafadd(this, data);
	}

	//find leaf
	this = bplus_findleaf(head, hash);
	if(this == 0)return 0;

	//normal insert
	if(this->head.len < leafstep)
	{
		return bplus_leafadd(this, data);
	}

	//split insert
	this = bplus_leafsplit(head, this, data);
	//bplus_debug(head);
	return this;
}
void* bplus_destory(struct bplushead* head, u64 hash)
{
	if(head == 0)return 0;
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
	struct leafdata* h;
//printf("fdas");
	h = bplus_search((void*)btnode, hash);
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
int strhash_export(u64 hash, u8* buf, int len)
{
	int j,k;
	u8* p;
	struct leafdata* h;

	h = bplus_search((void*)btnode, hash);
	if(h == 0)return 0;

	k = h->len;
	if(k <= 8)p = (void*)h;
	else p = strdata_read(h->off);

	if(k >= len)
	{
		for(j=0;j<len;j++)buf[j] = p[j];
	}
	else
	{
		for(j=0;j<k;j++)buf[j] = p[j];
		for(;j<len;j++)buf[j] = 0;
	}
	return k;
}




void* strhash_write(char* buf, int len)
{
	int j;
	u64 hash;
	void* addr;
	struct leafdata h;

	hash = strhash_generate(buf, len);

	addr = bplus_search((void*)btnode, hash);
	if(addr != 0)return addr;

	if(len <= 8)j = 0;
	else j = strdata_write(buf, len);

	h.hash = hash;
	h.off = j;
	h.len = len;
	h.irel = 0;
	h.orel = 0;

	return bplus_insert((void*)btnode, &h);
}
void* strhash_read(u64 hash)
{
	return bplus_search((void*)btnode, hash);
}
void strhash_start(int type)
{
	int j, fd;
	char name[32];
	snprintf(name, 32, ".42/str/index");

	if(type == 0)
	{
		bplus_prepare((void*)btnode, 0x4000000);
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
	ret = write(fd, btnode, 0x4000000);
	if(ret < 0)
	{
		printf("error@write: %s\n", name);
		return;
	}

	//close index
	close(fd);
}
void strhash_create()
{
}
void strhash_delete()
{
}
