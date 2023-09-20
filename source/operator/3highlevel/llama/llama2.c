#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<time.h>
#include<math.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define s8 signed char
#define s16 signed short
#define s32 signed int
#define s64 signed long long
#define DEBUG_WEIGHT 1
#define DEBUG_TOKEN 0
#define DEBUG_PROMPT 1

#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
int input(void*, int);
int output(void*, int);
//
void* vulkan_init(void*, void*);
void vulkan_exit();
void vulkan_myctx_create(void*, void*);
void vulkan_myctx_delete();
//
void cudamath_init();
void cudamath_exit();

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
u64 time_in_ns()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (u64)t.tv_sec*1000*1000*1000 + t.tv_nsec;
}
#endif
static long long t0tot1 = 0;
static long long t1tot2 = 0;
static long long t2tot3 = 0;
static long long t3tot4 = 0;
//
static long long tatotb = 0;
static long long tbtotc = 0;
static long long tctotd = 0;
static long long tdtote = 0;
static long long tetotf = 0;
//
static long long tAtotB = 0;
static long long tBtotC = 0;
static long long tCtotD = 0;
static long long tDtotE = 0;
static long long tEtotF = 0;


unsigned long long rng_seed = 0x8273478;
unsigned int random_u32() {
	// xorshift rng: https://en.wikipedia.org/wiki/Xorshift#xorshift.2A
	rng_seed ^= rng_seed >> 12;
	rng_seed ^= rng_seed << 25;
	rng_seed ^= rng_seed >> 27;
	return (rng_seed * 0x2545F4914F6CDD1Dull) >> 32;
}
float random_f32() { // random float32 in [0,1)
	return (random_u32() >> 8) / 16777216.0f;
}


