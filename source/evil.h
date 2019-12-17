typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#define hex16(a,b) (a | (b<<8))
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
//chiptype
#define _hash_ hex32('h','a','s','h')
#define _file_ hex32('f','i','l','e')
#define _func_ hex32('f','u','n','c')
#define _chip_ hex32('c','h','i','p')
#define _pin_  hex32('p','i','n',0)
#define _shap_ hex32('s','h','a','p')
#define _poin_ hex32('p','o','i','n')
//foottype
#define _subfunc_ hex32('s','u','b','f')
#define  _caller_ hex32('c','a','e','r')
#define     _str_ hex32('s','t','r',0)
#define  _parent_ hex32('m','o','m',0)
#define   _child_ hex32('s','o','n',0)
#define   _metro_ hex32('m','a','p',0)
#define _station_ hex32('s','t','a',0)
#define _owner_ hex32('o','w','n','r')
#define _localstr_ hex32('l','s','t','r')
#define _globalstr_ hex32('g','s','t','r')




struct relation
{
	u64 srcchip;
	u64 srcfoot;
	u32 srcchiptype;	//eg: 'dir', 'file', 'func', 'hash'
	u32 srcfoottype;	//eg: 'todir', 'tofile'
	u32 samesrcprevdst;
	u32 samesrcnextdst;

	u64 dstchip;
	u64 dstfoot;
	u32 dstchiptype;	//eg: 'hash', 'dir', 'file', 'func'
	u32 dstfoottype;	//eg: 'todir', 'tofile'
	u32 samedstprevsrc;
	u32 samedstnextsrc;
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
void* relationcreate(void* sc, u64 sf, u32 sct, u32 sft, void* dc, u64 df, u32 dct, u32 dft);
//
void* samedstprevsrc(struct relation* rel);
void* samedstnextsrc(struct relation* rel);
void* samesrcprevdst(struct relation* rel);
void* samesrcnextdst(struct relation* rel);
