#include <stdio.h>
#include <math.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#define _hash_ hex32('h','a','s','h')
#define _file_ hex32('f','i','l','e')
#define _func_ hex32('f','u','n','c')
#define _chip_ hex32('c','h','i','p')
#define _pin_ hex32('p','i','n',0)
#define _shape_ hex32('s','h','a','p')




struct context
{
	u64 type;
	u64 addr;
	u8 str[16];
};
struct pair
{
	u16 parent;
	u16 child;
};
struct vert2d
{
	float x;
	float y;
};
struct vertex
{
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
};




void forcedirected_2d(
	struct vert2d* obuf, int olen,
	struct vert2d* vbuf, int vlen,
	struct pair* lbuf, int llen)
{
	int j,k,m,n;
	float x,y,t;

	//coulomb force
	for(j=0;j<vlen;j++)
	{
		obuf[j].x = obuf[j].y = 0.0;
		for(k=0;k<vlen;k++)
		{
			if(j == k)continue;
			x = vbuf[j].x - vbuf[k].x;
			y = vbuf[j].y - vbuf[k].y;

			//F = (vec/r)*(k*q1*q2)/(r^2)
			//  = vec * (k*q1*q2*)/(r^3)
			t = x*x + y*y;
			t = 100000 / t / sqrt(t);
			x *= t;
			y *= t;

			obuf[j].x += x;
			obuf[j].y += y;
		}
	}

	//spring force
	for(j=0;j<llen;j++)
	{
		m = lbuf[j].parent;
		if(m >= vlen)continue;
		n = lbuf[j].child;
		if(n >= vlen)continue;

		x = vbuf[m].x - vbuf[n].x;
		y = vbuf[m].y - vbuf[n].y;

		//F = (vec/x)*(k*x)
		//  = vec * k
		t = 0.1;
		x *= t;
		y *= t;

		obuf[n].x += x;
		obuf[n].y += y;

		obuf[m].x -= x;
		obuf[m].y -= y;
	}

	//move point
	for(j=0;j<vlen;j++)
	{
		printf("%f,%f -> %f,%f\n",
			vbuf[j].x, vbuf[j].y,
			obuf[j].x, obuf[j].y
		);

		vbuf[j].x += obuf[j].x;
		vbuf[j].y += obuf[j].y;
	}
	//say("\n");
}
void forcedirected_3d(
	struct vertex* obuf, int olen,
	struct vertex* vbuf, int vlen,
	struct pair* lbuf, int llen)
{
	int j,k,m,n;
	float x,y,z,t;

	//coulomb force
	for(j=0;j<vlen;j+=2)
	{
		obuf[j].x = obuf[j].y = obuf[j].z = 0.0;
		for(k=0;k<vlen;k+=2)
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
		m = (lbuf[j].parent)&0xfffe;
		n = (lbuf[j].child)&0xfffe;

		x = vbuf[m].x - vbuf[n].x;
		y = vbuf[m].y - vbuf[n].y;
		z = vbuf[m].z - vbuf[n].z;

		//F = vec*k*r
		t = sqrt(x*x + y*y + z*z);
		x /= t;
		y /= t;
		z /= t;

		obuf[n].x += x;
		obuf[n].y += y;
		obuf[n].z += z;

		obuf[m].x -= x;
		obuf[m].y -= y;
		obuf[m].z -= z;
	}

	//move point
	for(j=0;j<vlen;j+=2)
	{
/*
		printf("%f,%f,%f -> %f,%f,%f\n",
			vbuf[j].x, vbuf[j].y, vbuf[j].z,
			obuf[j].x, obuf[j].y, obuf[j].z
		);
*/
		vbuf[j].x += obuf[j].x / 50.0;
		vbuf[j].y += obuf[j].y / 50.0;
		vbuf[j].z += obuf[j].z / 50.0;
		vbuf[j+1].x = vbuf[j].x;
		vbuf[j+1].y = vbuf[j].y;
		vbuf[j+1].z = vbuf[j].z;
	}
	//say("\n");
}