#define MODELWEIGHT_HEADSIZE 0x1c
#define MODELWEIGHT_FLOATTYPE __bf16
typedef struct{
	int dim; // transformer dimension
	int hidden_dim; // for ffn layers
	int n_layers; // number of layers
	int n_heads; // number of query heads
	int n_kv_heads; // number of key/value heads (can be < query heads because of multiquery)
	int vocab_size; // vocabulary size, usually 256 (byte-level)
	int seq_len; // max sequence length

	// token embedding table
	unsigned long long token_embedding_table_size;    // (vocab_size, dim)
	// weights for rmsnorms
	unsigned long long rms_att_weight_size; // (layer, dim) rmsnorm weights
	// weights for muladds
	unsigned long long wq_size; // (layer, dim, dim)
	unsigned long long wk_size; // (layer, dim, dim)
	unsigned long long wv_size; // (layer, dim, dim)
	unsigned long long wo_size; // (layer, dim, dim)
	//
	unsigned long long rms_ffn_weight_size; // (layer, dim)
	// weights for ffn
	unsigned long long w1_size; // (layer, hidden_dim, dim)
	unsigned long long w2_size; // (layer, dim, hidden_dim)
	unsigned long long w3_size; // (layer, hidden_dim, dim)
	// final rmsnorm
	unsigned long long rms_final_weight_size; // (dim,)
	// (optional) classifier weights for the logits, on the last layer
	unsigned long long wcls_size;

	MODELWEIGHT_FLOATTYPE* token_embedding_table_data;
	MODELWEIGHT_FLOATTYPE* rms_att_weight_data;
	MODELWEIGHT_FLOATTYPE* wq_data;
	MODELWEIGHT_FLOATTYPE* wk_data;
	MODELWEIGHT_FLOATTYPE* wv_data;
	MODELWEIGHT_FLOATTYPE* wo_data;
	MODELWEIGHT_FLOATTYPE* rms_ffn_weight_data;
	MODELWEIGHT_FLOATTYPE* w1_data;
	MODELWEIGHT_FLOATTYPE* w2_data;
	MODELWEIGHT_FLOATTYPE* w3_data;
	MODELWEIGHT_FLOATTYPE* rms_final_weight_data;
	MODELWEIGHT_FLOATTYPE* wcls_data;
}modelinfo;
void printfloat(void* addr)
{
	MODELWEIGHT_FLOATTYPE* p = addr;
	printf("float:	%f,%f,%f,%f\n", (float)p[0], (float)p[1], (float)p[2], (float)p[3]);
}
void printu8(void* addr)
{
	int j;
	u8* p = addr;
	printf("uchar8:	");
	for(j=0;j<16;j++)printf("%2x%c", p[j], j==15?'\n':',');
}
void printminmax(MODELWEIGHT_FLOATTYPE* buf, int len)
{
	float min = 1000*1000*1000.0;
	float max =-1000*1000*1000.0;

	int j;
	len /= sizeof(MODELWEIGHT_FLOATTYPE);
	for(j=0;j<len;j++){
		if(min > buf[j])min = buf[j];
		if(max < buf[j])max = buf[j];
	}
	printf("min=%f,max=%f\n", min, max);
}
s64 fullread(int fd, void* buf, u64 len)
{
#define eachread 0x1000000
	s64 j;
	s64 ret = -1;
	for(j=0;j<len;j+=eachread){
		if(j+eachread > len){
			ret = read(fd, buf+j, len-j);
		}
		else{
			ret = read(fd, buf+j, eachread);
		}
		//printf("offs=%llx,ret=%llx, errno=%d\n", j, ret, errno);
		if(eachread == ret)continue;
		if(ret >= 0)return j+ret;
		return -1;
	}
	return len;
}
void llama_initmodel(char* modelpath, modelinfo* mi)
{
	int fd = open(modelpath, O_RDONLY|O_BINARY);
	if(fd <= 0){
		printf("errno=%d@open\n", errno);
		return;
	}

	if(0 == mi){
		printf("errno=%d@malloc\n", errno);
		goto theend;
	}

	s64 ret = lseek64(fd, 0, SEEK_SET);
	if(ret < 0){
		printf("errno=%d@lseek\n", errno);
		goto releasehead;
	}

	ret = read(fd, mi, MODELWEIGHT_HEADSIZE);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto releasehead;
	}


	printf("--------header--------\n");
	int shared_weights = mi->vocab_size > 0 ? 1 : 0;
	if(mi->vocab_size < 0)mi->vocab_size = -mi->vocab_size;

	printf("dim=%d\n", mi->dim);
	printf("hidden_dim=%d\n", mi->hidden_dim);
	printf("n_layers=%d\n", mi->n_layers);
	printf("n_heads=%d\n", mi->n_heads);
	printf("n_kv_heads=%d\n", mi->n_kv_heads);
	printf("vocab_size=%d\n", mi->vocab_size);
	printf("seq_len=%d\n", mi->seq_len);
	printf("shared_weights=%d\n",shared_weights);

	printf("\n");


	u64 offs = MODELWEIGHT_HEADSIZE;
	u64 next = 0;
	printf("--------weight parsing--------\n");

	mi->token_embedding_table_size = mi->vocab_size * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next = offs + mi->token_embedding_table_size;
	printf("[%16llx,%16llx)%16lldMB        token_embedding_table\n", offs, next, mi->token_embedding_table_size>>20);
	offs = next;

	mi->rms_att_weight_size = mi->n_layers * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->rms_att_weight_size;
	printf("[%16llx,%16llx)%16lldMB        rms_att_weight\n", offs, next, mi->rms_att_weight_size>>20);
	offs = next;

	mi->wq_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->wq_size;
	printf("[%16llx,%16llx)%16lldMB        wq\n", offs, next, mi->wq_size>>20);
	offs = next;

	mi->wk_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->wk_size;
	printf("[%16llx,%16llx)%16lldMB        wk\n", offs, next, mi->wk_size>>20);
	offs = next;

	mi->wv_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->wv_size;
	printf("[%16llx,%16llx)%16lldMB        wv\n", offs, next, mi->wv_size>>20);
	offs = next;

	mi->wo_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->wo_size;
	printf("[%16llx,%16llx)%16lldMB        wo\n", offs, next, mi->wo_size>>20);
	offs = next;

	mi->rms_ffn_weight_size = (u64)mi->n_layers * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->rms_ffn_weight_size;
	printf("[%16llx,%16llx)%16lldMB        rms_ffn_weight\n", offs, next, mi->rms_ffn_weight_size>>20);
	offs = next;

	mi->w1_size = (u64)mi->n_layers * mi->dim * mi->hidden_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->w1_size;
	printf("[%16llx,%16llx)%16lldMB        w1\n", offs, next, mi->w1_size>>20);
	offs = next;

	mi->w2_size = (u64)mi->n_layers * mi->hidden_dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->w2_size;
	printf("[%16llx,%16llx)%16lldMB        w2\n", offs, next, mi->w2_size>>20);
	offs = next;

	mi->w3_size = (u64)mi->n_layers * mi->dim * mi->hidden_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->w3_size;
	printf("[%16llx,%16llx)%16lldMB        w3\n", offs, next, mi->w3_size>>20);
	offs = next;

	mi->rms_final_weight_size = (u64)mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	next += mi->rms_final_weight_size;
	printf("[%16llx,%16llx)%16lldMB        rms_final_weight\n", offs, next, mi->rms_final_weight_size>>20);
	offs = next;

	int head_size = mi->dim / mi->n_heads;
	u64 freq_cis_real_size = (u64)mi->seq_len * head_size / 2 * sizeof(MODELWEIGHT_FLOATTYPE);
	next += freq_cis_real_size;
	printf("[%16llx,%16llx)%16lldMB        freq_cis_real\n", offs, next, freq_cis_real_size>>20);
	offs = next;

	u64 freq_cis_imag_size = (u64)mi->seq_len * head_size / 2 * sizeof(MODELWEIGHT_FLOATTYPE);
	next += freq_cis_imag_size;
	printf("[%16llx,%16llx)%16lldMB        freq_cis_imag\n", offs, next, freq_cis_imag_size>>20);
	offs = next;

	if(shared_weights){
		mi->wcls_size = 0;
		printf("shared_weights@wcls\n");
	}
	else{
		mi->wcls_size = (u64)mi->vocab_size * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
		next += mi->wcls_size;
		printf("[%16llx,%16llx)%16lldMB        wcls\n", offs, next, mi->wcls_size>>20);
		offs = next;
	}

	printf("\n");


	printf("--------weight reading--------\n");
	offs = MODELWEIGHT_HEADSIZE;

	//token_embedding_table
	mi->token_embedding_table_data = malloc(mi->token_embedding_table_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->token_embedding_table_data, mi->token_embedding_table_size);
	printf("read:	%llx / %llx\n", ret, mi->token_embedding_table_size);
	if(DEBUG_WEIGHT){
		printu8(mi->token_embedding_table_data);
		printfloat(mi->token_embedding_table_data);
		printminmax(mi->token_embedding_table_data, mi->token_embedding_table_size);
	}
	offs += mi->token_embedding_table_size;

	//rms_att_weight
	mi->rms_att_weight_data = malloc(mi->rms_att_weight_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->rms_att_weight_data, mi->rms_att_weight_size);
	printf("read:	%llx / %llx\n", ret, mi->rms_att_weight_size);
	if(DEBUG_WEIGHT){
		printu8(mi->rms_att_weight_data);
		printfloat(mi->rms_att_weight_data);
		printminmax(mi->rms_att_weight_data, mi->rms_att_weight_size);
	}
	offs += mi->rms_att_weight_size;

	//wq
	mi->wq_data = malloc(mi->wq_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wq_data, mi->wq_size);
	printf("read:	%llx / %llx\n", ret, mi->wq_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wq_data);
		printfloat(mi->wq_data);
		printminmax(mi->wq_data, mi->wq_size);
	}
	offs += mi->wq_size;

	//wk
	mi->wk_data = malloc(mi->wk_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wk_data, mi->wk_size);
	printf("read:	%llx / %llx\n", ret, mi->wk_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wk_data);
		printfloat(mi->wk_data);
		printminmax(mi->wk_data, mi->wk_size);
	}
	offs += mi->wk_size;

	//wv
	mi->wv_data = malloc(mi->wv_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wv_data, mi->wv_size);
	printf("read:	%llx / %llx\n", ret, mi->wv_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wv_data);
		printfloat(mi->wv_data);
		printminmax(mi->wv_data, mi->wv_size);
	}
	offs += mi->wv_size;

	//wo
	mi->wo_data = malloc(mi->wo_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wo_data, mi->wo_size);
	printf("read:	%llx / %llx\n", ret, mi->wo_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wo_data);
		printfloat(mi->wo_data);
		printminmax(mi->wo_data, mi->wo_size);
	}
	offs += mi->wo_size;

	//rms_ffn_weight
	mi->rms_ffn_weight_data = malloc(mi->rms_ffn_weight_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->rms_ffn_weight_data, mi->rms_ffn_weight_size);
	printf("read:	%llx / %llx\n", ret, mi->rms_ffn_weight_size);
	if(DEBUG_WEIGHT){
		printu8(mi->rms_ffn_weight_data);
		printfloat(mi->rms_ffn_weight_data);
		printminmax(mi->rms_ffn_weight_data, mi->rms_ffn_weight_size);
	}
	offs += mi->rms_ffn_weight_size;

	//w1
	mi->w1_data = malloc(mi->w1_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->w1_data, mi->w1_size);
	printf("read:	%llx / %llx\n", ret, mi->w1_size);
	if(DEBUG_WEIGHT){
		printu8(mi->w1_data);
		printfloat(mi->w1_data);
		printminmax(mi->w1_data, mi->w1_size);
	}
	offs += mi->w1_size;

	//w2
	mi->w2_data = malloc(mi->w2_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->w2_data, mi->w2_size);
	printf("read:	%llx / %llx\n", ret, mi->w2_size);
	if(DEBUG_WEIGHT){
		printu8(mi->w2_data);
		printfloat(mi->w2_data);
		printminmax(mi->w2_data, mi->w2_size);
	}
	offs += mi->w2_size;

	//w3
	mi->w3_data = malloc(mi->w3_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->w3_data, mi->w3_size);
	printf("read:	%llx / %llx\n", ret, mi->w3_size);
	if(DEBUG_WEIGHT){
		printu8(mi->w3_data);
		printfloat(mi->w3_data);
		printminmax(mi->w3_data, mi->w3_size);
	}
	offs += mi->w3_size;

	//rms_final_weight
	mi->rms_final_weight_data = malloc(mi->rms_final_weight_size);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->rms_final_weight_data, mi->rms_final_weight_size);
	printf("read:	%llx / %llx\n", ret, mi->rms_final_weight_size);
	if(DEBUG_WEIGHT){
		printu8(mi->rms_final_weight_data);
		printfloat(mi->rms_final_weight_data);
		printminmax(mi->rms_final_weight_data, mi->rms_final_weight_size);
	}
	offs += mi->rms_final_weight_size;

	offs += freq_cis_real_size;
	offs += freq_cis_imag_size;

	if(shared_weights){
		mi->wcls_data = mi->token_embedding_table_data;
	}
	else{
		mi->wcls_data = malloc(mi->wcls_size);
		lseek64(fd, offs, SEEK_SET);
		ret = fullread(fd, mi->wcls_data, mi->wcls_size);
		printf("read:	%llx / %llx\n", ret, mi->wcls_size);
		if(DEBUG_WEIGHT){
			printu8(mi->wcls_data);
			printfloat(mi->wcls_data);
			printminmax(mi->wcls_data, mi->wcls_size);
		}
		offs += mi->wcls_size;
	}

	if(	!mi->token_embedding_table_data||
		!mi->rms_att_weight_data||
		!mi->wq_data||
		!mi->wk_data||
		!mi->wv_data||
		!mi->wo_data||
		!mi->rms_ffn_weight_data||
		!mi->w1_data||
		!mi->w2_data||
		!mi->w3_data||
		!mi->rms_final_weight_data||
		!mi->wcls_data)
	{
		fprintf(stderr, "malloc failed!\n");
		exit(EXIT_FAILURE);
	}
	else{
		printf("malloc ok\n\n");
	}

