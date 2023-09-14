#include <stdio.h>
#include <cuda_runtime.h>
#include <cuda_bf16.h>

#define u64 unsigned long long
#ifdef _WIN32
#include <windows.h>
u64 time_in_ns()
{
	LARGE_INTEGER count,freq;
	int ret = QueryPerformanceFrequency(&freq);
	if(ret && freq.QuadPart){
		ret = QueryPerformanceCounter(&count);
		//say("count=%lld,freq=%lld,time=%lld\n", count.QuadPart, freq.QuadPart, (u64)count.QuadPart*1000*1000 / (freq.QuadPart/1000));
		if(ret && count.QuadPart)return (u64)count.QuadPart*1000*1000 / (freq.QuadPart/1000);		//without (u64)=overflow, 10^9*count/freq = overflow
	}

	return 1000 * 1000 * GetTickCount64();
}
#elif __APPLE__
#include <mach/mach_time.h>
#define lseek64 lseek
u64 time_in_ns()
{
	return mach_absolute_time();
}
#else
#include <time.h>
u64 time_in_ns()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (u64)t.tv_sec*1000*1000*1000 + t.tv_nsec;
}
#endif

__global__ void muladd_kernel(float* out, float* vec, __nv_bfloat16* mat, int xdim, int ydim)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	int x;
	float f = 0.0;
	for(x=0;x<xdim;x+=4){
		f +=(float)mat[idx*xdim + x+0] * vec[x+0]+
			(float)mat[idx*xdim + x+1] * vec[x+1]+
			(float)mat[idx*xdim + x+2] * vec[x+2]+
			(float)mat[idx*xdim + x+3] * vec[x+3];
	}
	out[idx] = f;
}

void printoutput(float* data, int len)
{
	int y;
	for(y=0;y<1;y++){
		printf("%.1f, %.1f, %.1f, %.1f......%.1f, %.1f\n", data[y*16+0], data[y*16+1], data[y*16+2], data[y*16+3], data[y*16+14], data[y*16+15]);
	}
}
void cudamath_bf16tofloat(unsigned int* out, unsigned short* in, int cnt)
{
	int x;
	for(x=0;x<cnt;x++){
		out[x] = (unsigned int)in[x]<<16;
	}
}
void cudamath_bf16copy(unsigned short* out, unsigned short* in, int cnt)
{
	int x;
	for(x=0;x<cnt;x++)out[x] = in[x];
}


