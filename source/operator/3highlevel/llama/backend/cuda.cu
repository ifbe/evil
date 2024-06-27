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
		//float2 w0w1 = __bfloat1622float2(*(reinterpret_cast<__nv_bfloat162*>(&mat[idx*xdim + x+0])));
		//float2 w2w3 = __bfloat1622float2(*(reinterpret_cast<__nv_bfloat162*>(&mat[idx*xdim + x+2])));
		//float4 weight = make_float4(w0w1.x, w0w1.y, w2w3.x, w2w3.y);
		float4 weight = make_float4(mat[idx*xdim + x+0], mat[idx*xdim + x+1], mat[idx*xdim + x+2], mat[idx*xdim + x+3]);
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
	//int x;
	//for(x=0;x<cnt;x++)out[x] = in[x];
	memcpy(out, in, cnt*2);		//replace naive copy by memcpy: speed up from 0.2 token/s to 0.6 token/s
}
void cudamath_bf16transpose(unsigned short* out, unsigned short* in, int w, int h, int offset, int stride)
{
	int x,y;
	for(y=0;y<h;y++){
		for(x=0;x<w;x++)out[stride*x + offset+y] = in[w*y+x];
	}
}


extern "C"{


#define DEBUG_MALLOC 1
#define MATRIXCOPY_EARLY 1
#define OPTIMISE_TRANSPOSE 0	//not working
#define OPTIMISE_RESIDENTPINMEM_MATRIX 1		//consume gpumem = 12G
#define OPTIMISE_RESIDENTPINMEM_LOGITS 1		//consume pinmem = 4096*32000*2
#define OPTIMISE_RESIDENTGPUMEM_MATRIX 0		//consume gpumem = 12G
#define OPTIMISE_RESIDENTGPUMEM_LOGITS 1		//consume gpumem = 4096*32000*2
//
#define SPECIAL_HANDLE_FOR_LOGITS 32000
//
static cudaStream_t stream[2];
#define QUEUE_KERN 0
#define QUEUE_COPY 1
//
static cudaEvent_t event[5];
static cudaEvent_t copyevent[5];
//
static int xdim = 16384;	//llama2=11008, llama3=14336
static int ydim = 131072;	//llama2=32000, llama3=128256
//
static int outbyte = ydim * sizeof(float);
static int vecbyte = xdim * sizeof(float);
static int matbyte = xdim * ydim * 2;	//sizeof(float);
//
static float *cpuout = 0;
static float *cpuvec = 0;
static __nv_bfloat16* pinmem_logits = 0;
//
static float *gpuout = 0;
static float *gpuvec = 0;
static __nv_bfloat16* gpumem_logits = 0;
//
int gpumem_resident_MB = 0;




				//pinmem is big enough
				#if OPTIMISE_RESIDENTPINMEM_MATRIX==1
static __nv_bfloat16* pinmem[32*4]={};	//llama2 7b: layer=32
#define LAYER_0 0
#define LAYER_1 1
#define LAYER_2 2
__nv_bfloat16* pinmem_get(int handle)
{
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		return pinmem_logits;
	}

	return pinmem[handle];
}
__nv_bfloat16* pinmem_create_or_get(int handle, int size)
{
	cudaError_t ret;
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		if(0 == pinmem_logits){
			ret = cudaMallocHost((void **)&pinmem_logits, size);
			if(DEBUG_MALLOC)printf("pinmem_create_or_get1: ret=%d\n", ret);
		}
		return pinmem_logits;
	}

	if(0 == pinmem[handle]){
		ret = cudaMallocHost((void **)&pinmem[handle], size);
		if(DEBUG_MALLOC)printf("pinmem_create_or_get2: ret=%d\n", ret);
	}
	return pinmem[handle];
}




				//pinmem is insufficient
				#else
static __nv_bfloat16* pinmem[4] = {};	//each layer 4 muladd
#define PINMEM_WQWKWV 0
#define PINMEM_WO     1
#define PINMEM_W1W3   2
#define PINMEM_W2     3