releasebody:
	//free(body);

releasehead:
	//free(head);

theend:
	close(fd);
}


#define RUNSTATE_FLOATTYPE float
typedef struct {
	// activation at current time stamp (dim,)
	u64 x_size;
	RUNSTATE_FLOATTYPE *x_data;

	// same, but inside a residual branch (dim,)
	u64 xb_size;
	RUNSTATE_FLOATTYPE *xb_data;

	// an additional buffer just for convenience (dim,)
	u64 xb2_size;
	RUNSTATE_FLOATTYPE *xb2_data;

	// buffer for hidden dimension in the ffn (hidden_dim,)
	u64 hb_size;
	RUNSTATE_FLOATTYPE *hb_data;

	// buffer for hidden dimension in the ffn (hidden_dim,)
	u64 hb2_size;
	RUNSTATE_FLOATTYPE *hb2_data;

	// query (dim,)
	u64 q_size;
	RUNSTATE_FLOATTYPE *q_data;

	// key (dim,)
	u64 k_size;
	RUNSTATE_FLOATTYPE *k_data;

	// value (dim,)
	u64 v_size;
	RUNSTATE_FLOATTYPE *v_data;

	// buffer for scores/attention values (n_heads, seq_len)
	u64 att_size;
	RUNSTATE_FLOATTYPE *att_data;

	// output logits
	u64 logits_size;
	RUNSTATE_FLOATTYPE *logits_data;

	// kv cache// (layer, seq_len, dim)
	u64 key_cache_size;
	RUNSTATE_FLOATTYPE* key_cache_data;

	// (layer, seq_len, dim)
	u64 value_cache_size;
	RUNSTATE_FLOATTYPE* value_cache_data;
} RunState;
void llama_initstate(modelinfo* mi, RunState* rs) {
	u64 offs = 0;
	u64 next = 0;
	int kv_dim = (mi->dim * mi->n_kv_heads) / mi->n_heads;
	printf("--------state parsing--------\n");

	rs->x_size = mi->dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->x_size;
	printf("[%16llx,%16llx)%16lldMB        x\n", offs, next, rs->x_size>>20);
	offs = next;

	rs->xb_size = mi->dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->xb_size;
	printf("[%16llx,%16llx)%16lldMB        xb\n", offs, next, rs->xb_size>>20);
	offs = next;

	rs->xb2_size = mi->dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->xb2_size;
	printf("[%16llx,%16llx)%16lldMB        xb2\n", offs, next, rs->xb2_size>>20);
	offs = next;

	rs->hb_size = mi->hidden_dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->hb_size;
	printf("[%16llx,%16llx)%16lldMB        hb\n", offs, next, rs->hb_size>>20);
	offs = next;

	rs->hb2_size = mi->hidden_dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->hb2_size;
	printf("[%16llx,%16llx)%16lldMB        hb2\n", offs, next, rs->hb2_size>>20);
	offs = next;

	rs->q_size = mi->dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->q_size;
	printf("[%16llx,%16llx)%16lldMB        q\n", offs, next, rs->q_size>>20);
	offs = next;

	rs->k_size = kv_dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->k_size;
	printf("[%16llx,%16llx)%16lldMB        k\n", offs, next, rs->k_size>>20);
	offs = next;

	rs->v_size = kv_dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->v_size;
	printf("[%16llx,%16llx)%16lldMB        v\n", offs, next, rs->v_size>>20);
	offs = next;

	rs->att_size = mi->n_heads * mi->seq_len * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->att_size;
	printf("[%16llx,%16llx)%16lldMB        att\n", offs, next, rs->att_size>>20);
	offs = next;

	rs->logits_size = mi->vocab_size * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->logits_size;
	printf("[%16llx,%16llx)%16lldMB        logits\n", offs, next, rs->logits_size>>20);
	offs = next;

	rs->key_cache_size = mi->n_layers * mi->seq_len * kv_dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->key_cache_size;
	printf("[%16llx,%16llx)%16lldMB        key_cache\n", offs, next, rs->key_cache_size>>20);
	offs = next;

	rs->value_cache_size = mi->n_layers * mi->seq_len * kv_dim * sizeof(RUNSTATE_FLOATTYPE);
	next = offs + rs->value_cache_size;
	printf("[%16llx,%16llx)%16lldMB        value_cache\n", offs, next, rs->value_cache_size>>20);
	offs = next;

	printf("\n");

	printf("--------state mallocing--------\n");
	rs->x_data = malloc(rs->x_size);
	rs->xb_data = malloc( rs->xb_size);
	rs->xb2_data = malloc( rs->xb2_size);
	rs->hb_data = malloc( rs->hb_size);
	rs->hb2_data = malloc( rs->hb2_size);
	rs->q_data = malloc( rs->q_size);
	rs->k_data = malloc( rs->k_size);
	rs->v_data = malloc( rs->v_size);
	rs->att_data = malloc( rs->att_size);
	rs->logits_data = malloc( rs->logits_size);
	rs->key_cache_data = malloc( rs->key_cache_size);
	rs->value_cache_data = malloc( rs->value_cache_size);

	if(	!rs->x_data ||
		!rs->xb_data ||
		!rs->xb2_data ||
		!rs->hb_data ||
		!rs->hb2_data ||
		!rs->q_data ||
		!rs->k_data ||
		!rs->v_data ||
		!rs->att_data ||
		!rs->logits_data ||
		!rs->key_cache_data ||
		!rs->value_cache_data)
	{
		fprintf(stderr, "malloc failed!\n");
		exit(EXIT_FAILURE);
	}
	else{
		printf("malloc ok\n\n");
	}
}


