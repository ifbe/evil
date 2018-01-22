#include<math.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))




struct binfo
{
	u64 vertexcount;
	u64 normalcount;
	u64 colorcount;
	u64 texturecount;
	u64 pointcount;
	u64 linecount;
	u64 tricount;
	u64 rectcount;
};
struct context
{
	u64 type;
	void* addr;
};
struct pair
{
	u16 parent;
	u16 child;
};
struct vertex
{
	float x;
	float y;
	float z;
};




void forcedirected_3d(
	struct vertex* obuf, int olen,
	struct vertex* vbuf, int vlen,
	struct pair* lbuf, int llen)
{
	int j,k;
	float x,y,z,t;

	//coulomb force
	for(j=0;j<vlen;j++)
	{
		obuf[j].x = obuf[j].y = obuf[j].z = 0.0;
		for(k=0;k<vlen;k++)
		{
			if(j == k)continue;
			x = vbuf[j].x - vbuf[k].x;
			y = vbuf[j].y - vbuf[k].y;
			z = vbuf[j].z - vbuf[k].z;

			//F = (vec/r)*(k*q1*q2)/(r^2)
			//F = vec*(k*q1*q2*)/(r^3)
			t = x*x + y*y + z*z;
			t = 0.020 / t / sqrt(t);
			x *= t;
			y *= t;
			z *= t;

			obuf[j].x += x;
			obuf[j].y += y;
			obuf[j].z += z;
		}
	}

	//spring force
	for(j=0;j<llen;j++)
	{
		x = vbuf[lbuf[j].parent].x - vbuf[lbuf[j].child].x;
		y = vbuf[lbuf[j].parent].y - vbuf[lbuf[j].child].y;
		z = vbuf[lbuf[j].parent].z - vbuf[lbuf[j].child].z;

		//F = vec*k*r
		t = sqrt(x*x + y*y + z*z);
		x /= t;
		y /= t;
		z /= t;

		obuf[lbuf[j].child].x += x;
		obuf[lbuf[j].child].y += y;
		obuf[lbuf[j].child].z += z;

		obuf[lbuf[j].parent].x -= x;
		obuf[lbuf[j].parent].y -= y;
		obuf[lbuf[j].parent].z -= z;
	}

