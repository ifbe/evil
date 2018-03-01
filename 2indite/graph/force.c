#include<math.h>
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
struct vvvvvv
{
	float x;
	float y;
	float z;
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
struct object
{
	u32 vbo;
	u32 len;
	void* buf;
};
void carveshape(struct object* obj, u32 rgb, float x, float y, float z)
{
	int j,k;
	float r = ((rgb>>16)&0xff)/256.0;
	float g = ((rgb>>8)&0xff)/256.0;
	float b = (rgb&0xff)/256.0;
	int ilen = obj[4].len;
	u16* ibuf = obj[4].buf + (6*ilen);
	int vlen = obj[5].len;
	float* vbuf = obj[5].buf + (36*vlen);
	obj[4].len += 8;
	obj[5].len += 6;

	vbuf[ 0] = x-0.04;
	vbuf[ 1] = y;
	vbuf[ 2] = z;
	vbuf[ 3] = r;
	vbuf[ 4] = g;
	vbuf[ 5] = b;
	vbuf[ 6] = -1.0;
	vbuf[ 7] = 0.0;
	vbuf[ 8] = 0.0;

	vbuf[ 9] = x+0.04;
	vbuf[10] = y;
	vbuf[11] = z;
	vbuf[12] = r;
	vbuf[13] = g;
	vbuf[14] = b;
	vbuf[15] = 1.0;
	vbuf[16] = 0.0;
	vbuf[17] = 0.0;

	vbuf[18] = x;
	vbuf[19] = y-0.04;
	vbuf[20] = z;
	vbuf[21] = r;
	vbuf[22] = g;
	vbuf[23] = b;
	vbuf[24] = 0.0;
	vbuf[25] = -1.0;
	vbuf[26] = 0.0;

	vbuf[27] = x;
	vbuf[28] = y+0.04;
	vbuf[29] = z;
	vbuf[30] = r;
	vbuf[31] = g;
	vbuf[32] = b;
	vbuf[33] = 0.0;
	vbuf[34] = 1.0;
	vbuf[35] = 0.0;

	vbuf[36] = x;
	vbuf[37] = y;
	vbuf[38] = z-0.04;
	vbuf[39] = r;
	vbuf[40] = g;
	vbuf[41] = b;
	vbuf[42] = 0.0;
	vbuf[43] = 0.0;
	vbuf[44] = -1.0;

	vbuf[45] = x;
	vbuf[46] = y;
	vbuf[47] = z+0.04;
	vbuf[48] = r;
	vbuf[49] = g;
	vbuf[50] = b;
	vbuf[51] = 0.0;
	vbuf[52] = 0.0;
	vbuf[53] = 1.0;

	ibuf[ 0] = vlen+0;
	ibuf[ 1] = vlen+2;
	ibuf[ 2] = vlen+4;

	ibuf[ 3] = vlen+0;
	ibuf[ 4] = vlen+2;
	ibuf[ 5] = vlen+5;

	ibuf[ 6] = vlen+0;
	ibuf[ 7] = vlen+3;
	ibuf[ 8] = vlen+4;

	ibuf[ 9] = vlen+0;
	ibuf[10] = vlen+3;
	ibuf[11] = vlen+5;

	ibuf[12] = vlen+1;
	ibuf[13] = vlen+2;
	ibuf[14] = vlen+4;

	ibuf[15] = vlen+1;
	ibuf[16] = vlen+2;
	ibuf[17] = vlen+5;

	ibuf[18] = vlen+1;
	ibuf[19] = vlen+3;
	ibuf[20] = vlen+4;

	ibuf[21] = vlen+1;
	ibuf[22] = vlen+3;
	ibuf[23] = vlen+5;
}
void carveascii(
	struct object* obj, u32 rgb,
	float x, float y, float z, u8 ch)
{
	float r = ((rgb>>16)&0xff)/256.0;
	float g = ((rgb>>8)&0xff)/256.0;
	float b = (rgb&0xff)/256.0;
	int ilen = obj[6].len;
	u16* ibuf = obj[6].buf + (6*ilen);
	int vlen = obj[7].len;
	float* vbuf = obj[7].buf + (36*vlen);
	obj[6].len += 2;
	obj[7].len += 4;

	ch -= 0x20;

	vbuf[ 0] = x-0.02;
	vbuf[ 1] = y-0.04;
	vbuf[ 2] = z;
	vbuf[ 3] = r;
	vbuf[ 4] = g;
	vbuf[ 5] = b;
	vbuf[ 6] = (ch&0xf)/15.9;
	vbuf[ 7] = ((ch>>4)+1)/8.0;

	vbuf[ 9] = x+0.02;
	vbuf[10] = y-0.04;
	vbuf[11] = z;
	vbuf[12] = r;
	vbuf[13] = g;
	vbuf[14] = b;
	vbuf[15] = ((ch&0xf)+1)/15.9;
	vbuf[16] = ((ch>>4)+1)/8.0;

	vbuf[18] = x-0.02;
	vbuf[19] = y+0.04;
	vbuf[20] = z;
	vbuf[21] = r;
	vbuf[22] = g;
	vbuf[23] = b;
	vbuf[24] = (ch&0xf)/15.9;
	vbuf[25] = (ch>>4)/8.0;

	vbuf[27] = x+0.02;
	vbuf[28] = y+0.04;
	vbuf[29] = z;
	vbuf[30] = r;
	vbuf[31] = g;
	vbuf[32] = b;
	vbuf[33] = ((ch&0xf)+1)/15.9;
	vbuf[34] = (ch>>4)/8.0;

	ibuf[0] = vlen+0;
	ibuf[1] = vlen+1;
	ibuf[2] = vlen+3;

	ibuf[3] = vlen+0;
	ibuf[4] = vlen+2;
	ibuf[5] = vlen+3;
}
void carvestring(
	struct object* obj, u32 rgb,
	float x, float y, float z,
	u8* buf, int len)
{
	int j;
	for(len=0;len<16;len++)
	{
		if(buf[len] == 0)break;
	}
	for(j=0;j<len;j++)
	{
		carveascii(
			obj, rgb,
			x+(j-(len/2))*0.04, y, z,
			buf[j]
		);
	}
}




void forcedirected_2d(
	struct vvvvvv* obuf, int olen,
	struct vvvvvv* vbuf, int vlen,
	struct pair* lbuf, int llen)
{
	int j,k,m,n;
	float x,y,t;