typedef struct{
	unsigned int max_token_length;
	float* vocab_scores;
	char** vocab;
}tokeninfo;
void llama_inittokenizer(char* tokenpath, modelinfo* mi, tokeninfo* ti)
{
	printf("--------inittokenizer--------\n");

	FILE *file = fopen(tokenpath, "rb");
	if (!file) { printf("couldn't load tokenizer.bin\n"); return; }

	unsigned int max_token_length;
	if (fread(&max_token_length, sizeof(int), 1, file) != 1) { printf("failed read\n"); return; }
	printf("max_token_length=%d\n",max_token_length);
	ti->max_token_length = max_token_length;

	float* vocab_scores = (float*)malloc(mi->vocab_size * sizeof(float));
	ti->vocab_scores = vocab_scores;

	char** vocab = (char**)malloc(mi->vocab_size * sizeof(char*));
	ti->vocab = vocab;

	int len;
	float f32;
	for (int i = 0; i < mi->vocab_size; i++) {
		if (fread(&f32, sizeof(float), 1, file) != 1) { printf("failed read\n"); return;}
		vocab_scores[i] = f32;
		if (fread(&len, sizeof(int), 1, file) != 1) { printf("failed read\n"); return; }
		vocab[i] = (char *)malloc(len + 1);
		if (fread(vocab[i], len, 1, file) != 1) { printf("failed read\n"); return; }
		vocab[i][len] = '\0'; // add the string terminating token
		if(DEBUG_TOKEN)printf("%d: score=%f, string=%.*s\n", i, vocab_scores[i], len, vocab[i]);
	}
	fclose(file);

	printf("\n");
}


typedef struct {
	int num_prompt_tokens;
	int* prompt_tokens;
} TokenState;
void llama_initprompt(modelinfo* mi, TokenState* ts)
{
	printf("--------initprompt--------\n");

	ts->prompt_tokens = (int*)malloc(mi->seq_len * sizeof(int));
	ts->num_prompt_tokens = 0;
	printf("size=%d,addr=%p\n", ts->num_prompt_tokens, ts->prompt_tokens);

	printf("\n");
}