__nv_bfloat16* pinmem_get(int handle)
{
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		return pinmem_logits;
	}

	int k = handle & 0x3;
	return pinmem[k];
}
__nv_bfloat16* pinmem_create_or_get(int handle, int size)
{
	cudaError_t ret;
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		if(0 == pinmem_logits){
			ret = cudaMallocHost((void **)&pinmem_logits, size);
			if(DEBUG_MALLOC)printf("pinmem_create_or_get1: ret=%d\n", ret);
		}
		return pinmem_logits;
	}

	int k = handle & 0x3;
	if(0 == pinmem[k]){
		ret = cudaMallocHost((void **)&pinmem[k], size);
		if(DEBUG_MALLOC)printf("pinmem_create_or_get3: ret=%d\n", ret);
	}
	return pinmem[k];
}
void maybe_start_copy_cpu2pin(int handle)
{
}
				//pinmem macro end
				#endif




				//gpumem is big enough
				#if OPTIMISE_RESIDENTGPUMEM_MATRIX==1
static __nv_bfloat16* gpumem[32*4]={};	//llama2 7b: layer=32

__nv_bfloat16* gpumem_get(int handle)
{
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		return gpumem_logits;
	}

	return gpumem[handle];
}
__nv_bfloat16* gpumem_create_or_get(int handle, int size)
{
	cudaError_t ret;
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		if(0 == gpumem_logits){
			ret = cudaMalloc((void **)&gpumem_logits, size);
			if(DEBUG_MALLOC)printf("gpumem_create_or_get1: ret=%d\n", ret);
		}
		return gpumem_logits;
	}

	if(0 == gpumem[handle]){
		ret = cudaMalloc((void **)&gpumem[handle], size);
		if(DEBUG_MALLOC)printf("gpumem_create_or_get2: ret=%d\n", ret);
	}
	return gpumem[handle];
}

				//gpumem is insufficient
				#else
#define GPUMEM_COUNT_LIMIT 36		//gtx1060 only have 6g gram
static __nv_bfloat16* gpumem[GPUMEM_COUNT_LIMIT] = {};	//each layer 4 muladd
static __nv_bfloat16* gpumem_staging[4];

__nv_bfloat16* gpumem_get(int handle)
{
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		return gpumem_logits;
	}

	if(handle >= GPUMEM_COUNT_LIMIT)return gpumem_staging[handle%4];

	return gpumem[handle];
}
__nv_bfloat16* gpumem_create_or_get(int handle, int size)
{
	cudaError_t ret;
	if(SPECIAL_HANDLE_FOR_LOGITS == handle){
		if(0 == gpumem_logits){
			ret = cudaMalloc((void **)&gpumem_logits, size);
			if(DEBUG_MALLOC)printf("gpumem_create_or_get1: ret=%d\n", ret);
		}
		return gpumem_logits;
	}

	if(handle >= GPUMEM_COUNT_LIMIT){
		int k = handle&3;
		if(0 == gpumem_staging[k]){
			ret = cudaMalloc((void **)&gpumem_staging[k], size);
			if(DEBUG_MALLOC)printf("gpumem_create_or_get2: k=%d, ret=%d\n", k, ret);
		}
		return gpumem_staging[k];
	}

	int k = handle;
	if(0 == gpumem[k]){
		ret = cudaMalloc((void **)&gpumem[k], size);
		if(DEBUG_MALLOC)printf("gpumem_create_or_get3: k=%d, ret=%d\n", k, ret);
	}
	return gpumem[k];
}

struct pendingcopyh2d{
	int size;
}pending_data[32*4] = {};
int gpumem_count_max = 0;

