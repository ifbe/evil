#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
typedef struct hash* hash;




//'dir'		xxxxxxx,	dirname,	filename
//'file',	linenum,	filename,	funcname
//'call',	linenum,	funcname,	callname
//'struct',	linenum,	structname,	elementname
void connect(u64 relation, u64 detail, hash first, hash last)
{
}




void connect_create()
{
}
void connect_delete()
{
}