int str_lookup(char *str, char **vocab, int vocab_size) {
	// find the first perfect match for str in vocab, return its index or -1 if not found
	for (int i = 0; i < vocab_size; i++) {
		if (strcmp(str, vocab[i]) == 0) {
			return i;
		}
	}
	return -1;
}
int bpe_encode(unsigned char *text, char **vocab, float *vocab_scores, int vocab_size, unsigned int max_token_length, int *tokens, int *n_tokens) {
	
	// a temporary buffer to merge two consecutive tokens
	unsigned char* str_buffer = malloc((max_token_length*2+1) * sizeof(char)); // *2 for concat, +1 for null terminator

	// first encode every individual byte in the input string
	*n_tokens = 0; // the number of tokens
	for (unsigned char *c = text; *c != '\0'; c++) {
		if(*c >= 0xf0){
			memcpy(str_buffer, c, 4);
			str_buffer[4] = 0;
			c += 3;
		}
		else if(*c >= 0xe0){
			memcpy(str_buffer, c, 3);
			str_buffer[3] = 0;
			c += 2;
		}
		else if(*c >= 0xc0){
			memcpy(str_buffer, c, 2);
			str_buffer[2] = 0;
			c += 1;
		}
		else if(*c == 0xa){
			sprintf((void*)str_buffer, "<0x0A>");
		}
		else{
			sprintf((char*)str_buffer, "%c", *c);
		}
		int id = str_lookup((char*)str_buffer, vocab, vocab_size);
		if (id == -1) {
			printf("not good: %x,%x,%x,%x\n",str_buffer[0],str_buffer[1],str_buffer[2],str_buffer[3]);
			return -1;
		}
		tokens[*n_tokens] = id;
		(*n_tokens)++;
	}

	// merge the best consecutive pair each iteration, according the scores in vocab_scores
	while (1) {
		float best_score = -1e10;
		int best_id = -1;
		int best_idx = -1;

		for (int i=0; i < (*n_tokens-1); i++) {
			// check if we can merge the pair (tokens[i], tokens[i+1])
			sprintf((char*)str_buffer, "%s%s", vocab[tokens[i]], vocab[tokens[i+1]]);
			int id = str_lookup((char*)str_buffer, vocab, vocab_size);
			if (id != -1 && vocab_scores[id] > best_score) {
				// this merge pair exists in vocab! record its score and position
				best_score = vocab_scores[id];
				best_id = id;
				best_idx = i;
			}
		}

		if (best_idx == -1) {
			break; // we couldn't find any more pairs to merge, so we're done
		}

		// merge the consecutive pair (best_idx, best_idx+1) into new token best_id
		tokens[best_idx] = best_id;
		// delete token at position best_idx+1, shift the entire sequence back 1
		for (int i = best_idx+1; i < (*n_tokens-1); i++) {
			tokens[i] = tokens[i+1];
		}
		(*n_tokens)--; // token length decreased
	}

	free(str_buffer);
	return 1;
}
int llama_prompt(modelinfo* mi, tokeninfo* tk, TokenState* ts, char* prompt)
{
	printf("--------prompt--------\n");
	if(NULL == prompt){
		goto theend;
	}

	int ret = bpe_encode((u8*)prompt, tk->vocab, tk->vocab_scores, mi->vocab_size, tk->max_token_length, ts->prompt_tokens, &ts->num_prompt_tokens);
	if(ret<0)return -1;
	for(int j=0;j<ts->num_prompt_tokens;j++){
		if(DEBUG_PROMPT){
			int t = ts->prompt_tokens[j];
			printf("%d: token=%d, string=", j, t);
			output(tk->vocab[t], strlen(tk->vocab[t]));
			printf("\n");
		}
	}

theend:
	printf("\n");
	return ts->num_prompt_tokens;
}


void rmsnorm(RUNSTATE_FLOATTYPE* o, RUNSTATE_FLOATTYPE* x, MODELWEIGHT_FLOATTYPE* weight, int size) {
	// calculate sum of squares
	float ss = 0.0f;
	for (int j = 0; j < size; j++) {
		ss += x[j] * x[j];
	}
	ss /= size;
	ss += 1e-5f;
	ss = 1.0f / sqrtf(ss);
	// normalize and scale
	for (int j = 0; j < size; j++) {
		o[j] = weight[j] * x[j] * ss;
	}
}
void softmax(RUNSTATE_FLOATTYPE* x, int size) {
	// find max value (for numerical stability)
	float max_val = x[0];
	for (int i = 1; i < size; i++) {
		if (x[i] > max_val) {
			max_val = x[i];
		}
	}
	// exp and sum
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		x[i] = expf(x[i] - max_val);
		sum += x[i];
	}
	// normalize
	for (int i = 0; i < size; i++) {
		x[i] /= sum;
	}
}

#ifdef BACKEND_VULKAN
void vulkan_muladd(RUNSTATE_FLOATTYPE* xout, RUNSTATE_FLOATTYPE* x, MODELWEIGHT_FLOATTYPE* w, int n, int d, int handle);
void vulkan_muladd2(
	RUNSTATE_FLOATTYPE* xout0, RUNSTATE_FLOATTYPE* x0, MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	RUNSTATE_FLOATTYPE* xout1, RUNSTATE_FLOATTYPE* x1, MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle1);
void vulkan_muladd3(
	RUNSTATE_FLOATTYPE* xout0, RUNSTATE_FLOATTYPE* x0, MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	RUNSTATE_FLOATTYPE* xout1, RUNSTATE_FLOATTYPE* x1, MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle1,
	RUNSTATE_FLOATTYPE* xout2, RUNSTATE_FLOATTYPE* x2, MODELWEIGHT_FLOATTYPE* w2, int n2, int d2, int handle2);
#define muladd vulkan_muladd
#define muladd2 vulkan_muladd2
#define muladd3 vulkan_muladd3

#elif BACKEND_CUDA
void cudamath_upload(MODELWEIGHT_FLOATTYPE* w, int n, int d, int handle);
void cudamath_upload2(
	MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle2);
void cudamath_upload3(
	MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle1,
	MODELWEIGHT_FLOATTYPE* w2, int n2, int d2, int handle2);
void cudamath_muladd(RUNSTATE_FLOATTYPE* xout, RUNSTATE_FLOATTYPE* x, MODELWEIGHT_FLOATTYPE* w, int n, int d, int handle);
void cudamath_muladd2(
	RUNSTATE_FLOATTYPE* xout0, RUNSTATE_FLOATTYPE* x0, MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	RUNSTATE_FLOATTYPE* xout1, RUNSTATE_FLOATTYPE* x1, MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle2);
void cudamath_muladd3(
	RUNSTATE_FLOATTYPE* xout0, RUNSTATE_FLOATTYPE* x0, MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	RUNSTATE_FLOATTYPE* xout1, RUNSTATE_FLOATTYPE* x1, MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle1,
	RUNSTATE_FLOATTYPE* xout2, RUNSTATE_FLOATTYPE* x2, MODELWEIGHT_FLOATTYPE* w2, int n2, int d2, int handle2);
