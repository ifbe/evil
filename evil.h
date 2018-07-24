typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#define hex16(a,b) (a | (b<<8))
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#define _hash_ hex32('h','a','s','h')
#define _file_ hex32('f','i','l','e')
#define _func_ hex32('f','u','n','c')
#define _chip_ hex32('c','h','i','p')
#define _pin_  hex32('p','i','n',0)




struct relation
{
	u64 destchip;
	u64 destfoot;
	u32 desttype;	   //eg: 'hash', 'dir', 'file', 'func'
	u32 destflag;	   //eg: 'bad', 'ok'
	u32 samedstprevsrc;
	u32 samedstnextsrc;

	u64 selfchip;
	u64 selffoot;
	u32 selftype;	   //eg: 'dir', 'file', 'func', 'hash'
	u32 selfflag;	   //eg: 'bad', 'ok'
	u32 samesrcprevdst;
	u32 samesrcnextdst;
};
struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};
struct chipindex
{
	u32 self;
	u32 what;
	u32 type;
	float data;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};
struct funcindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};
struct pinindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};
struct pointindex
{
	u64 self;
	u64 pppp;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};
struct shapeindex
{
	u64 self;
	u64 type;

	u32 irel0;
	u32 ireln;
	u32 orel0;
	u32 oreln;
};




//
void readthemall(int);
void writethemall();
//
void* relationread(u32);
void* relationcreate(void* uc, u64 uf, u64 ut, void* bc, u64 bf, u64 bt);
//
void* samedstprevsrc(struct relation* rel);
void* samedstnextsrc(struct relation* rel);
void* samesrcprevdst(struct relation* rel);
void* samesrcnextdst(struct relation* rel);