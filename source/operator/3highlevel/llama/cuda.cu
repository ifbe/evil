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
#pragma unroll
	for(x=0;x<xdim;x+=4){
		float2 w0w1 = __bfloat1622float2(*(reinterpret_cast<__nv_bfloat162*>(&mat[idx*xdim + x+0])));
		float2 w2w3 = __bfloat1622float2(*(reinterpret_cast<__nv_bfloat162*>(&mat[idx*xdim + x+2])));
		float4 weight = make_float4(w0w1.x, w0w1.y, w2w3.x, w2w3.y);
		float4 xyzw = *(reinterpret_cast<float4*>(&vec[x+0]));
		f += weight.x*xyzw.x + weight.y*xyzw.y + weight.z*xyzw.z + weight.w*xyzw.w;
	}
	out[idx] = f;
}
__global__ void muladd_kernel_transposed(float* out, float* vec, __nv_bfloat16* mat, int xdim, int ydim)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	int x;
	float f = 0.0;
#pragma unroll
	for(x=0;x<xdim;x+=1){
		f +=(float)mat[x*ydim + idx+0] * vec[x+0];
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
void cudamath_bf16transpose(unsigned short* out, unsigned short* in, int w, int h, int offset, int stride)
{
	int x,y;
	for(y=0;y<h;y++){
		for(x=0;x<w;x++)out[stride*x + offset+y] = in[w*y+x];
	}
}


extern "C"{


static int xdim = 16384;
static int ydim = 32000;
//
static int outbyte = ydim * sizeof(float);
static int vecbyte = xdim * sizeof(float);
static int matbyte = xdim * ydim * 2;	//sizeof(float);
//
static float *cpuout = 0;
static float *cpuvec = 0;
static __nv_bfloat16* cpumat[5] = {};
//
static float *gpuout = 0;
static float *gpuvec = 0;
static __nv_bfloat16* gpumat[5] = {};
static int gpumat_filled[5] = {};
//
#define MATRIXCOPY_EARLY 1
#define OPTIMISE_TRANSPOSE 0
#define WQWKWV 0
#define WO 1
#define W1W3 2
#define W2 3
#define LOGITS 4
static cudaEvent_t event[4];
static cudaEvent_t copyevent[4];

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
void cuda_compute(int handle)
{
	u64 time[5];
	time[0] = time_in_ns();

	int tx = 32;
	if(0 == (ydim%128))tx = 128;
	if(0 == (ydim%512))tx = 512;
	dim3 threads = dim3(tx, 1, 1);
	dim3 blocks  = dim3(ydim/tx, 1, 1);

	time[1] = time_in_ns();
	cudaEventRecord(event[0], 0);

	__nv_bfloat16* themat = 0;
	if(32000 == handle){
		themat = gpumat[LOGITS];
		if(gpumat_filled[LOGITS] < 2){
			printf("upload logits to gpumem\n");
			cudaMemcpyAsync(themat, cpumat[LOGITS], matbyte, cudaMemcpyHostToDevice, 0);
			gpumat_filled[LOGITS] = 2;
		}
	}
	else{
		themat = gpumat[handle];
		if(!MATRIXCOPY_EARLY){
			cudaMemcpyAsync(themat, cpumat[handle], matbyte, cudaMemcpyHostToDevice, 0);
		}
		else{
			while(cudaEventQuery(copyevent[handle]) == cudaErrorNotReady);
		}
	}
	cudaMemcpyAsync(gpuvec, cpuvec, vecbyte, cudaMemcpyHostToDevice, 0);

	cudaEventRecord(event[1], 0);

	// asynchronously issue work to the GPU (all to stream 0)
	if(OPTIMISE_TRANSPOSE){
		muladd_kernel_transposed<<<blocks, threads, 0, 0>>>(gpuout, gpuvec, themat, xdim, ydim);
	}
	else{
		muladd_kernel<<<blocks, threads, 0, 0>>>(gpuout, gpuvec, themat, xdim, ydim);
	}

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

	time[4] = time_in_ns();
	//printf("gpu %d %d: %f, %f, %f\n", xdim, ydim, gputime[0]*1e-3, gputime[1]*1e-3, gputime[2]*1e-3);
	//printf("cpu %d %d: %f, %f, %f, %f\n", xdim, ydim, (time[1]-time[0])*1e-9, (time[2]-time[1])*1e-9, (time[3]-time[2])*1e-9, (time[4]-time[3])*1e-9);
}
__declspec(dllexport) void cudamath_upload(unsigned short* wbuf, int n, int d, int handle)
{
	if(!MATRIXCOPY_EARLY)return;

	if(OPTIMISE_TRANSPOSE){
	cudamath_bf16transpose((unsigned short*)cpumat[handle], wbuf, n, d, 0, d);
	}
	else{
	cudamath_bf16copy((unsigned short*)cpumat[handle], wbuf, n*d);
	}
	cudaMemcpyAsync(gpumat[handle], cpumat[handle], n*d*2, cudaMemcpyHostToDevice, 0);
	cudaEventRecord(copyevent[handle], 0);
}
__declspec(dllexport) void cudamath_upload2(
	unsigned short* w0, int n0, int d0, int handle0,
	unsigned short* w1, int n1, int d1, int handle1)
{
	if(!MATRIXCOPY_EARLY)return;

	if(OPTIMISE_TRANSPOSE){
	cudamath_bf16transpose((unsigned short*)&cpumat[handle0][0], w0, n0, d0,  0, d0+d1);
	cudamath_bf16transpose((unsigned short*)&cpumat[handle0][0], w1, n1, d1, d0, d0+d1);
	}
	else{
	cudamath_bf16copy((unsigned short*)&cpumat[handle0][    0], w0, n0*d0);
	cudamath_bf16copy((unsigned short*)&cpumat[handle0][n0*d0], w1, n1*d1);
	}
	cudaMemcpyAsync(gpumat[handle0], cpumat[handle0], n0*(d0+d1)*2, cudaMemcpyHostToDevice, 0);
	cudaEventRecord(copyevent[handle0], 0);
}
__declspec(dllexport) void cudamath_upload3(
	unsigned short* w0, int n0, int d0, int handle0,
	unsigned short* w1, int n1, int d1, int handle1,
	unsigned short* w2, int n2, int d2, int handle2)
{
	if(!MATRIXCOPY_EARLY)return;

	if(OPTIMISE_TRANSPOSE){
	cudamath_bf16transpose((unsigned short*)&cpumat[handle0][0], w0, n0, d0,     0, d0+d1+d2);
	cudamath_bf16transpose((unsigned short*)&cpumat[handle0][0], w1, n1, d1,    d0, d0+d1+d2);
	cudamath_bf16transpose((unsigned short*)&cpumat[handle0][0], w2, n2, d2, d0+d1, d0+d1+d2);
	}
	else{
	cudamath_bf16copy((unsigned short*)&cpumat[handle0][          0], w0, n0*d0);
	cudamath_bf16copy((unsigned short*)&cpumat[handle0][      n0*d0], w1, n1*d1);
	cudamath_bf16copy((unsigned short*)&cpumat[handle0][n0*d0+n1*d1], w2, n2*d2);
	}
	cudaMemcpyAsync(gpumat[handle0], cpumat[handle0], n0*(d0+d1+d2)*2, cudaMemcpyHostToDevice, 0);
	cudaEventRecord(copyevent[handle0], 0);
}
__declspec(dllexport) void cudamath_muladd(float* xout, float* xin, unsigned short* wbuf, int n, int d, int handle)
{
	xdim = n;
	ydim = d;
	outbyte = ydim * sizeof(float);
	vecbyte = xdim * sizeof(float);
	matbyte = xdim * ydim * 2;	//sizeof(float);

	int x;
	for(x=0;x<xdim;x++)cpuvec[x] = xin[x];
	if(32000 == handle){
		if(gpumat_filled[LOGITS] < 1){
			printf("upload logits to cpumem\n");
			cudamath_upload(wbuf, n, d, LOGITS);
			gpumat_filled[LOGITS] = 1;
		}
	}
	else{
		if(!MATRIXCOPY_EARLY){
			cudamath_upload(wbuf, n, d, handle);
		}
	}

	cuda_compute(handle);

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
	float* xout0, float* xin0, unsigned short* w0, int n0, int d0, int handle0,
	float* xout1, float* xin1, unsigned short* w1, int n1, int d1, int handle1)
{
	xdim = n0;
	ydim = d0+d1;
	outbyte = ydim * sizeof(float);
	vecbyte = xdim * sizeof(float);
	matbyte = xdim * ydim * 2;	//sizeof(float);

	int x,y;
	for(x=0;x<n0;x++)cpuvec[x] = xin0[x];
	for(x=0;x<n1;x++)cpuvec[n0+x] = xin1[x];

	if(!MATRIXCOPY_EARLY){
		cudamath_upload2(w0, n0, d0, handle0, w1, n1, d1, handle1);
	}

	cuda_compute(handle0);

	for(y=0;y<d0;y++)xout0[y] = cpuout[y];
	for(y=0;y<d1;y++)xout1[y] = cpuout[d0+y];
}
__declspec(dllexport) void cudamath_muladd3(
	float* xout0, float* xin0, unsigned short* w0, int n0, int d0, int handle0,
	float* xout1, float* xin1, unsigned short* w1, int n1, int d1, int handle1,
	float* xout2, float* xin2, unsigned short* w2, int n2, int d2, int handle2)
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

	if(!MATRIXCOPY_EARLY){
		cudamath_upload3(w0, n0, d0, handle0, w1, n1, d1, handle1, w2, n2, d2, handle2);
	}

	cuda_compute(handle0);

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
	for(int j=0;j<5;j++)cudaMallocHost((void **)&cpumat[j], matbyte);

	// allocate device memory
	cudaMalloc((void **)&gpuout, outbyte);
	cudaMalloc((void **)&gpuvec, vecbyte);
	for(int j=0;j<5;j++)cudaMalloc((void **)&gpumat[j], matbyte);
	//cudaMemset(gpumem, 255, nbytes);

	for(int i=0;i<4;i++)cudaEventCreate(&event[i]);
	for(int i=0;i<4;i++)cudaEventCreate(&copyevent[i]);
	cudaDeviceSynchronize();

	u64 t1 = time_in_ns();
	printf("backend_init costtime: %f\n", (t1-t0)*1e-9);
}
__declspec(dllexport) void cudamath_exit()
{
	u64 t0 = time_in_ns();

	for(int i=0;i<4;i++)cudaEventDestroy(event[i]);
	for(int i=0;i<4;i++)cudaEventDestroy(copyevent[i]);

	for(int j=0;j<5;j++)cudaFree(gpumat[j]);
	cudaFree(gpuvec);
	cudaFree(gpuout);

	for(int j=0;j<5;j++)cudaFreeHost(cpumat[j]);
	cudaFreeHost(cpuvec);
	cudaFreeHost(cpuout);

	//printf("time spent executing by the GPU: %f, %f, %f\n", gputime[0]*1e-3, gputime[1]*1e-3, gputime[2]*1e-3);

	//printf("time spent executing by the CPU: %f, %f\n", (time[1]-time[0])*1e-9, (time[2]-time[1])*1e-9);

	//printf("cycle spent executing by the CPU: %lu\n", counter);

	u64 t1 = time_in_ns();
	printf("backend_exit costtime: %f\n", (t1-t0)*1e-9);
}


}	//extern "C"