	//coulomb force
	for(j=0;j<vlen;j+=2)
	{
		obuf[j].x = obuf[j].y = obuf[j].z = 0.0;
		for(k=0;k<vlen;k+=2)
		{
			if(j == k)continue;
			x = vbuf[j].x - vbuf[k].x;
			y = vbuf[j].y - vbuf[k].y;

			//F = (vec/r)*(k*q1*q2)/(r^2)
			//F = vec*(k*q1*q2*)/(r^3)
			t = x*x + y*y;
			t = 0.020 / t / sqrt(t);
			x *= t;
			y *= t;

			obuf[j].x += x;
			obuf[j].y += y;
		}
	}

	//spring force
	for(j=0;j<llen;j++)
	{
		m = (lbuf[j].parent)&0xfffe;
		n = (lbuf[j].child)&0xfffe;

		x = vbuf[m].x - vbuf[n].x;
		y = vbuf[m].y - vbuf[n].y;

		//F = vec*k*r
		t = sqrt(x*x + y*y);
		x /= t;
		y /= t;

		obuf[n].x += x;
		obuf[n].y += y;

		obuf[m].x -= x;
		obuf[m].y -= y;
	}

	//move point
	for(j=0;j<vlen;j+=2)
	{
/*
		say("%f,%f,%f -> %f,%f,%f\n",
			vbuf[j].x, vbuf[j].y, vbuf[j].z,
			obuf[j].x, obuf[j].y, obuf[j].z
		);
*/
		vbuf[j].x += obuf[j].x / 100.0;
		vbuf[j].y += obuf[j].y / 100.0;
		vbuf[j+1].x = vbuf[j].x;
		vbuf[j+1].y = vbuf[j].y;
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