//1.h2d copy will not overlap on 2 stream
//2.h2d copy will not preempt
//3.compute will not begin until all copy are done(including in other stream)
//so the copy must happen right after calling kernel compute
void maybe_start_copy_pin2gpu(int handle)
{
	/*
	0 -> return
	1 -> return
	GPUMEM_COUNT_LIMIT-1 -> return
	GPUMEM_COUNT_LIMIT+0 -> GPUMEM_COUNT_LIMIT+1
	GPUMEM_COUNT_LIMIT+1 -> GPUMEM_COUNT_LIMIT+2
	GPUMEM_COUNT_LIMIT+2 -> GPUMEM_COUNT_LIMIT+3
	GPUMEM_COUNT_LIMIT+3 -> GPUMEM_COUNT_LIMIT+4
	...
	max -> GPUMEM_COUNT_LIMIT+0
	*/
	if(handle == SPECIAL_HANDLE_FOR_LOGITS)return;
	if(handle < GPUMEM_COUNT_LIMIT)return;

	int tocopy = (handle<gpumem_count_max) ? handle+1 : GPUMEM_COUNT_LIMIT;
	if(pending_data[tocopy].size == 0)return;

	__nv_bfloat16* gpumat = gpumem_get(tocopy);
	if(0==gpumat)return;	//not in gpumem yet
	__nv_bfloat16* cpumat = pinmem_get(tocopy);
	if(0==cpumat)return;	//not in cpumem yet

	int evid = tocopy&3;
	int size = pending_data[evid].size;
	cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
	cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
}
void maybe_delete_unused_pinmem(int handle)
{
	if(handle == SPECIAL_HANDLE_FOR_LOGITS)return;
	if(handle >= GPUMEM_COUNT_LIMIT)return;
	if(pinmem[handle]){
		cudaFreeHost(pinmem[handle]);
		pinmem[handle] = 0;
	}
}
				//gpumem macro end
				#endif