#define muladd cudamath_muladd
#define muladd2 cudamath_muladd2
#define muladd3 cudamath_muladd3
#define upload cudamath_upload
#define upload2 cudamath_upload2
#define upload3 cudamath_upload3

#else
void muladd(RUNSTATE_FLOATTYPE* xout, RUNSTATE_FLOATTYPE* x, MODELWEIGHT_FLOATTYPE* w, int n, int d, int handle)
{
	// W (d,n) @ x (n,) -> xout (d,)
	// by far the most amount of time is spent inside this little function
	int i;
	#pragma omp parallel for private(i)
	for (i = 0; i < d; i++) {
		float val = 0.0f;
		for (int j = 0; j < n; j++) {
			val += w[i * n + j] * x[j];
		}
		xout[i] = val;
	}
	//printf("%f,%f,%f\n",xout[0], xout[767], xout[d-1]);
}
void muladd2(
	RUNSTATE_FLOATTYPE* xout0, RUNSTATE_FLOATTYPE* x0, MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	RUNSTATE_FLOATTYPE* xout1, RUNSTATE_FLOATTYPE* x1, MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle1)
{
	muladd(xout0, x0, w0, n0, d0, handle0);
	muladd(xout1, x1, w1, n1, d1, handle1);
}
void muladd3(
	RUNSTATE_FLOATTYPE* xout0, RUNSTATE_FLOATTYPE* x0, MODELWEIGHT_FLOATTYPE* w0, int n0, int d0, int handle0,
	RUNSTATE_FLOATTYPE* xout1, RUNSTATE_FLOATTYPE* x1, MODELWEIGHT_FLOATTYPE* w1, int n1, int d1, int handle1,
	RUNSTATE_FLOATTYPE* xout2, RUNSTATE_FLOATTYPE* x2, MODELWEIGHT_FLOATTYPE* w2, int n2, int d2, int handle2)
{
	muladd(xout0, x0, w0, n0, d0, handle0);
	muladd(xout1, x1, w1, n1, d1, handle1);
	muladd(xout2, x2, w2, n2, d2, handle2);
}
#endif