extern "C"{


static int xdim = 16384;
static int ydim = 32000;
static int outbyte = ydim * sizeof(float);
static int vecbyte = xdim * sizeof(float);
static int matbyte = xdim * ydim * 2;	//sizeof(float);
static float *cpuout = 0;
static float *cpuvec = 0;
static __nv_bfloat16 *cpumat = 0;
static float *gpuout = 0;
static float *gpuvec = 0;
static __nv_bfloat16 *gpumat = 0;
void cuda_cpu_compute(float* tmp0, float* tmp1, float* tmp2)
{
	int x,y;
	for(y=0;y<ydim;y++){
		float tmp = 0.0;
		for(x=0;x<xdim;x++){
		tmp += tmp2[y*xdim+x] * tmp1[x];
		}
		tmp0[y] = tmp;
	}
}
void cuda_compute()
{
	u64 time[5];
	time[0] = time_in_ns();

	int tx = 32;
	if(0 == (ydim%128))tx = 128;
	if(0 == (ydim%512))tx = 512;
	dim3 threads = dim3(tx, 1, 1);
	dim3 blocks  = dim3(ydim/tx, 1, 1);

	cudaEvent_t event[4];
	for(int i=0;i<4;i++)cudaEventCreate(&event[i]);

	cudaDeviceSynchronize();

	// asynchronously issue work to the GPU (all to stream 0)
	time[1] = time_in_ns();
	cudaEventRecord(event[0], 0);
	cudaMemcpyAsync(gpuvec, cpuvec, vecbyte, cudaMemcpyHostToDevice, 0);
	cudaMemcpyAsync(gpumat, cpumat, matbyte, cudaMemcpyHostToDevice, 0);
	cudaEventRecord(event[1], 0);
	muladd_kernel<<<blocks, threads, 0, 0>>>(gpuout, gpuvec, gpumat, xdim, ydim);
	cudaEventRecord(event[2], 0);
	cudaMemcpyAsync(cpuout, gpuout, outbyte, cudaMemcpyDeviceToHost, 0);
	cudaEventRecord(event[3], 0);
	time[2] = time_in_ns();

	// have CPU do some work while waiting for stage 1 to finish
	unsigned long int counter=0;
	while (cudaEventQuery(event[3]) == cudaErrorNotReady)
	{
		counter++;
	}
	time[3] = time_in_ns();

	float gputime[3] = {};
	for(int i=0;i<3;i++)cudaEventElapsedTime(&gputime[i], event[i], event[i+1]);

	for(int i=0;i<4;i++)cudaEventDestroy(event[i]);

	time[4] = time_in_ns();
	//printf("gpu %d %d: %f, %f, %f\n", xdim, ydim, gputime[0]*1e-3, gputime[1]*1e-3, gputime[2]*1e-3);
	//printf("cpu %d %d: %f, %f, %f, %f\n", xdim, ydim, (time[1]-time[0])*1e-9, (time[2]-time[1])*1e-9, (time[3]-time[2])*1e-9, (time[4]-time[3])*1e-9);
}
__declspec(dllexport) void cudamath_muladd(float* xout, float* xin, unsigned short* w, int n, int d)
{
	xdim = n;
	ydim = d;
	outbyte = ydim * sizeof(float);
	vecbyte = xdim * sizeof(float);
	matbyte = xdim * ydim * 2;	//sizeof(float);

	int x;
	for(x=0;x<xdim;x++)cpuvec[x] = xin[x];
	//cudamath_bf16tofloat((unsigned int*)cpumat, w, xdim*ydim);
	cudamath_bf16copy((unsigned short*)cpumat, w, xdim*ydim);

	cuda_compute();

	for(x=0;x<ydim;x++)xout[x] = cpuout[x];
/*
	printf("gpu: %d,%d\n",n,d);
	printoutput(xout, 16);

	cuda_cpu_compute(xout, xin, cpumat);
	printf("cpu: %d,%d\n",n,d);
	printoutput(xout, 16);
*/
}
__declspec(dllexport) void cudamath_muladd2(
	float* xout0, float* xin0, unsigned short* w0, int n0, int d0,
	float* xout1, float* xin1, unsigned short* w1, int n1, int d1)
{
	xdim = n0;
	ydim = d0+d1;
	outbyte = ydim * sizeof(float);
	vecbyte = xdim * sizeof(float);
	matbyte = xdim * ydim * 2;	//sizeof(float);

	int x,y;
	for(x=0;x<n0;x++)cpuvec[x] = xin0[x];
	for(x=0;x<n1;x++)cpuvec[n0+x] = xin1[x];
	//cudamath_bf16tofloat((unsigned int*)cpumat, w0, n0*d0);
	//cudamath_bf16tofloat((unsigned int*)&cpumat[n0*d0], w1, n1*d1);
	cudamath_bf16copy((unsigned short*)cpumat, w0, n0*d0);
	cudamath_bf16copy((unsigned short*)&cpumat[n0*d0], w1, n1*d1);

	cuda_compute();

	for(y=0;y<d0;y++)xout0[y] = cpuout[y];
	for(y=0;y<d1;y++)xout1[y] = cpuout[d0+y];
}
__declspec(dllexport) void cudamath_muladd3(
	float* xout0, float* xin0, unsigned short* w0, int n0, int d0,
	float* xout1, float* xin1, unsigned short* w1, int n1, int d1,
	float* xout2, float* xin2, unsigned short* w2, int n2, int d2)
{
	xdim = n0;
	ydim = d0+d1+d2;
	outbyte = ydim * sizeof(float);
	vecbyte = xdim * sizeof(float);
	matbyte = xdim * ydim * 2;	//sizeof(float);

	int x,y;
	for(x=0;x<n0;x++)cpuvec[x] = xin0[x];
	for(x=0;x<n1;x++)cpuvec[n0+x] = xin1[x];
	for(x=0;x<n2;x++)cpuvec[n0+n1+x] = xin2[x];
	//cudamath_bf16tofloat((unsigned int*)cpumat, w0, n0*d0);
	//cudamath_bf16tofloat((unsigned int*)&cpumat[n0*d0], w1, n1*d1);
	//cudamath_bf16tofloat((unsigned int*)&cpumat[n0*d0+n1*d1], w2, n2*d2);
	cudamath_bf16copy((unsigned short*)cpumat, w0, n0*d0);
	cudamath_bf16copy((unsigned short*)&cpumat[n0*d0], w1, n1*d1);
	cudamath_bf16copy((unsigned short*)&cpumat[n0*d0+n1*d1], w2, n2*d2);

	cuda_compute();

	for(y=0;y<d0;y++)xout0[y] = cpuout[y      ];
	for(y=0;y<d1;y++)xout1[y] = cpuout[d0+y   ];
	for(y=0;y<d2;y++)xout2[y] = cpuout[d0+d1+y];
}



__declspec(dllexport) void cudamath_init()
{
	u64 t0 = time_in_ns();
	cudaSetDevice(0);

	// allocate host memory
	cudaMallocHost((void **)&cpuout, outbyte);
	cudaMallocHost((void **)&cpuvec, vecbyte);
	cudaMallocHost((void **)&cpumat, matbyte);

	// allocate device memory
	cudaMalloc((void **)&gpuout, outbyte);
	cudaMalloc((void **)&gpuvec, vecbyte);
	cudaMalloc((void **)&gpumat, matbyte);
	//cudaMemset(gpumem, 255, nbytes);

	u64 t1 = time_in_ns();
	printf("backend_init costtime: %f\n", (t1-t0)*1e-9);
}
__declspec(dllexport) void cudamath_exit()
{
	u64 t0 = time_in_ns();

	cudaFree(gpumat);
	cudaFree(gpuvec);
	cudaFree(gpuout);
	cudaFreeHost(cpumat);
	cudaFreeHost(cpuvec);
	cudaFreeHost(cpuout);

	//printf("time spent executing by the GPU: %f, %f, %f\n", gputime[0]*1e-3, gputime[1]*1e-3, gputime[2]*1e-3);

	//printf("time spent executing by the CPU: %f, %f\n", (time[1]-time[0])*1e-9, (time[2]-time[1])*1e-9);

	//printf("cycle spent executing by the CPU: %lu\n", counter);

	u64 t1 = time_in_ns();
	printf("backend_exit costtime: %f\n", (t1-t0)*1e-9);
}


}	//extern "C"