/*
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
}*/
void cuda_compute(int handle)
{
	u64 time[6];
	time[0] = time_in_ns();
	cudaEventRecord(event[0], stream[QUEUE_KERN]);

	cudaMemcpyAsync(gpuvec, cpuvec, vecbyte, cudaMemcpyHostToDevice, stream[QUEUE_KERN]);

	time[1] = time_in_ns();
	cudaEventRecord(event[1], stream[QUEUE_KERN]);

	__nv_bfloat16* gpumat = gpumem_get(handle);
	int evid = (handle==SPECIAL_HANDLE_FOR_LOGITS) ? 4 : (handle&3);
	//while(cudaEventQuery(copyevent[evid]) == cudaErrorNotReady);
	cudaEventSynchronize(copyevent[evid]);

	time[2] = time_in_ns();
	cudaEventRecord(event[2], stream[QUEUE_KERN]);

	// asynchronously issue work to the GPU
	int tx = 32;
	if(0 == (ydim%128))tx = 128;
	if(0 == (ydim%512))tx = 512;
	dim3 threads = dim3(tx, 1, 1);
	dim3 blocks  = dim3(ydim/tx, 1, 1);
	if(OPTIMISE_TRANSPOSE){
		muladd_kernel_transposed<<<blocks, threads, 0, stream[QUEUE_KERN]>>>(gpuout, gpuvec, gpumat, xdim, ydim);
	}
	else{
		muladd_kernel<<<blocks, threads, 0, stream[QUEUE_KERN]>>>(gpuout, gpuvec, gpumat, xdim, ydim);
	}
#if OPTIMISE_RESIDENTGPUMEM_MATRIX!=1
	maybe_start_copy_pin2gpu(handle);
#endif

	time[3] = time_in_ns();
	cudaEventRecord(event[3], stream[QUEUE_KERN]);

	cudaMemcpyAsync(cpuout, gpuout, outbyte, cudaMemcpyDeviceToHost, stream[QUEUE_KERN]);

	time[4] = time_in_ns();
	cudaEventRecord(event[4], stream[QUEUE_KERN]);

	// waiting for compute to finish
	cudaEventSynchronize(event[4]);
	time[5] = time_in_ns();

	float gputime[4] = {};
	for(int i=0;i<4;i++)cudaEventElapsedTime(&gputime[i], event[i], event[i+1]);

	float cputime[5] = {};
	for(int i=0;i<5;i++)cputime[i] = time[i+1] - time[i];
	//printf("gpu %d %d: %f, %f, %f, %f\n", xdim, ydim, gputime[0]*1e-3, gputime[1]*1e-3, gputime[2]*1e-3, gputime[3]*1e-3);
	//printf("cpu %d %d: %f, %f, %f, %f, %f\n", xdim, ydim, cputime[0]*1e-9, cputime[1]*1e-9, cputime[2]*1e-9, cputime[3]*1e-9, cputime[4]*1e-9);
}
__declspec(dllexport) void cudamath_upload(unsigned short* wbuf, int n, int d, int handle)
{
	int size = 2 * n * d;

	//pinmem get
	__nv_bfloat16* cpumat = pinmem_get(handle);
	if(0 == cpumat){
		cpumat = pinmem_create_or_get(handle, size);
	}

	//pinmem copy every upload
	if(DEBUG_MALLOC)printf("cpumem: handle=%d,size=%x,addr=%p\n", handle, size, cpumat);
	if(OPTIMISE_TRANSPOSE){
		cudamath_bf16transpose((unsigned short*)cpumat, wbuf, n, d, 0, d);
	}
	else{
		cudamath_bf16copy((unsigned short*)cpumat, wbuf, n*d);
	}

	//gpumem get
	int evid = (handle==SPECIAL_HANDLE_FOR_LOGITS) ? 4 : (handle&3);
	__nv_bfloat16* gpumat = gpumem_get(handle);
#if OPTIMISE_RESIDENTGPUMEM_MATRIX==1
	if(0 == gpumat){
		gpumat = gpumem_create_or_get(handle, size);
		if(DEBUG_MALLOC)printf("gpumem: handle=%d,size=%x,addr=%p\n", handle, size, gpumat);

		gpumem_resident_MB += size>>20;
		if(DEBUG_MALLOC)printf("gpumem_resident_MB=%d\n",gpumem_resident_MB);

		//gpumem copy only when first malloc
		cudaMemcpy(gpumat, cpumat, size, cudaMemcpyHostToDevice);
		cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
	}
#else
	if(0 == gpumat){
		gpumat = gpumem_create_or_get(handle, size);
		if(DEBUG_MALLOC)printf("gpumem: handle=%d,size=%x,addr=%p\n", handle, size, gpumat);
		gpumem_resident_MB += size>>20;
		if(DEBUG_MALLOC)printf("gpumem_resident_MB=%d\n",gpumem_resident_MB);

		//gpumem copy only when first malloc
		if( (handle==SPECIAL_HANDLE_FOR_LOGITS) | (handle<GPUMEM_COUNT_LIMIT) ){
			//cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
			cudaMemcpy(gpumat, cpumat, size, cudaMemcpyHostToDevice);
			cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
			maybe_delete_unused_pinmem(handle);
		}
	}
	if(handle!=SPECIAL_HANDLE_FOR_LOGITS){
		pending_data[handle].size = size;
		if(gpumem_count_max < handle)gpumem_count_max = handle;

		if(handle==GPUMEM_COUNT_LIMIT){
			cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
			cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
		}
	}
#endif
}
__declspec(dllexport) void cudamath_upload2(
	unsigned short* w0, int n0, int d0, int handle0,
	unsigned short* w1, int n1, int d1, int handle1)
{
	int size = 2 * n0 * (d0+d1);

	//pinmem get
	__nv_bfloat16* cpumat = pinmem_get(handle0);
	if(0 == cpumat){
		cpumat = pinmem_create_or_get(handle0, size);
	}

	//pinmem copy every upload
	if(DEBUG_MALLOC)printf("cpumem: handle=%d,size=%x,addr=%p\n", handle0, size, cpumat);
	if(OPTIMISE_TRANSPOSE){
		cudamath_bf16transpose((unsigned short*)cpumat, w0, n0, d0,  0, d0+d1);
		cudamath_bf16transpose((unsigned short*)cpumat, w1, n1, d1, d0, d0+d1);
	}
	else{
		cudamath_bf16copy((unsigned short*)&cpumat[    0], w0, n0*d0);
		cudamath_bf16copy((unsigned short*)&cpumat[n0*d0], w1, n1*d1);
	}

	//gpumem get
	int evid = (handle0==SPECIAL_HANDLE_FOR_LOGITS) ? 4 : (handle0&3);
	__nv_bfloat16* gpumat = gpumem_get(handle0);
#if OPTIMISE_RESIDENTGPUMEM_MATRIX==1
	if(0 == gpumat){
		gpumat = gpumem_create_or_get(handle0, size);
		if(DEBUG_MALLOC)printf("gpumem: handle=%d,size=%x,addr=%p\n", handle0, size, gpumat);

		gpumem_resident_MB += size>>20;
		if(DEBUG_MALLOC)printf("gpumem_resident_MB=%d\n",gpumem_resident_MB);

		//gpumem copy only when first malloc
		cudaMemcpy(gpumat, cpumat, size, cudaMemcpyHostToDevice);
		cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
	}
#else
	if(0 == gpumat){
		gpumat = gpumem_create_or_get(handle0, size);
		if(DEBUG_MALLOC)printf("gpumem: handle=%d,size=%x,addr=%p\n", handle0, size, gpumat);
		gpumem_resident_MB += size>>20;
		if(DEBUG_MALLOC)printf("gpumem_resident_MB=%d\n",gpumem_resident_MB);

		//gpumem copy only when first malloc
		if( (handle0==SPECIAL_HANDLE_FOR_LOGITS) | (handle0<GPUMEM_COUNT_LIMIT) ){
			//gpumem copy only when first malloc
			//cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
			cudaMemcpy(gpumat, cpumat, size, cudaMemcpyHostToDevice);
			cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
			maybe_delete_unused_pinmem(handle0);
		}
	}
	if(handle0!=SPECIAL_HANDLE_FOR_LOGITS){
		pending_data[handle0].size = size;
		if(gpumem_count_max < handle0)gpumem_count_max = handle0;

		if(handle0==GPUMEM_COUNT_LIMIT){
			cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
			cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
		}
	}
#endif
}
__declspec(dllexport) void cudamath_upload3(
	unsigned short* w0, int n0, int d0, int handle0,
	unsigned short* w1, int n1, int d1, int handle1,
	unsigned short* w2, int n2, int d2, int handle2)
{
	int size = 2 * n0 * (d0+d1+d2);

	//pinmem get
	__nv_bfloat16* cpumat = pinmem_get(handle0);
	if(0 == cpumat){
		cpumat = pinmem_create_or_get(handle0, size);
	}

	//pinmem copy every upload
	if(DEBUG_MALLOC)printf("cpumem: handle=%d,size=%x,addr=%p\n", handle0, size, cpumat);
	if(OPTIMISE_TRANSPOSE){
		cudamath_bf16transpose((unsigned short*)cpumat, w0, n0, d0,     0, d0+d1+d2);
		cudamath_bf16transpose((unsigned short*)cpumat, w1, n1, d1,    d0, d0+d1+d2);
		cudamath_bf16transpose((unsigned short*)cpumat, w2, n2, d2, d0+d1, d0+d1+d2);
	}
	else{
		cudamath_bf16copy((unsigned short*)&cpumat[          0], w0, n0*d0);
		cudamath_bf16copy((unsigned short*)&cpumat[      n0*d0], w1, n1*d1);
		cudamath_bf16copy((unsigned short*)&cpumat[n0*d0+n1*d1], w2, n2*d2);
	}

	//gpumem get
	int evid = (handle0==SPECIAL_HANDLE_FOR_LOGITS) ? 4 : (handle0&3);
	__nv_bfloat16* gpumat = gpumem_get(handle0);
#if OPTIMISE_RESIDENTGPUMEM_MATRIX==1
	if(0 == gpumat){
		gpumat = gpumem_create_or_get(handle0, size);
		if(DEBUG_MALLOC)printf("gpumem: handle=%d,size=%x,addr=%p\n", handle0, size, gpumat);

		gpumem_resident_MB += size>>20;
		if(DEBUG_MALLOC)printf("gpumem_resident_MB=%d\n",gpumem_resident_MB);

		//gpumem copy only when first malloc
		cudaMemcpy(gpumat, cpumat, size, cudaMemcpyHostToDevice);
		cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
	}
#else
	if(0 == gpumat){
		gpumat = gpumem_create_or_get(handle0, size);
		if(DEBUG_MALLOC)printf("gpumem: handle=%d,size=%x,addr=%p\n", handle0, size, gpumat);
		gpumem_resident_MB += size>>20;
		if(DEBUG_MALLOC)printf("gpumem_resident_MB=%d\n",gpumem_resident_MB);

		//gpumem copy only when first malloc
		if( (handle0==SPECIAL_HANDLE_FOR_LOGITS) | (handle0<GPUMEM_COUNT_LIMIT) ){
			//cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
			cudaMemcpy(gpumat, cpumat, size, cudaMemcpyHostToDevice);
			cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
			maybe_delete_unused_pinmem(handle0);
		}
	}
	if(handle0!=SPECIAL_HANDLE_FOR_LOGITS){
		pending_data[handle0].size = size;
		if(gpumem_count_max < handle0)gpumem_count_max = handle0;

		if(handle0==GPUMEM_COUNT_LIMIT){
			cudaMemcpyAsync(gpumat, cpumat, size, cudaMemcpyHostToDevice, stream[QUEUE_COPY]);
			cudaEventRecord(copyevent[evid], stream[QUEUE_COPY]);
		}
	}
#endif
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
	if(!MATRIXCOPY_EARLY){
		cudamath_upload(wbuf, n, d, handle);
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
	//for(int j=0;j<4;j++)cudaMallocHost((void **)&pinmem[j], matbyte);

	// allocate device memory
	cudaMalloc((void **)&gpuout, outbyte);
	cudaMalloc((void **)&gpuvec, vecbyte);
	//for(int j=0;j<4;j++)cudaMalloc((void **)&gpumem[j], matbyte);
	//cudaMemset(gpumem, 255, nbytes);

	for(int i=0;i<5;i++)cudaEventCreate(&event[i]);
	for(int i=0;i<5;i++)cudaEventCreate(&copyevent[i]);

	int hi,lo;
	cudaDeviceGetStreamPriorityRange(&lo, &hi);
	cudaStreamCreateWithPriority(&stream[QUEUE_KERN], cudaStreamNonBlocking, hi);
	cudaStreamCreateWithPriority(&stream[QUEUE_COPY], cudaStreamNonBlocking, lo);

	cudaDeviceSynchronize();

	u64 t1 = time_in_ns();
	printf("backend_init costtime: %f\n", (t1-t0)*1e-9);
}
__declspec(dllexport) void cudamath_exit()
{
	u64 t0 = time_in_ns();

	for(int i=0;i<2;i++)cudaStreamDestroy(stream[i]);

	for(int i=0;i<5;i++)cudaEventDestroy(event[i]);
	for(int i=0;i<5;i++)cudaEventDestroy(copyevent[i]);

	cudaFree(gpuvec);
	cudaFree(gpuout);
#if OPTIMISE_RESIDENTGPUMEM_MATRIX==1
	for(int j=0;j<32*4;j++)cudaFree(gpumem[j]);
#else
	for(int j=0;j<4;j++)cudaFree(gpumem[j]);
#endif

	cudaFreeHost(cpuvec);
	cudaFreeHost(cpuout);
#if OPTIMISE_RESIDENTPINMEM_MATRIX==1
	for(int j=0;j<32*4;j++)cudaFreeHost(pinmem[j]);
#else
	for(int j=0;j<4;j++)cudaFreeHost(pinmem[j]);
#endif

	//printf("time spent executing by the GPU: %f, %f, %f\n", gputime[0]*1e-3, gputime[1]*1e-3, gputime[2]*1e-3);

	//printf("time spent executing by the CPU: %f, %f\n", (time[1]-time[0])*1e-9, (time[2]-time[1])*1e-9);

	//printf("cycle spent executing by the CPU: %lu\n", counter);

	u64 t1 = time_in_ns();
	printf("backend_exit costtime: %f\n", (t1-t0)*1e-9);
}


}	//extern "C"