void dequantization(RUNSTATE_FLOATTYPE* dst, MODELWEIGHT_FLOATTYPE* src, int cnt)
{
	int j;
	for(j=0;j<cnt;j++)dst[j] = src[j];
}
void transformer_eachlayer(RUNSTATE_FLOATTYPE* x, int pos, modelinfo* mi, RunState* rs, int layer)
{
//----------------first stage----------------
	int dim = mi->dim;
	int head_size = dim / mi->n_heads;
	int kv_dim = (mi->dim * mi->n_kv_heads) / mi->n_heads;
	int kv_mul = mi->n_heads / mi->n_kv_heads;

	MODELWEIGHT_FLOATTYPE* w_rms_att_weight = mi->rms_att_weight_data + layer*dim;
	MODELWEIGHT_FLOATTYPE* w_wq = mi->wq_data + layer*dim*dim;
	MODELWEIGHT_FLOATTYPE* w_wk = mi->wk_data + layer*dim*kv_dim;
	MODELWEIGHT_FLOATTYPE* w_wv = mi->wv_data + layer*dim*kv_dim;
	MODELWEIGHT_FLOATTYPE* w_wo = mi->wo_data + layer*dim*dim;

	RUNSTATE_FLOATTYPE* rs_xb = rs->xb_data;
	RUNSTATE_FLOATTYPE* rs_xb2 = rs->xb2_data;
	RUNSTATE_FLOATTYPE* rs_q = rs->q_data;
	RUNSTATE_FLOATTYPE* rs_k = rs->k_data;
	RUNSTATE_FLOATTYPE* rs_v = rs->v_data;
	RUNSTATE_FLOATTYPE* rs_att = rs->att_data;
	RUNSTATE_FLOATTYPE* rs_key_cache = rs->key_cache_data;
	RUNSTATE_FLOATTYPE* rs_value_cache = rs->value_cache_data;

//----------------second stage----------------
	//int dim = mi->dim;	//redefinition
	int hidden_dim =  mi->hidden_dim;

	MODELWEIGHT_FLOATTYPE* w_rms_ffn_weight = mi->rms_ffn_weight_data + layer*dim;
	MODELWEIGHT_FLOATTYPE* w_w1 = mi->w1_data + layer*dim*hidden_dim;
	MODELWEIGHT_FLOATTYPE* w_w2 = mi->w2_data + layer*dim*hidden_dim;
	MODELWEIGHT_FLOATTYPE* w_w3 = mi->w3_data + layer*dim*hidden_dim;

	//RUNSTATE_FLOATTYPE* rs_xb = rs->xb_data;	//redefinition
	RUNSTATE_FLOATTYPE* rs_hb = rs->hb_data;
	RUNSTATE_FLOATTYPE* rs_hb2 = rs->hb2_data;

//----------------upload data----------------
#ifdef BACKEND_CUDA
	upload3(	//copy 0
		w_wq, dim,    dim, layer*4+0,
		w_wk, dim, kv_dim, 0,
		w_wv, dim, kv_dim, 0);
#endif
#ifdef BACKEND_CUDA
	upload(w_wo, dim, dim, layer*4+1);	//copy 1
#endif
#ifdef BACKEND_CUDA
	upload2(	//copy 2
		w_w1, dim, hidden_dim, layer*4+2,
		w_w3, dim, hidden_dim, 0);
#endif
#ifdef BACKEND_CUDA
	upload(w_w2, hidden_dim, dim, layer*4+3);	//copy 3
#endif



//----------------first stage----------------
	u64 ta = time_in_ns();

	// attention rmsnorm
	rmsnorm(rs_xb, x, w_rms_att_weight, dim);

	u64 tb = time_in_ns();
	tatotb += tb-ta;

	// qkv muladds for this position
	muladd3(	//compute 0
		rs_q, rs_xb, w_wq, dim,    dim, layer*4+0,
		rs_k, rs_xb, w_wk, dim, kv_dim, 0,
		rs_v, rs_xb, w_wv, dim, kv_dim, 0);

	u64 tc = time_in_ns();
	tbtotc += tc-tb;

	// RoPE relative positional encoding: complex-valued rotate q and k by freq_cis in each head
	for (int i = 0; i < dim; i+=2) {
		int head_dim = i % head_size;
		float freq = 1.0f / powf(10000.0f, head_dim / (float)head_size);
		float val = pos * freq;
		float fcr = cosf(val);
		float fci = sinf(val);
		int rotn = i < kv_dim ? 2 : 1; // how many vectors? 2 = q & k, 1 = q only
		for (int v = 0; v < rotn; v++) {
			RUNSTATE_FLOATTYPE* vec = (v == 0) ? rs_q : rs_k; // the vector to rotate (query or key)
			float v0 = vec[i];
			float v1 = vec[i+1];
			vec[i]   = v0 * fcr - v1 * fci;
			vec[i+1] = v0 * fci + v1 * fcr;
		}
	}

	// save key,value at this time step (pos) to our kv cache
	int loff = layer * mi->seq_len * kv_dim; // kv cache layer offset for convenience
	RUNSTATE_FLOATTYPE* key_cache_row = rs_key_cache + loff + pos * kv_dim;
	RUNSTATE_FLOATTYPE* value_cache_row = rs_value_cache + loff + pos * kv_dim;
	memcpy(key_cache_row, rs_k, kv_dim*sizeof(*key_cache_row));
	memcpy(value_cache_row, rs_v, kv_dim*sizeof(*value_cache_row));

	// multihead attention. iterate over all heads
	int h;
	#pragma omp parallel for private(h)
	for (h = 0; h < mi->n_heads; h++) {
		// get the query vector for this head
		RUNSTATE_FLOATTYPE* q = rs_q + h * head_size;
		// attention scores for this head
		RUNSTATE_FLOATTYPE* att = rs_att + h * mi->seq_len;
		// iterate over all timesteps, including the current one
		for (int t = 0; t <= pos; t++) {
			// get the key vector for this head and at this timestep
			RUNSTATE_FLOATTYPE* k = rs_key_cache + loff + t * kv_dim + (h/kv_mul) * head_size;
			// calculate the attention score as the dot product of q and k
			float score = 0.0f;
			for (int i = 0; i < head_size; i++) {
				score += q[i] * k[i];
			}
			score /= sqrtf(head_size);
			// save the score to the attention buffer
			att[t] = score;
		}

		// softmax the scores to get attention weights, from 0..pos inclusively
		softmax(att, pos + 1);

		// weighted sum of the values, store back into xb
		RUNSTATE_FLOATTYPE* xb = rs_xb + h * head_size;
		memset(xb, 0, head_size * sizeof(RUNSTATE_FLOATTYPE));
		for (int t = 0; t <= pos; t++) {
			// get the value vector for this head and at this timestep
			RUNSTATE_FLOATTYPE* v = rs_value_cache + loff + t * kv_dim + (h/kv_mul) * head_size;
			// get the attention weight for this timestep
			float a = att[t];
			// accumulate the weighted value into xb
			for (int i = 0; i < head_size; i++) {
				xb[i] += a * v[i];
			}
		}
	}

	u64 td = time_in_ns();
	tctotd += td-tc;

	// final muladd to get the output of the attention
	muladd(rs_xb2, rs_xb, w_wo, dim, dim, layer*4+1);	//compute 1

	u64 te = time_in_ns();
	tdtote += te-td;

	// residual connection back into x
	for (int i = 0; i < dim; i++) {
		x[i] += rs_xb2[i];
	}

	u64 tf = time_in_ns();
	tetotf += tf-te;




//----------------second stage----------------
	u64 tA = time_in_ns();

	// ffn rmsnorm
	rmsnorm(rs_xb, x, w_rms_ffn_weight, dim);

	u64 tB = time_in_ns();
	tAtotB += tB-tA;

	// Now for FFN in PyTorch we have: self.w2(F.silu(self.w1(x)) * self.w3(x))
	// first calculate self.w1(x) and self.w3(x)
	muladd2(	//compute 2
		rs_hb , rs_xb, w_w1, dim, hidden_dim, layer*4+2,
		rs_hb2, rs_xb, w_w3, dim, hidden_dim, 0);

	u64 tC = time_in_ns();
	tBtotC += tC-tB;

	// SwiGLU non-linearity
	for (int i = 0; i < hidden_dim; i++) {
		float val = rs_hb[i];
		// silu(x)=x*σ(x), where σ(x) is the logistic sigmoid
		val *= (1.0f / (1.0f + expf(-val)));
		// elementwise multiply with w3(x)
		val *= rs_hb2[i];
		rs_hb[i] = val;
	}

	u64 tD = time_in_ns();
	tCtotD += tD-tC;

	// final muladd to get the output of the ffn
	muladd(rs_xb, rs_hb, w_w2, hidden_dim, dim, layer*4+3);	//compute 3

	u64 tE = time_in_ns();
	tDtotE += tE-tD;

	// residual connection
	for (int i = 0; i < dim; i++) {
		x[i] += rs_xb[i];
	}

	u64 tF = time_in_ns();
	tEtotF += tF-tE;
}
void transformer(int token, int pos, modelinfo* mi, RunState* rs) {
	u64 t0 = time_in_ns();

	int dim = mi->dim;
	MODELWEIGHT_FLOATTYPE* w_token_embedding_table = mi->token_embedding_table_data;
	MODELWEIGHT_FLOATTYPE* w_rms_final_weight = mi->rms_final_weight_data;
	MODELWEIGHT_FLOATTYPE* w_wcls = mi->wcls_data;

	RUNSTATE_FLOATTYPE* rs_x = rs->x_data;
	RUNSTATE_FLOATTYPE* rs_logits = rs->logits_data;

	// copy the token embedding into x
	dequantization(rs_x, &w_token_embedding_table[token * dim], dim);

	u64 t1 = time_in_ns();
	t0tot1 += t1-t0;

	// forward all the layers
	for(int l = 0; l < mi->n_layers; l++) {
		transformer_eachlayer(rs_x, pos, mi, rs, l);
	}
	upload(w_wcls, mi->dim, mi->vocab_size, 32000);	//copy 5

	u64 t2 = time_in_ns();
	t1tot2 += t2-t1;

	// final rmsnorm
	rmsnorm(rs_x, rs_x, w_rms_final_weight, dim);

	u64 t3 = time_in_ns();
	t2tot3 += t3-t2;

	// classifier into logits
	muladd(rs_logits, rs_x, w_wcls, mi->dim, mi->vocab_size, 32000);

	u64 t4 = time_in_ns();
	t3tot4 += t4-t3;
}
int sample(RUNSTATE_FLOATTYPE* probabilities, int n) {
	// sample index from probabilities, they must sum to 1
	float r = random_f32();
	float cdf = 0.0f;
	for (int i = 0; i < n; i++) {
		cdf += probabilities[i];
		if (r < cdf) {
			return i;
		}
	}
	return n - 1; // in case of rounding errors
}
int argmax(RUNSTATE_FLOATTYPE* v, int n) {
	// return argmax of v in elements 0..n
	int max_i = 0;
	MODELWEIGHT_FLOATTYPE max_p = v[0];
	for (int i = 1; i < n; i++) {
		if (v[i] > max_p) {
			max_i = i;
			max_p = v[i];
		}
	}
	return max_i;
}
void llama_runmodel(modelinfo* mi, RunState* rs, tokeninfo* ti, TokenState* ts)
{
	printf("--------runmodel--------\n");
	RUNSTATE_FLOATTYPE* rs_logits = rs->logits_data;

	//time
	u64 start = 0;  // used to time our code, only initialized after first iteration
	t0tot1 = t1tot2 = t2tot3 = t3tot4 = 0;
	tatotb = tbtotc = tctotd = tdtote = tetotf = 0;
	tAtotB = tBtotC = tCtotD = tDtotE = tEtotF = 0;

	// start the main loop
	int next;        // will store the next token in the sequence
	int token = 1;   // init with token 1 (=BOS), as done in Llama-2 sentencepiece tokenizer
	int pos = 0;     // position in the sequence
	//printf("<s>\n"); // explicit print the initial BOS token for stylistic symmetry reasons

	float temperature = 1.0;
	int steps = mi->seq_len;
	while (pos < steps) {
		//printf("pos=%d,steps=%d\n",pos,steps);

		// forward the transformer to get logits for the next token
		transformer(token, pos, mi, rs);

		if(pos < ts->num_prompt_tokens) {
			// if we are still processing the input prompt, force the next prompt token
			next = ts->prompt_tokens[pos];
		} else {
			// sample the next token
			if (temperature == 0.0f) {
				// greedy argmax sampling: take the token with the highest probability
				next = argmax(rs_logits, mi->vocab_size);
			} else {
				// apply the temperature to the logits
				for (int q=0; q<mi->vocab_size; q++) { rs_logits[q] /= temperature; }
				// apply softmax to the logits to get the probabilities for next token
				softmax(rs_logits, mi->vocab_size);
				// we sample from this distribution to get the next token
				next = sample(rs_logits, mi->vocab_size);
			}
		}
		pos++;

		if(2 >= next)break;

		// following BOS token (1), sentencepiece decoder strips any leading whitespace (see PR #89)
		char *token_str = (token == 1 && ti->vocab[next][0] == ' ') ? ti->vocab[next]+1 : ti->vocab[next];
		//printf("%s", token_str);
		if(0 == strncmp(token_str, "<0x0A>", 6))token_str = "\n";
		output(token_str, strlen(token_str));
		fflush(stdout);
		token = next;

		// init our timer here because the first iteration is slow due to memmap
		if (start == 0) { start = time_in_ns(); }
	}
	printf("\n\n");

	if (pos > 1) {
		u64 end = time_in_ns();
		printf("--------evaluate--------\n");
		printf("token=%d, time=%f, t/s=%f\n", pos-1, (end-start)*1e-9, 1e9 * (pos-1) / (end-start));
		printf("t0tot1=%f, t1tot2=%f, t2tot3=%f, t3tot4=%f\n", t0tot1*1e-9, t1tot2*1e-9, t2tot3*1e-9, t3tot4*1e-9);
		printf("tatotb=%f, tbtotc=%f, tctotd=%f, tdtote=%f, tetotf=%f\n", tatotb*1e-9, tbtotc*1e-9, tctotd*1e-9, tdtote*1e-9, tetotf*1e-9);
		printf("tAtotB=%f, tBtotC=%f, tCtotD=%f, tDtotE=%f, tEtotF=%f\n", tAtotB*1e-9, tBtotC*1e-9, tCtotD*1e-9, tDtotE*1e-9, tEtotF*1e-9);
	}
}