	//move point
	for(j=0;j<vlen;j++)
	{
/*
		say("%f,%f,%f -> %f,%f,%f\n",
			vbuf[j].x, vbuf[j].y, vbuf[j].z,
			obuf[j].x, obuf[j].y, obuf[j].z
		);
*/
		vbuf[j].x += obuf[j].x / 50.0;
		vbuf[j].y += obuf[j].y / 50.0;
		vbuf[j].z += obuf[j].z / 50.0;
	}
	//say("\n");
}
void graph_tria(void* buffer, struct binfo* info,
	struct context* ctxbuf, int ctxlen,
	int j, float s)
{
	float r,g,b,x,y,z;
	int vlen = info->vertexcount;
	int tlen = (info->tricount)*3;
	struct vertex* vbuf = buffer;
	struct vertex* nbuf = buffer+0x100000;
	struct vertex* cbuf = buffer+0x200000;
	u16* tbuf = buffer+0x600000;

	info->vertexcount += 6;
	info->tricount += 8;

	x = vbuf[j].x;
	y = vbuf[j].y;
	z = vbuf[j].z;
	if(ctxbuf[j].type == hex32('h','a','s','h'))
	{
		r = 1.0;
		g = 0.0;
		b = 0.0;
	}
	else if(ctxbuf[j].type == hex32('f','i','l','e'))
	{
		r = 0.0;
		g = 1.0;
		b = 0.0;
	}
	else if(ctxbuf[j].type == hex32('f','u','n','c'))
	{
		r = 0.0;
		g = 0.0;
		b = 1.0;
	}
	else
	{
		r = 0.9;
		g = 0.5;
		b = 0.1;
	}

	vbuf[vlen+0].x = x+s;
	vbuf[vlen+0].y = y;
	vbuf[vlen+0].z = z;

	vbuf[vlen+1].x = x-s;
	vbuf[vlen+1].y = y;
	vbuf[vlen+1].z = z;

	vbuf[vlen+2].x = x;
	vbuf[vlen+2].y = y+s;
	vbuf[vlen+2].z = z;

	vbuf[vlen+3].x = x;
	vbuf[vlen+3].y = y-s;
	vbuf[vlen+3].z = z;

	vbuf[vlen+4].x = x;
	vbuf[vlen+4].y = y;
	vbuf[vlen+4].z = z+s;

	vbuf[vlen+5].x = x;
	vbuf[vlen+5].y = y;
	vbuf[vlen+5].z = z-s;


	nbuf[vlen+0].x = 1.0;
	nbuf[vlen+0].y = 0.0;
	nbuf[vlen+0].z = 0.0;

	nbuf[vlen+1].x = -1.0;
	nbuf[vlen+1].y = 0.0;
	nbuf[vlen+1].z = 0.0;

	nbuf[vlen+2].x = 0.0;
	nbuf[vlen+2].y = 1.0;
	nbuf[vlen+2].z = 0.0;

	nbuf[vlen+3].x = 0.0;
	nbuf[vlen+3].y = -1.0;
	nbuf[vlen+3].z = 0.0;

	nbuf[vlen+4].x = 0.0;
	nbuf[vlen+4].y = 0.0;
	nbuf[vlen+4].z = 1.0;

	nbuf[vlen+5].x = 0.0;
	nbuf[vlen+5].y = 0.0;
	nbuf[vlen+5].z = -1.0;


	cbuf[vlen+0].x = r;
	cbuf[vlen+0].y = g;
	cbuf[vlen+0].z = b;

	cbuf[vlen+1].x = r;
	cbuf[vlen+1].y = g;
	cbuf[vlen+1].z = b;

	cbuf[vlen+2].x = r;
	cbuf[vlen+2].y = g;
	cbuf[vlen+2].z = b;

	cbuf[vlen+3].x = r;
	cbuf[vlen+3].y = g;
	cbuf[vlen+3].z = b;

	cbuf[vlen+4].x = r;
	cbuf[vlen+4].y = g;
	cbuf[vlen+4].z = b;

	cbuf[vlen+5].x = r;
	cbuf[vlen+5].y = g;
	cbuf[vlen+5].z = b;


	tbuf[tlen+ 0] = vlen+0;
	tbuf[tlen+ 1] = vlen+2;
	tbuf[tlen+ 2] = vlen+4;

	tbuf[tlen+ 3] = vlen+0;
	tbuf[tlen+ 4] = vlen+2;
	tbuf[tlen+ 5] = vlen+5;

	tbuf[tlen+ 6] = vlen+0;
	tbuf[tlen+ 7] = vlen+3;
	tbuf[tlen+ 8] = vlen+4;

	tbuf[tlen+ 9] = vlen+0;
	tbuf[tlen+10] = vlen+3;
	tbuf[tlen+11] = vlen+5;

	tbuf[tlen+12] = vlen+1;
	tbuf[tlen+13] = vlen+2;
	tbuf[tlen+14] = vlen+4;

	tbuf[tlen+15] = vlen+1;
	tbuf[tlen+16] = vlen+2;
	tbuf[tlen+17] = vlen+5;

	tbuf[tlen+18] = vlen+1;
	tbuf[tlen+19] = vlen+3;
	tbuf[tlen+20] = vlen+4;

	tbuf[tlen+21] = vlen+1;
	tbuf[tlen+22] = vlen+3;
	tbuf[tlen+23] = vlen+5;
}
void graph_hack(void* buffer, struct binfo* info,
	struct context* ctxbuf, int ctxlen)
{
	int j;
	int olen = ctxlen;
	int vlen = ctxlen;
	int llen = info->linecount;
	struct vertex* obuf = buffer+0x700000;
	struct vertex* vbuf = buffer;
	void* lbuf = buffer+0x500000;

	info->vertexcount = ctxlen;
	info->normalcount = ctxlen;
	info->colorcount = ctxlen;
	forcedirected_3d(obuf, olen, vbuf, vlen, lbuf, llen);
	vbuf[0].x = vbuf[0].y = vbuf[0].z = 0.0;

	info->tricount = 0;
	for(j=0;j<ctxlen;j++)
	{
		graph_tria(buffer, info, ctxbuf, ctxlen, j, 0.05);
	}
}
