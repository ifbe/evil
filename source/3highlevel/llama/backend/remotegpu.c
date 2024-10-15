#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u64 unsigned long long
u64 time_in_ns();



int remotegpu_exit()
{
	printf("%s\n",__func__);
	return 0;
}
int remotegpu_init(int cnt, const char** ext)
{
	printf("%s\n",__func__);
	return 0;
}




void remotegpu_upload(unsigned short* wbuf, int n, int d, int handle)
{
	printf("%s\n",__func__);
}
void remotegpu_upload2(
	unsigned short* w0, int n0, int d0, int handle0,
	unsigned short* w1, int n1, int d1, int handle1)
{
	printf("%s\n",__func__);
}
void remotegpu_upload3(
	unsigned short* w0, int n0, int d0, int handle0,
	unsigned short* w1, int n1, int d1, int handle1,
	unsigned short* w2, int n2, int d2, int handle2)
{
	printf("%s\n",__func__);
}




void remotegpu_muladd(float* xout, float* xin, __bf16* w, int n, int d, int handle)
{
	printf("%s\n",__func__);
	unsigned long long t0 = time_in_ns();

	//sendhead and command
	//send xin1
	//send xin2
	//send xin3

	unsigned long long t1 = time_in_ns();

	//wait recv

	unsigned long long t2 = time_in_ns();
}
void remotegpu_muladd2(
	float* xout0, float* xin0, __bf16* w0, int n0, int d0, int handle0,
	float* xout1, float* xin1, __bf16* w1, int n1, int d1, int handle1)
{
	printf("%s\n",__func__);
	unsigned long long t0 = time_in_ns();

	//sendhead and command
	//send xin1
	//send xin2
	//send xin3

	unsigned long long t1 = time_in_ns();

	//wait recv

	unsigned long long t2 = time_in_ns();
}
void remotegpu_muladd3(
	float* xout0, float* xin0, __bf16* w0, int n0, int d0, int handle0,
	float* xout1, float* xin1, __bf16* w1, int n1, int d1, int handle1,
	float* xout2, float* xin2, __bf16* w2, int n2, int d2, int handle2)
{
	printf("%s\n",__func__);
	unsigned long long t0 = time_in_ns();

	//sendhead and command
	//send xin1
	//send xin2
	//send xin3

	unsigned long long t1 = time_in_ns();

	//wait recv

	unsigned long long t2 = time_in_ns();
}