int getinputincludingcrlf(char* str, int len)
{
	int ret = 0;
	int off = 0;
	do{
		ret = input(str+off, len-off);
		if(1==ret)break;

		if(off>1)str[off-1] = '\n';

		str[off+ret] = 0;
		off += ret;
	}while(ret>1);

	return off;
}
void llama(int argc, char** argv)
{
	modelinfo model;
	llama_initmodel(argv[1], &model);

	RunState modelstate;
	llama_initstate(&model, &modelstate);

	tokeninfo token;
	llama_inittokenizer(argv[2], &model, &token);

	TokenState tokenstate;
	llama_initprompt(&model, &tokenstate);

#ifdef BACKEND_VULKAN
	void* ins = vulkan_init(0, 0);
	if(0 == ins)return;
	vulkan_myctx_create(0, 0);
#elif BACKEND_CUDA
	cudamath_init();
#endif

	int ret;
	char str[4096];
	do{
		printf("--------userinput--------\n");
		printf(">>");
		getinputincludingcrlf(str, 4096);
		printu8(str);
		printf("\n");

		ret = llama_prompt(&model, &token, &tokenstate, str);
		if(ret<0)continue;

		llama_runmodel(&model, &modelstate, &token, &tokenstate);
		printf("\n");
	}while(1);
#ifdef BACKEND_VULKAN
	vulkan_myctx_delete();
	vulkan_exit();
#elif BACKEND_CUDA
	cudamath_exit();
#endif
	//llama_exitprompt();
	//llama_exittokenizer();
	//llama_exitstate();
	//llama_exitmodel();
}
