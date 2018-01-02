#include<stdio.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define pagesize 0x1000
#define indxstep ((pagesize/0x10) - 2)
#define leafstep ((pagesize/0x20) - 1)




struct bplushead
{
	u64 left:56;
	u8 type;
	u64 right:56;
	u8 len;
	u64 parent:56;
	u8 lock;
	u64 child:56;
	u8 flag;
};
struct indexdata
{
	u64 hash;
	u64 buf;
};
struct bplusindex
{
	struct bplushead head;
	struct indexdata node[indxstep];
};
struct leafdata
{
	u64 hash;
	u64 buf;
	u64 irel;
	u64 orel;
};
struct bplusleaf
{
	struct bplushead head;
	struct leafdata node[leafstep];
};




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




void bplus_indexcopy(struct indexdata* a, struct indexdata* b)
{
	int j;
	u8* src;
	u8* dst;
	if(a == 0)return;
	if(b == 0)return;
	if(a == b)return;

	src = (void*)a;
	dst = (void*)b;
	for(j=0;j<sizeof(struct indexdata);j++)
	{
		dst[j] = src[j];
	}
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
	for(;k>j;k--)
	{
		bplus_indexcopy(&this->node[k-1], &this->node[k]);
	}

	//put
	bplus_indexcopy(data, &this->node[j]);
	this->head.len += 1;
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
	right->head.len = 1;
	right->head.left = bplus_memory2logic(head, left);
	right->head.right = left->head.right;

	left->head.len = 2;
	left->head.right = bplus_memory2logic(head, right);

	temp = bplus_getright(head, &right->head);
	if(temp != 0)temp->head.left = bplus_memory2logic(head, right);


	//step2: inner data
	hash = data->hash;
	for(j=0;j<3;j++)
	{
		if(hash < left->node[j].hash)break;
	}

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
	haha.buf = bplus_memory2logic(head, right);

	if(right->head.type == '?')
	{
		for(j=0;j<right->head.len;j++)
		{
			temp = bplus_getchild(head, right, j);
			if(temp == 0)break;

			temp->head.parent = bplus_memory2logic(head, right);
		}
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

		if(temp->head.len < 3)bplus_indexadd(temp, &haha);
		else bplus_indexsplit(head, temp, &haha);
	}
}




void bplus_leafcopy(struct leafdata* a, struct leafdata* b)
{
	int j;
	u8* src;
	u8* dst;
	if(a == 0)return;
	if(b == 0)return;
	if(a == b)return;

	src = (void*)a;
	dst = (void*)b;
	for(j=0;j<sizeof(struct leafdata);j++)
	{
		dst[j] = src[j];
	}
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
	for(;k>j;k--)
	{
		bplus_leafcopy(&this->node[k-1], &this->node[k]);
	}

	//put
	this->head.len += 1;
	bplus_leafcopy(data, &this->node[j]);
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
	right->head.len = 2;
	right->head.left = bplus_memory2logic(head, left);
	right->head.right = left->head.right;

	left->head.len = 2;
	left->head.right = bplus_memory2logic(head, right);

	temp = bplus_getright(head, &right->head);
	if(temp == 0)head->right = bplus_memory2logic(head, right);
	else temp->head.left = bplus_memory2logic(head, right);


	//step2: inner data
	hash = data->hash;
	for(j=0;j<3;j++)
	{
		if(hash < left->node[j].hash)break;
	}

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

		if(temp->head.len < 3)bplus_indexadd(temp, &haha);
		else bplus_indexsplit(head, temp, &haha);
	}
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
printf("insert:%llx\n", hash);

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
		bplus_leafadd(this, data);
		return this;
	}

	//split insert
	return bplus_leafsplit(head, this, data);
}
void* bplus_destory(struct bplushead* head, u64 hash)
{
	if(head == 0)return 0;
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
		printf("%08x->%08x<-%08x:",
			index->head.left, addr, index->head.right);

		if(index->head.type == '?')
		{
			for(j=0;j<index->head.len;j++)
			{
				printf(" %016llx", index->node[j].hash);
			}
			printf("\n");
		}
		else
		{
			leaf = (void*)index;
			for(j=0;j<leaf->head.len;j++)
			{
				printf(" %016llx", leaf->node[j].hash);
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
		printf("!%04x: ", addr);
		for(j=0;j<leaf->head.len;j++)
		{
			printf(" %016llx", leaf->node[j].hash);
		}
		printf("\n");
		return;
	}

	printf("?%08x: ", addr);
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
