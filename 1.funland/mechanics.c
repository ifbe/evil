#include<stdio.h>
#include<stdlib.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
void* point_read(int);
void* shape_read(int);
void* connect_read(int);




struct pointindex
{
	u32 self;
	float x;
	float y;
	float z;

	u64 irel;
	u64 orel;
};
struct shapeindex
{
	u32 self;
	u32 x;
	u32 y;
	u32 z;

	u64 irel;
	u64 orel;
};
struct wire
{
	u64 destchip;
	u64 destfoot;
	u32 desttype;		//eg: 'hash', 'dir', 'file', 'func'
	u32 destflag;		//eg: 'bad', 'ok'
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selfchip;
	u64 selffoot;
	u32 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u32 selfflag;		//eg: 'bad', 'ok'
	u32 samechipprevpin;
	u32 samechipnextpin;
};
int mesh_position(float* p, struct wire* w)
{
	int t;
	struct pointindex* point;

	t = 0;
	while(1)
	{
		if(w == 0)break;
		if(w->desttype == hex32('p','o','i','n'))
		{
			point = point_read(w->destchip);

			p[t + 0] = point->x;
			p[t + 1] = point->y;
			p[t + 2] = point->z;
			t += 3;
		}

		if(w->samechipnextpin == 0)break;
		w = connect_read(w->samechipnextpin);
	}

	return t;
}




int mesh_point(float* position, float* normal, float* color)
{
	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 0.0f;

	normal[0] = 0.0f;
	normal[1] = 0.0f;
	normal[2] = 1.0f;

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;

	return 1;
}
int mesh_line(float* position, float* normal, float* color)
{
/*
	position[0] = -1.0f;
	position[1] = 0.0f;
	position[2] = 1.0f;
	position[3] = 1.0f;
	position[4] = 0.0f;
	position[5] = 1.0f;
*/
	int j, k, count;
	struct shapeindex* shape;
	struct wire* wire;

	count = 0;
	for(j=0x20;j<0x1000;j+=0x20)
	{
		shape = shape_read(j);
		if(shape == 0)break;
		if(shape->self == 0)break;

		if(shape->orel == 0)continue;
		wire = connect_read(shape->orel);

		k = mesh_position(&position[count*6], wire);
		if(k != 6)continue;

		normal[count*6 + 0] = 0.0f;
		normal[count*6 + 1] = 0.0f;
		normal[count*6 + 2] = 1.0f;
		normal[count*6 + 3] = 0.0f;
		normal[count*6 + 4] = 0.0f;
		normal[count*6 + 5] = 1.0f;

		color[count*6 + 0] = 0.0f;
		color[count*6 + 1] = 0.0f;
		color[count*6 + 2] = 1.0f;
		color[count*6 + 3] = 1.0f;
		color[count*6 + 4] = 0.0f;
		color[count*6 + 5] = 0.0f;

		count++;
	}

	printf("line:%x\n", count);
	return count;
}
int mesh_tri(float* position, float* normal, float* color)
{
/*
	position[0] = 0.0f;
	position[1] = 1.0f;
	position[2] = 2.0f;
	position[3] = -0.866f;
	position[4] = -0.5f;
	position[5] = 2.0f;
	position[6] = 0.866f;
	position[7] = -0.5f;
	position[8] = 2.0f;
*/
	int j, k, count;
	struct shapeindex* shape;
	struct wire* wire;

	count = 0;
	for(j=0x20;j<0x1000;j+=0x20)
	{
		shape = shape_read(j);
		if(shape == 0)break;
		if(shape->self == 0)break;

		if(shape->orel == 0)continue;
		wire = connect_read(shape->orel);

		k = mesh_position(&position[count*9], wire);
		if(k != 9)continue;

		normal[count*9 + 0] = 0.0f;
		normal[count*9 + 1] = 0.0f;
		normal[count*9 + 2] = 1.0f;
		normal[count*9 + 3] = 0.0f;
		normal[count*9 + 4] = 0.0f;
		normal[count*9 + 5] = 1.0f;
		normal[count*9 + 6] = 0.0f;
		normal[count*9 + 7] = 0.0f;
		normal[count*9 + 8] = 1.0f;

		color[count*9 + 0] = 0.0f;
		color[count*9 + 1] = 1.0f;
		color[count*9 + 2] = 0.0f;
		color[count*9 + 3] = 1.0f;
		color[count*9 + 4] = 0.0f;
		color[count*9 + 5] = 0.0f;
		color[count*9 + 6] = 0.0f;
		color[count*9 + 7] = 0.0f;
		color[count*9 + 8] = 1.0f;

		count++;
	}

	printf("tri:%x\n", count);
	return count;
}
int mesh_rect(float* position, float* normal, float* color)
{
/*
	position[0] = -0.707f;
	position[1] = -0.707f;
	position[2] = 3.0f;
	position[3] = 0.707f;
	position[4] = -0.707f;
	position[5] = 3.0f;
	position[6] = 0.707f;
	position[7] = 0.707f;
	position[8] = 3.0f;
	position[9] = -0.707f;
	position[10] = 0.707f;
	position[11] = 3.0f;
*/
	int j, k, count;
	struct shapeindex* shape;
	struct wire* wire;

	count = 0;
	for(j=0x20;j<0x1000;j+=0x20)
	{
		shape = shape_read(j);
		if(shape == 0)break;
		if(shape->self == 0)break;

		if(shape->orel == 0)continue;
		wire = connect_read(shape->orel);

		k = mesh_position(&position[count*12], wire);
		if(k != 12)continue;

		normal[count*12 + 0] = 0.0f;
		normal[count*12 + 1] = 0.0f;
		normal[count*12 + 2] = 1.0f;
		normal[count*12 + 3] = 0.0f;
		normal[count*12 + 4] = 0.0f;
		normal[count*12 + 5] = 1.0f;
		normal[count*12 + 6] = 0.0f;
		normal[count*12 + 7] = 0.0f;
		normal[count*12 + 8] = 1.0f;
		normal[count*12 + 9] = 0.0f;
		normal[count*12 + 10] = 0.0f;
		normal[count*12 + 11] = 1.0f;

		color[count*12 + 0] = 1.0f;
		color[count*12 + 1] = 0.0f;
		color[count*12 + 2] = 1.0f;
		color[count*12 + 3] = 0.0f;
		color[count*12 + 4] = 1.0f;
		color[count*12 + 5] = 1.0f;
		color[count*12 + 6] = 1.0f;
		color[count*12 + 7] = 1.0f;
		color[count*12 + 8] = 0.0f;
		color[count*12 + 9] = 1.0f;
		color[count*12 + 10] = 1.0f;
		color[count*12 + 11] = 1.0f;

		count++;
	}

	printf("rect:%x\n", count);
	return count;
}