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

#ifdef _WIN32
#include <windows.h>
int clock_gettime(int clk_id, struct timespec *tp) {
	u32 ticks = GetTickCount();
	tp->tv_sec = ticks / 1000;
	tp->tv_nsec = (ticks % 1000) * 1000000;
	return 0;
}
#elif __APPLE__
#define lseek64 lseek
#endif
long time_in_ms() {
	// return time in milliseconds, for benchmarking the model speed
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}


#define MODELWEIGHT_HEADSIZE 0x1c
#define MODELWEIGHT_FLOATTYPE float
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
	// weights for matmuls
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
	// freq_cis for RoPE relatively positional embeddings
	unsigned long long freq_cis_real_size; // (seq_len, dim/2)
	unsigned long long freq_cis_imag_size; // (seq_len, dim/2)
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
	MODELWEIGHT_FLOATTYPE* freq_cis_real_data;
	MODELWEIGHT_FLOATTYPE* freq_cis_imag_data;
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
	printf("--------weight--------\n");

	//token_embedding_table
	mi->token_embedding_table_size = mi->vocab_size * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->token_embedding_table_data = malloc(mi->token_embedding_table_size);
	next = offs + mi->token_embedding_table_size;
	printf("[%16llx,%16llx)@%p:token_embedding_table\n", offs, next, mi->token_embedding_table_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->token_embedding_table_data, mi->token_embedding_table_size);
	printf("read:	%llx / %llx\n", ret, mi->token_embedding_table_size);
	if(DEBUG_WEIGHT){
		printu8(mi->token_embedding_table_data);
		printfloat(mi->token_embedding_table_data);
		printminmax(mi->token_embedding_table_data, mi->token_embedding_table_size);
	}
	offs = next;

	//rms_att_weight
	mi->rms_att_weight_size = mi->n_layers * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->rms_att_weight_data = malloc(mi->rms_att_weight_size);
	next += mi->rms_att_weight_size;
	printf("[%16llx,%16llx)@%p:rms_att_weight\n", offs, next, mi->rms_att_weight_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->rms_att_weight_data, mi->rms_att_weight_size);
	printf("read:	%llx / %llx\n", ret, mi->rms_att_weight_size);
	if(DEBUG_WEIGHT){
		printu8(mi->rms_att_weight_data);
		printfloat(mi->rms_att_weight_data);
		printminmax(mi->rms_att_weight_data, mi->rms_att_weight_size);
	}
	offs = next;

	//wq
	mi->wq_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->wq_data = malloc(mi->wq_size);
	next += mi->wq_size;
	printf("[%16llx,%16llx)@%p:wq\n", offs, next, mi->wq_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wq_data, mi->wq_size);
	printf("read:	%llx / %llx\n", ret, mi->wq_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wq_data);
		printfloat(mi->wq_data);
		printminmax(mi->wq_data, mi->wq_size);
	}
	offs = next;

	//wk
	mi->wk_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->wk_data = malloc(mi->wk_size);
	next += mi->wk_size;
	printf("[%16llx,%16llx)@%p:wk\n", offs, next, mi->wk_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wk_data, mi->wk_size);
	printf("read:	%llx / %llx\n", ret, mi->wk_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wk_data);
		printfloat(mi->wk_data);
		printminmax(mi->wk_data, mi->wk_size);
	}
	offs = next;

	//wv
	mi->wv_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->wv_data = malloc(mi->wv_size);
	next += mi->wv_size;
	printf("[%16llx,%16llx)@%p:wv\n", offs, next, mi->wv_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wv_data, mi->wv_size);
	printf("read:	%llx / %llx\n", ret, mi->wv_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wv_data);
		printfloat(mi->wv_data);
		printminmax(mi->wv_data, mi->wv_size);
	}
	offs = next;

	//wo
	mi->wo_size = (u64)mi->n_layers * mi->dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->wo_data = malloc(mi->wo_size);
	next += mi->wo_size;
	printf("[%16llx,%16llx)@%p:wo\n", offs, next, mi->wo_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->wo_data, mi->wo_size);
	printf("read:	%llx / %llx\n", ret, mi->wo_size);
	if(DEBUG_WEIGHT){
		printu8(mi->wo_data);
		printfloat(mi->wo_data);
		printminmax(mi->wo_data, mi->wo_size);
	}
	offs = next;

	//rms_ffn_weight
	mi->rms_ffn_weight_size = (u64)mi->n_layers * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->rms_ffn_weight_data = malloc(mi->rms_ffn_weight_size);
	next += mi->rms_ffn_weight_size;
	printf("[%16llx,%16llx)@%p:rms_ffn_weight\n", offs, next, mi->rms_ffn_weight_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->rms_ffn_weight_data, mi->rms_ffn_weight_size);
	printf("read:	%llx / %llx\n", ret, mi->rms_ffn_weight_size);
	if(DEBUG_WEIGHT){
		printu8(mi->rms_ffn_weight_data);
		printfloat(mi->rms_ffn_weight_data);
		printminmax(mi->rms_ffn_weight_data, mi->rms_ffn_weight_size);
	}
	offs = next;

	//w1
	mi->w1_size = (u64)mi->n_layers * mi->dim * mi->hidden_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->w1_data = malloc(mi->w1_size);
	next += mi->w1_size;
	printf("[%16llx,%16llx)@%p:w1\n", offs, next, mi->w1_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->w1_data, mi->w1_size);
	printf("read:	%llx / %llx\n", ret, mi->w1_size);
	if(DEBUG_WEIGHT){
		printu8(mi->w1_data);
		printfloat(mi->w1_data);
		printminmax(mi->w1_data, mi->w1_size);
	}
	offs = next;

	//w2
	mi->w2_size = (u64)mi->n_layers * mi->hidden_dim * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->w2_data = malloc(mi->w2_size);
	next += mi->w2_size;
	printf("[%16llx,%16llx)@%p:w2\n", offs, next, mi->w2_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->w2_data, mi->w2_size);
	printf("read:	%llx / %llx\n", ret, mi->w2_size);
	if(DEBUG_WEIGHT){
		printu8(mi->w2_data);
		printfloat(mi->w2_data);
		printminmax(mi->w2_data, mi->w2_size);
	}
	offs = next;

	//w3
	mi->w3_size = (u64)mi->n_layers * mi->dim * mi->hidden_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->w3_data = malloc(mi->w3_size);
	next += mi->w3_size;
	printf("[%16llx,%16llx)@%p:w3\n", offs, next, mi->w3_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->w3_data, mi->w3_size);
	printf("read:	%llx / %llx\n", ret, mi->w3_size);
	if(DEBUG_WEIGHT){
		printu8(mi->w3_data);
		printfloat(mi->w3_data);
		printminmax(mi->w3_data, mi->w3_size);
	}
	offs = next;

	//rms_final_weight
	mi->rms_final_weight_size = (u64)mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->rms_final_weight_data = malloc(mi->rms_final_weight_size);
	next += mi->rms_final_weight_size;
	printf("[%16llx,%16llx)@%p:rms_final_weight\n", offs, next, mi->rms_final_weight_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->rms_final_weight_data, mi->rms_final_weight_size);
	printf("read:	%llx / %llx\n", ret, mi->rms_final_weight_size);
	if(DEBUG_WEIGHT){
		printu8(mi->rms_final_weight_data);
		printfloat(mi->rms_final_weight_data);
		printminmax(mi->rms_final_weight_data, mi->rms_final_weight_size);
	}
	offs = next;

	//freq_cis_real
	int head_size = mi->dim / mi->n_heads;
	mi->freq_cis_real_size = (u64)mi->seq_len * head_size / 2 * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->freq_cis_real_data = malloc(mi->freq_cis_real_size);
	next += mi->freq_cis_real_size;
	printf("[%16llx,%16llx)@%p:freq_cis_real\n", offs, next, mi->freq_cis_real_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->freq_cis_real_data, mi->freq_cis_real_size);
	printf("read:	%llx / %llx\n", ret, mi->freq_cis_real_size);
	if(DEBUG_WEIGHT){
		printu8(mi->freq_cis_real_data);
		printfloat(mi->freq_cis_real_data);
		printminmax(mi->freq_cis_real_data, mi->freq_cis_real_size);
	}
	offs = next;

	//freq_cis_imag
	mi->freq_cis_imag_size = (u64)mi->seq_len * head_size / 2 * sizeof(MODELWEIGHT_FLOATTYPE);
	mi->freq_cis_imag_data = malloc(mi->freq_cis_imag_size);
	next += mi->freq_cis_imag_size;
	printf("[%16llx,%16llx)@%p:freq_cis_imag\n", offs, next, mi->freq_cis_imag_data);
	lseek64(fd, offs, SEEK_SET);
	ret = fullread(fd, mi->freq_cis_imag_data, mi->freq_cis_imag_size);
	printf("read:	%llx / %llx\n", ret, mi->freq_cis_imag_size);
	if(DEBUG_WEIGHT){
		printu8(mi->freq_cis_imag_data);
		printfloat(mi->freq_cis_imag_data);
		printminmax(mi->freq_cis_imag_data, mi->freq_cis_imag_size);
	}
	offs = next;

	if(shared_weights){
		mi->wcls_size = 0;
		mi->wcls_data = mi->token_embedding_table_data;
		printf("shared_weights@wcls\n");
	}
	else{
		mi->wcls_size = (u64)mi->vocab_size * mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
		mi->wcls_data = malloc(mi->wcls_size);
		next += mi->wcls_size;
		printf("[%16llx,%16llx)@%p:wcls\n", offs, next, mi->wcls_data);
		lseek64(fd, offs, SEEK_SET);
		ret = fullread(fd, mi->wcls_data, mi->wcls_size);
		printf("read:	%llx / %llx\n", ret, mi->wcls_size);
		if(DEBUG_WEIGHT){
			printu8(mi->wcls_data);
			printfloat(mi->wcls_data);
			printminmax(mi->wcls_data, mi->wcls_size);
		}
		offs = next;
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
		!mi->freq_cis_real_data||
		!mi->freq_cis_imag_data||
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


typedef struct {
	// activation at current time stamp (dim,)
	u64 x_size;
	MODELWEIGHT_FLOATTYPE *x_data;

	// same, but inside a residual branch (dim,)
	u64 xb_size;
	MODELWEIGHT_FLOATTYPE *xb_data;

	// an additional buffer just for convenience (dim,)
	u64 xb2_size;
	MODELWEIGHT_FLOATTYPE *xb2_data;

	// buffer for hidden dimension in the ffn (hidden_dim,)
	u64 hb_size;
	MODELWEIGHT_FLOATTYPE *hb_data;

	// buffer for hidden dimension in the ffn (hidden_dim,)
	u64 hb2_size;
	MODELWEIGHT_FLOATTYPE *hb2_data;

	// query (dim,)
	u64 q_size;
	MODELWEIGHT_FLOATTYPE *q_data;

	// key (dim,)
	u64 k_size;
	MODELWEIGHT_FLOATTYPE *k_data;

	// value (dim,)
	u64 v_size;
	MODELWEIGHT_FLOATTYPE *v_data;

	// buffer for scores/attention values (n_heads, seq_len)
	u64 att_size;
	MODELWEIGHT_FLOATTYPE *att_data;

	// output logits
	u64 logits_size;
	MODELWEIGHT_FLOATTYPE *logits_data;

	// kv cache// (layer, seq_len, dim)
	u64 key_cache_size;
	MODELWEIGHT_FLOATTYPE* key_cache_data;

	// (layer, seq_len, dim)
	u64 value_cache_size;
	MODELWEIGHT_FLOATTYPE* value_cache_data;
} RunState;
void llama_initstate(modelinfo* mi, RunState* rs) {
	u64 offs = 0;
	u64 next = 0;
	int kv_dim = (mi->dim * mi->n_kv_heads) / mi->n_heads;
	printf("--------state--------\n");

	rs->x_size = mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->x_data = malloc(rs->x_size);
	next = offs + rs->x_size;
	printf("[%16llx,%16llx)@%p:x\n", offs, next, rs->x_data);
	offs = next;

	rs->xb_size = mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->xb_data = malloc( rs->xb_size);
	next = offs + rs->xb_size;
	printf("[%16llx,%16llx)@%p:xb\n", offs, next, rs->xb_data);
	offs = next;

	rs->xb2_size = mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->xb2_data = malloc( rs->xb2_size);
	next = offs + rs->xb2_size;
	printf("[%16llx,%16llx)@%p:xb2\n", offs, next, rs->xb2_data);
	offs = next;

	rs->hb_size = mi->hidden_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->hb_data = malloc( rs->hb_size);
	next = offs + rs->hb_size;
	printf("[%16llx,%16llx)@%p:hb\n", offs, next, rs->hb_data);
	offs = next;

	rs->hb2_size = mi->hidden_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->hb2_data = malloc( rs->hb2_size);
	next = offs + rs->hb2_size;
	printf("[%16llx,%16llx)@%p:hb2\n", offs, next, rs->hb2_data);
	offs = next;

	rs->q_size = mi->dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->q_data = malloc( rs->q_size);
	next = offs + rs->q_size;
	printf("[%16llx,%16llx)@%p:q\n", offs, next, rs->q_data);
	offs = next;

	rs->k_size = kv_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->k_data = malloc( rs->k_size);
	next = offs + rs->k_size;
	printf("[%16llx,%16llx)@%p:k\n", offs, next, rs->k_data);
	offs = next;

	rs->v_size = kv_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->v_data = malloc( rs->v_size);
	next = offs + rs->v_size;
	printf("[%16llx,%16llx)@%p:v\n", offs, next, rs->v_data);
	offs = next;

	rs->att_size = mi->n_heads * mi->seq_len * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->att_data = malloc( rs->att_size);
	next = offs + rs->att_size;
	printf("[%16llx,%16llx)@%p:att\n", offs, next, rs->att_data);
	offs = next;

	rs->logits_size = mi->vocab_size * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->logits_data = malloc( rs->logits_size);
	next = offs + rs->logits_size;
	printf("[%16llx,%16llx)@%p:logits\n", offs, next, rs->logits_data);
	offs = next;

	rs->key_cache_size = mi->n_layers * mi->seq_len * kv_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->key_cache_data = malloc( rs->key_cache_size);
	next = offs + rs->key_cache_size;
	printf("[%16llx,%16llx)@%p:key_cache\n", offs, next, rs->key_cache_data);
	offs = next;

	rs->value_cache_size = mi->n_layers * mi->seq_len * kv_dim * sizeof(MODELWEIGHT_FLOATTYPE);
	rs->value_cache_data = malloc( rs->value_cache_size);
	next = offs + rs->value_cache_size;
	printf("[%16llx,%16llx)@%p:value_cache\n", offs, next, rs->value_cache_data);
	offs = next;

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


void rmsnorm(MODELWEIGHT_FLOATTYPE* o, MODELWEIGHT_FLOATTYPE* x, MODELWEIGHT_FLOATTYPE* weight, int size) {
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
		o[j] = (weight[j] * x[j]) * ss;
	}
}
void softmax(MODELWEIGHT_FLOATTYPE* x, int size) {
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
void matmul(MODELWEIGHT_FLOATTYPE* xout, MODELWEIGHT_FLOATTYPE* x, MODELWEIGHT_FLOATTYPE* w, int n, int d) {
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
}
void transformer(int token, int pos, modelinfo* mi, RunState* rs) {
	MODELWEIGHT_FLOATTYPE* w_token_embedding_table = mi->token_embedding_table_data;
	MODELWEIGHT_FLOATTYPE* w_rms_att_weight = mi->rms_att_weight_data;
	MODELWEIGHT_FLOATTYPE* w_rms_ffn_weight = mi->rms_ffn_weight_data;
	MODELWEIGHT_FLOATTYPE* w_wq = mi->wq_data;
	MODELWEIGHT_FLOATTYPE* w_wk = mi->wk_data;
	MODELWEIGHT_FLOATTYPE* w_wv = mi->wv_data;
	MODELWEIGHT_FLOATTYPE* w_wo = mi->wo_data;
	MODELWEIGHT_FLOATTYPE* w_w1 = mi->w1_data;
	MODELWEIGHT_FLOATTYPE* w_w2 = mi->w2_data;
	MODELWEIGHT_FLOATTYPE* w_w3 = mi->w3_data;
	MODELWEIGHT_FLOATTYPE* w_rms_final_weight = mi->rms_final_weight_data;
	MODELWEIGHT_FLOATTYPE* w_freq_cis_real = mi->freq_cis_real_data;
	MODELWEIGHT_FLOATTYPE* w_freq_cis_imag = mi->freq_cis_imag_data;
	MODELWEIGHT_FLOATTYPE* w_wcls = mi->wcls_data;

	MODELWEIGHT_FLOATTYPE* rs_x = rs->x_data;
	MODELWEIGHT_FLOATTYPE* rs_xb = rs->xb_data;
	MODELWEIGHT_FLOATTYPE* rs_xb2 = rs->xb2_data;
	MODELWEIGHT_FLOATTYPE* rs_hb = rs->hb_data;
	MODELWEIGHT_FLOATTYPE* rs_hb2 = rs->hb2_data;
	MODELWEIGHT_FLOATTYPE* rs_q = rs->q_data;
	MODELWEIGHT_FLOATTYPE* rs_k = rs->k_data;
	MODELWEIGHT_FLOATTYPE* rs_v = rs->v_data;
	MODELWEIGHT_FLOATTYPE* rs_att = rs->att_data;
	MODELWEIGHT_FLOATTYPE* rs_logits = rs->logits_data;
	MODELWEIGHT_FLOATTYPE* rs_key_cache = rs->key_cache_data;
	MODELWEIGHT_FLOATTYPE* rs_value_cache = rs->value_cache_data;

	// a few convenience variables
	MODELWEIGHT_FLOATTYPE *x = rs_x;
	int dim = mi->dim;
	int hidden_dim =  mi->hidden_dim;
	int head_size = dim / mi->n_heads;
	int kv_dim = (mi->dim * mi->n_kv_heads) / mi->n_heads;
	int kv_mul = mi->n_heads / mi->n_kv_heads;

	// copy the token embedding into x
	MODELWEIGHT_FLOATTYPE* content_row = &(w_token_embedding_table[token * dim]);
	memcpy(x, content_row, dim*sizeof(MODELWEIGHT_FLOATTYPE));

	// pluck out the "pos" row of freq_cis_real and freq_cis_imag
	MODELWEIGHT_FLOATTYPE* freq_cis_real_row = w_freq_cis_real + pos * head_size / 2;
	MODELWEIGHT_FLOATTYPE* freq_cis_imag_row = w_freq_cis_imag + pos * head_size / 2;

	// forward all the layers
	for(int l = 0; l < mi->n_layers; l++) {

		// attention rmsnorm
		rmsnorm(rs_xb, x, w_rms_att_weight + l*dim, dim);

		// qkv matmuls for this position
		matmul(rs_q, rs_xb, w_wq + l*dim*dim, dim, dim);
		matmul(rs_k, rs_xb, w_wk + l*dim*kv_dim, dim, kv_dim);
		matmul(rs_v, rs_xb, w_wv + l*dim*kv_dim, dim, kv_dim);

		// RoPE relative positional encoding: complex-valued rotate q and k by freq_cis in each head
		for (int v = 0; v < 2; v++) {
			MODELWEIGHT_FLOATTYPE* vec = (v == 0) ? rs_q : rs_k;    // the vector to rotate (query or key)
			int vec_size = (v == 0) ? dim  : kv_dim;  // the size of the vector
			for (int i = 0; i < vec_size; i+=2) {
				float v0 = vec[i];
				float v1 = vec[i+1];
				float fcr = freq_cis_real_row[(i % head_size) / 2];
				float fci = freq_cis_imag_row[(i % head_size) / 2];
				vec[i]   = v0 * fcr - v1 * fci;
				vec[i+1] = v0 * fci + v1 * fcr;
			}
		}

		// save key,value at this time step (pos) to our kv cache
		int loff = l * mi->seq_len * kv_dim; // kv cache layer offset for convenience
		MODELWEIGHT_FLOATTYPE* key_cache_row = rs_key_cache + loff + pos * kv_dim;
		MODELWEIGHT_FLOATTYPE* value_cache_row = rs_value_cache + loff + pos * kv_dim;
		memcpy(key_cache_row, rs_k, kv_dim*sizeof(*key_cache_row));
		memcpy(value_cache_row, rs_v, kv_dim*sizeof(*value_cache_row));
		
		// multihead attention. iterate over all heads
		int h;
		#pragma omp parallel for private(h)
		for (h = 0; h < mi->n_heads; h++) {
			// get the query vector for this head
			MODELWEIGHT_FLOATTYPE* q = rs_q + h * head_size;
			// attention scores for this head
			MODELWEIGHT_FLOATTYPE* att = rs_att + h * mi->seq_len;
			// iterate over all timesteps, including the current one
			for (int t = 0; t <= pos; t++) {
				// get the key vector for this head and at this timestep
				MODELWEIGHT_FLOATTYPE* k = rs_key_cache + loff + t * kv_dim + (h/kv_mul) * head_size;
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
			MODELWEIGHT_FLOATTYPE* xb = rs_xb + h * head_size;
			memset(xb, 0, head_size * sizeof(MODELWEIGHT_FLOATTYPE));
			for (int t = 0; t <= pos; t++) {
				// get the value vector for this head and at this timestep
				MODELWEIGHT_FLOATTYPE* v = rs_value_cache + loff + t * kv_dim + (h/kv_mul) * head_size;
				// get the attention weight for this timestep
				float a = att[t];
				// accumulate the weighted value into xb
				for (int i = 0; i < head_size; i++) {
					xb[i] += a * v[i];
				}
			}
		}

		// final matmul to get the output of the attention
		matmul(rs_xb2, rs_xb, w_wo + l*dim*dim, dim, dim);

		// residual connection back into x
		for (int i = 0; i < dim; i++) {
			x[i] += rs_xb2[i];
		}

		// ffn rmsnorm
		rmsnorm(rs_xb, x, w_rms_ffn_weight + l*dim, dim);

		// Now for FFN in PyTorch we have: self.w2(F.silu(self.w1(x)) * self.w3(x))
		// first calculate self.w1(x) and self.w3(x)
		matmul(rs_hb, rs_xb, w_w1 + l*dim*hidden_dim, dim, hidden_dim);
		matmul(rs_hb2, rs_xb, w_w3 + l*dim*hidden_dim, dim, hidden_dim);
		
		// F.silu; silu(x)=x*σ(x),where σ(x) is the logistic sigmoid
		for (int i = 0; i < hidden_dim; i++) {
			rs_hb[i] = rs_hb[i] * (1.0f / (1.0f + expf(-rs_hb[i])));
		}

		// elementwise multiply with w3(x)
		for (int i = 0; i < hidden_dim; i++) {
			rs_hb[i] = rs_hb[i] * rs_hb2[i];
		}

		// final matmul to get the output of the ffn
		matmul(rs_xb, rs_hb, w_w2 + l*dim*hidden_dim, hidden_dim, dim);

		// residual connection
		for (int i = 0; i < dim; i++) {
			x[i] += rs_xb[i];
		}
	}
	//printf("111\n");
	
	// final rmsnorm
	rmsnorm(x, x, w_rms_final_weight, dim);
	//printf("222\n");

	// classifier into logits
	//printf("%p,%p,%p,%d,%d\n",rs_logits, x, w_wcls, mi->dim, mi->vocab_size);
	//printf("wcls=%p,offs=%x\n",w_wcls, (void*)w_wcls-mi->wholefile);
	//matmul_debug=1;
	matmul(rs_logits, x, w_wcls, mi->dim, mi->vocab_size);
	//matmul_debug=0;
	//printf("333\n");
}
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
int sample(MODELWEIGHT_FLOATTYPE* probabilities, int n) {
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
int argmax(MODELWEIGHT_FLOATTYPE* v, int n) {
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
	MODELWEIGHT_FLOATTYPE* rs_logits = rs->logits_data;

	// start the main loop
	long start = 0;  // used to time our code, only initialized after first iteration
	int next;        // will store the next token in the sequence
	int token = 1;   // init with token 1 (=BOS), as done in Llama-2 sentencepiece tokenizer
	int pos = 0;     // position in the sequence
	//printf("<s>\n"); // explicit print the initial BOS token for stylistic symmetry reasons

	float temperature = 1.0;
	int steps = 4096;
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

		if(1 == next)break;

		// following BOS token (1), sentencepiece decoder strips any leading whitespace (see PR #89)
		char *token_str = (token == 1 && ti->vocab[next][0] == ' ') ? ti->vocab[next]+1 : ti->vocab[next];
		//printf("%s", token_str);
		if(0 == strncmp(token_str, "<0x0A>", 6))token_str = "\n";
		output(token_str, strlen(token_str));
		fflush(stdout);
		token = next;

		// init our timer here because the first iteration is slow due to memmap
		if (start == 0) { start = time_in_ms(); }
	}
	printf("\n\n");

	if (pos > 1) {
		long end = time_in_ms();
		printf("--------evaluate--------\n");
		printf("token=%d, time=%f, t/s=%f\n", pos-1, (end-start)*0.001, (pos-1) / (double)(end-start)*1000);
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

	//llama_exitprompt();
	//llama_exittokenizer();
	//llama_exitstate();
	//llama_exitmodel();
}
