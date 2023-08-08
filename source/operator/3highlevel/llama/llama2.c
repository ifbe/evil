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
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
#define DEBUG_WEIGHT 0
#define DEBUG_TOKEN 0
#define DEBUG_PROMPT 1


typedef struct {
	int dim; // transformer dimension
	int hidden_dim; // for ffn layers
	int n_layers; // number of layers
	int n_heads; // number of query heads
	int n_kv_heads; // number of key/value heads (can be < query heads because of multiquery)
	int vocab_size; // vocabulary size, usually 256 (byte-level)
	int seq_len; // max sequence length
} Config;
typedef struct {
	// token embedding table
	unsigned long long token_embedding_table;    // (vocab_size, dim)
	// weights for rmsnorms
	unsigned long long rms_att_weight; // (layer, dim) rmsnorm weights
	unsigned long long rms_ffn_weight; // (layer, dim)
	// weights for matmuls
	unsigned long long wq; // (layer, dim, dim)
	unsigned long long wk; // (layer, dim, dim)
	unsigned long long wv; // (layer, dim, dim)
	unsigned long long wo; // (layer, dim, dim)
	// weights for ffn
	unsigned long long w1; // (layer, hidden_dim, dim)
	unsigned long long w2; // (layer, dim, hidden_dim)
	unsigned long long w3; // (layer, hidden_dim, dim)
	// final rmsnorm
	unsigned long long rms_final_weight; // (dim,)
	// freq_cis for RoPE relatively positional embeddings
	unsigned long long freq_cis_real; // (seq_len, dim/2)
	unsigned long long freq_cis_imag; // (seq_len, dim/2)
	// (optional) classifier weights for the logits, on the last layer
	unsigned long long wcls;
} TransformerWeights;
typedef struct{
	Config cfg;
	TransformerWeights weight;
	void* wholefile;
}modelinfo;
void llama_initmodel(char* modelpath, modelinfo* modelinfo)
{
	int fd = open(modelpath, O_RDONLY|O_BINARY);
	if(fd <= 0){
		printf("errno=%d@open\n", errno);
		return;
	}

	Config* cfg = &modelinfo->cfg;
	if(0 == cfg){
		printf("errno=%d@malloc\n", errno);
		goto theend;
	}

	int ret = lseek(fd, 0, SEEK_SET);
	if(ret < 0){
		printf("errno=%d@lseek\n", errno);
		goto releasehead;
	}

	ret = read(fd, cfg, 0x1c);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto releasehead;
	}


	printf("--------header--------\n");
	printf("dim=%d\n", cfg->dim);
	printf("hidden_dim=%d\n", cfg->hidden_dim);
	printf("n_layers=%d\n", cfg->n_layers);
	printf("n_heads=%d\n", cfg->n_heads);
	printf("n_kv_heads=%d\n", cfg->n_kv_heads);
	printf("vocab_size=%d\n", cfg->vocab_size);
	printf("seq_len=%d\n", cfg->seq_len);
	int shared_weights = cfg->vocab_size > 0 ? 1 : 0;
	printf("shared_weights=%d\n",shared_weights);
	printf("\n");


	float body[0x200];
	TransformerWeights* weight = &modelinfo->weight;


	printf("--------weight--------\n");
	unsigned long long offs = 0x1c;
	weight->token_embedding_table = offs;
	printf("%llx@token_embedding_table\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->vocab_size * cfg->dim * 4;
	weight->rms_att_weight = offs;
	printf("%llx@rms_att_weight\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * 4;
	weight->wq = offs;
	printf("%llx@wq\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * cfg->dim * 4;
	weight->wk = offs;
	printf("%llx@wk\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * cfg->dim * 4;
	weight->wv = offs;
	printf("%llx@wv\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * cfg->dim * 4;
	weight->wo = offs;
	printf("%llx@wo\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * cfg->dim * 4;
	weight->rms_ffn_weight = offs;
	printf("%llx@rms_ffn_weight\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * 4;
	weight->w1 = offs;
	printf("%llx@w1\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * cfg->hidden_dim * 4;
	weight->w2 = offs;
	printf("%llx@w2\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->hidden_dim * cfg->dim * 4;
	weight->w3 = offs;
	printf("%llx@w3\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->n_layers * cfg->dim * cfg->hidden_dim * 4;
	weight->rms_final_weight = offs;
	printf("%llx@rms_final_weight\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->dim * 4;
	weight->freq_cis_real = offs;
	printf("%llx@freq_cis_real\n", offs);
	if(DEBUG_WEIGHT){
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	int head_size = cfg->dim / cfg->n_heads;
	offs += cfg->seq_len * head_size / 2 * 4;
	weight->freq_cis_imag = offs;
	if(DEBUG_WEIGHT){
		printf("%llx@freq_cis_imag\n", offs);
		lseek(fd, offs-16, SEEK_SET);
		read(fd, body, 0x20);
		printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
		printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
	}

	offs += cfg->seq_len * head_size / 2 * 4;
	if(shared_weights){
		weight->wcls = 0x1c;
		printf("1c@wcls\n");
	}
	else{
		weight->wcls = offs;
		printf("%llx@wcls\n", offs);
		if(DEBUG_WEIGHT){
			lseek(fd, offs-16, SEEK_SET);
			read(fd, body, 0x20);
			printf("-	%f,%f,%f,%f\n",body[0],body[1],body[2],body[3]);
			printf("+	%f,%f,%f,%f\n",body[4],body[5],body[6],body[7]);
		}
	}

	if(!shared_weights)offs += cfg->vocab_size * cfg->dim * 4;
	printf("%llx@total_file\n", offs);

	modelinfo->wholefile = malloc(offs);
	printf("malloc all: %p\n",modelinfo->wholefile);

	lseek(fd, 0, SEEK_SET);
	ret = read(fd, modelinfo->wholefile, offs);
	printf("reading all: %x/%llx\n\n", ret, offs);

releasebody:
	//free(body);

releasehead:
	//free(head);

theend:
	close(fd);
}


typedef struct {    // current wave of activations
	int x; // activation at current time stamp (dim,)
	int xb; // same, but inside a residual branch (dim,)
	int xb2; // an additional buffer just for convenience (dim,)
	int hb; // buffer for hidden dimension in the ffn (hidden_dim,)
	int hb2; // buffer for hidden dimension in the ffn (hidden_dim,)
	int q; // query (dim,)
	int k; // key (dim,)
	int v; // value (dim,)
	int att; // buffer for scores/attention values (n_heads, seq_len)
	int logits; // output logits
	// kv cache
	int key_cache;   // (layer, seq_len, dim)
	int value_cache; // (layer, seq_len, dim)

	void* wholememory;
} RunState;
void llama_initstate(modelinfo* mi, RunState* rs) {
	Config* cfg = &mi->cfg;
	int offs = 0;
	printf("--------state--------\n");

	rs->x = offs;
	printf("%x@x\n", offs);
	offs += cfg->dim * 4;

	rs->xb = offs;
	printf("%x@xb\n", offs);
	offs += cfg->dim * 4;

	rs->xb2 = offs;
	printf("%x@xb2\n", offs);
	offs += cfg->dim * 4;

	rs->hb = offs;
	printf("%x@hb\n", offs);
	offs += cfg->hidden_dim * 4;

	rs->hb2 = offs;
	printf("%x@hb2\n", offs);
	offs += cfg->hidden_dim * 4;

	rs->q = offs;
	printf("%x@q\n", offs);
	offs += cfg->dim * 4;

	rs->k = offs;
	printf("%x@k\n", offs);
	offs += cfg->dim * 4;

	rs->v = offs;
	printf("%x@v\n", offs);
	offs += cfg->dim * 4;

	rs->att = offs;
	printf("%x@att\n", offs);
	offs += cfg->n_heads * cfg->seq_len * 4;

	rs->logits = offs;
	printf("%x@logits\n", offs);
	offs += cfg->vocab_size * 4;

	rs->key_cache = offs;
	printf("%x@key_cache\n", offs);
	offs += cfg->n_layers * cfg->seq_len * cfg->dim * 4;

	rs->value_cache = offs;
	printf("%x@value_cache\n", offs);
	offs += cfg->n_layers * cfg->seq_len * cfg->dim * 4;

	printf("%x@toatl_size\n\n", offs);
	rs->wholememory = malloc(offs);
}


typedef struct{
	unsigned int max_token_length;
	float* vocab_scores;
	char** vocab;
}tokeninfo;
void llama_inittokenizer(char* tokenpath, modelinfo* mi, tokeninfo* ti)
{
	Config* cfg = &mi->cfg;
	printf("--------inittokenizer--------\n");

	FILE *file = fopen(tokenpath, "rb");
	if (!file) { printf("couldn't load tokenizer.bin\n"); return; }

	unsigned int max_token_length;
	if (fread(&max_token_length, sizeof(int), 1, file) != 1) { printf("failed read\n"); return; }
	printf("max_token_length=%d\n",max_token_length);
	ti->max_token_length = max_token_length;

	float* vocab_scores = (float*)malloc(cfg->vocab_size * sizeof(float));
	ti->vocab_scores = vocab_scores;

	char** vocab = (char**)malloc(cfg->vocab_size * sizeof(char*));
	ti->vocab = vocab;

	int len;
	for (int i = 0; i < cfg->vocab_size; i++) {
		if (fread(vocab_scores + i, sizeof(float), 1, file) != 1) { printf("failed read\n"); return;}
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

	Config* cfg = &mi->cfg;
	ts->prompt_tokens = (int*)malloc(cfg->seq_len * sizeof(int));
	ts->num_prompt_tokens = 0;
	printf("size=%d,addr=%p\n", ts->num_prompt_tokens, ts->prompt_tokens);

	printf("\n");
}


long time_in_ms() {
	// return time in milliseconds, for benchmarking the model speed
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_sec * 1000 + time.tv_nsec / 1000000;
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
void bpe_encode(char *text, char **vocab, float *vocab_scores, int vocab_size, unsigned int max_token_length, int *tokens, int *n_tokens) {
	
	// a temporary buffer to merge two consecutive tokens
	char* str_buffer = malloc((max_token_length*2+1) * sizeof(char)); // *2 for concat, +1 for null terminator

	// first encode every individual byte in the input string
	*n_tokens = 0; // the number of tokens
	for (char *c = text; *c != '\0'; c++) {
		sprintf(str_buffer, "%c", *c);
		int id = str_lookup(str_buffer, vocab, vocab_size);
		if (id == -1) { printf("not good\n"); exit(1);}
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
			sprintf(str_buffer, "%s%s", vocab[tokens[i]], vocab[tokens[i+1]]);
			int id = str_lookup(str_buffer, vocab, vocab_size);
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
}
void llama_prompt(modelinfo* mi, tokeninfo* tk, TokenState* ts, char* prompt)
{
	printf("--------prompt--------\n");
	if(NULL == prompt){
		goto theend;
	}

	Config* cfg = &mi->cfg;
	bpe_encode(prompt, tk->vocab, tk->vocab_scores, cfg->vocab_size, tk->max_token_length, ts->prompt_tokens, &ts->num_prompt_tokens);
	for(int j=0;j<ts->num_prompt_tokens;j++){
		if(DEBUG_PROMPT){
			int t = ts->prompt_tokens[j];
			printf("%d: token=%d, string=%s\n", j, t, tk->vocab[t]);
		}
	}

theend:
	printf("\n");
}


void accum(float *a, float *b, int size) {
	for (int i = 0; i < size; i++) {
		a[i] += b[i];
	}
}
void rmsnorm(float* o, float* x, float* weight, int size) {
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
		o[j] = weight[j] * (ss * x[j]);
	}
}
void softmax(float* x, int size) {
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
void matmul(float* xout, float* x, float* w, int n, int d) {
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
	Config* cfg = &mi->cfg;
	TransformerWeights* w = &mi->weight;

	float* w_token_embedding_table = mi->wholefile + w->token_embedding_table;
	float* w_rms_att_weight = mi->wholefile + w->rms_att_weight;
	float* w_rms_ffn_weight = mi->wholefile + w->rms_ffn_weight;
	float* w_wq = mi->wholefile + w->wq;
	float* w_wk = mi->wholefile + w->wk;
	float* w_wv = mi->wholefile + w->wv;
	float* w_wo = mi->wholefile + w->wo;
	float* w_w1 = mi->wholefile + w->w1;
	float* w_w2 = mi->wholefile + w->w2;
	float* w_w3 = mi->wholefile + w->w3;
	float* w_rms_final_weight = mi->wholefile + w->rms_final_weight;
	float* w_freq_cis_real = mi->wholefile + w->freq_cis_real;
	float* w_freq_cis_imag = mi->wholefile + w->freq_cis_imag;
	float* w_wcls = mi->wholefile + w->wcls;

	float* rs_x = rs->wholememory + rs->x;
	float* rs_xb = rs->wholememory + rs->xb;
	float* rs_xb2 = rs->wholememory + rs->xb2;
	float* rs_hb = rs->wholememory + rs->hb;
	float* rs_hb2 = rs->wholememory + rs->hb2;
	float* rs_q = rs->wholememory + rs->q;
	float* rs_k = rs->wholememory + rs->k;
	float* rs_v = rs->wholememory + rs->v;
	float* rs_att = rs->wholememory + rs->att;
	float* rs_logits = rs->wholememory + rs->logits;
	float* rs_key_cache = rs->wholememory + rs->key_cache;
	float* rs_value_cache = rs->wholememory + rs->value_cache;

	// a few convenience variables
	float *x = rs_x;
	int dim = cfg->dim;
	int hidden_dim =  cfg->hidden_dim;
	int head_size = dim / cfg->n_heads;

	// copy the token embedding into x
	float* content_row = &(w_token_embedding_table[token * dim]);
	memcpy(x, content_row, dim*sizeof(*x));

	// pluck out the "pos" row of freq_cis_real and freq_cis_imag
	float* freq_cis_real_row = w_freq_cis_real + pos * head_size / 2;
	float* freq_cis_imag_row = w_freq_cis_imag + pos * head_size / 2;

	// forward all the layers
	for(int l = 0; l < cfg->n_layers; l++) {

		// attention rmsnorm
		rmsnorm(rs_xb, x, w_rms_att_weight + l*dim, dim);

		// qkv matmuls for this position
		matmul(rs_q, rs_xb, w_wq + l*dim*dim, dim, dim);
		matmul(rs_k, rs_xb, w_wk + l*dim*dim, dim, dim);
		matmul(rs_v, rs_xb, w_wv + l*dim*dim, dim, dim);

		// apply RoPE rotation to the q and k vectors for each head
		for (int h = 0; h < cfg->n_heads; h++) {
			// get the q and k vectors for this head
			float* q = rs_q + h * head_size;
			float* k = rs_k + h * head_size;
			// rotate q and k by the freq_cis_real and freq_cis_imag
			for (int i = 0; i < head_size; i+=2) {
				float q0 = q[i];
				float q1 = q[i+1];
				float k0 = k[i];
				float k1 = k[i+1];
				float fcr = freq_cis_real_row[i/2];
				float fci = freq_cis_imag_row[i/2];
				q[i]   = q0 * fcr - q1 * fci;
				q[i+1] = q0 * fci + q1 * fcr;
				k[i]   = k0 * fcr - k1 * fci;
				k[i+1] = k0 * fci + k1 * fcr;
			}
		}

		// save key,value at this time step (pos) to our kv cache
		int loff = l * cfg->seq_len * dim; // kv cache layer offset for convenience
		float* key_cache_row = rs_key_cache + loff + pos * dim;
		float* value_cache_row = rs_value_cache + loff + pos * dim;
		memcpy(key_cache_row, rs_k, dim*sizeof(*key_cache_row));
		memcpy(value_cache_row, rs_v, dim*sizeof(*value_cache_row));
		
		// multihead attention. iterate over all heads
		int h;
		#pragma omp parallel for private(h)
		for (h = 0; h < cfg->n_heads; h++) {
			// get the query vector for this head
			float* q = rs_q + h * head_size;
			// attention scores for this head
			float* att = rs_att + h * cfg->seq_len;
			// iterate over all timesteps, including the current one
			for (int t = 0; t <= pos; t++) {
				// get the key vector for this head and at this timestep
				float* k = rs_key_cache + loff + t * dim + h * head_size;
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
			float* xb = rs_xb + h * head_size;
			memset(xb, 0, head_size * sizeof(float));
			for (int t = 0; t <= pos; t++) {
				// get the value vector for this head and at this timestep
				float* v = rs_value_cache + loff + t * dim + h * head_size;
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
		accum(x, rs_xb2, dim);

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
		accum(x, rs_xb, dim);
	}
	//printf("111\n");
	
	// final rmsnorm
	rmsnorm(x, x, w_rms_final_weight, dim);
	//printf("222\n");

	// classifier into logits
	//printf("%p,%p,%p,%d,%d\n",rs_logits, x, w_wcls, cfg->dim, cfg->vocab_size);
	//printf("wcls=%p,offs=%x\n",w_wcls, (void*)w_wcls-mi->wholefile);
	//matmul_debug=1;
	matmul(rs_logits, x, w_wcls, cfg->dim, cfg->vocab_size);
	//matmul_debug=0;
	//printf("333\n");
}
unsigned long long rng_seed;
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
int sample(float* probabilities, int n) {
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
int argmax(float* v, int n) {
	// return argmax of v in elements 0..n
	int max_i = 0;
	float max_p = v[0];
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
	Config* cfg = &mi->cfg;
	float* rs_logits = rs->wholememory + rs->logits;

	// start the main loop
	long start = 0;  // used to time our code, only initialized after first iteration
	int next;        // will store the next token in the sequence
	int token = 1;   // init with token 1 (=BOS), as done in Llama-2 sentencepiece tokenizer
	int pos = 0;     // position in the sequence
	//printf("<s>\n"); // explicit print the initial BOS token for stylistic symmetry reasons

	float temperature = 0.0;
	int steps = 256;
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
				next = argmax(rs_logits, cfg->vocab_size);
			} else {
				// apply the temperature to the logits
				for (int q=0; q<cfg->vocab_size; q++) { rs_logits[q] /= temperature; }
				// apply softmax to the logits to get the probabilities for next token
				softmax(rs_logits, cfg->vocab_size);
				// we sample from this distribution to get the next token
				next = sample(rs_logits, cfg->vocab_size);
			}
		}

		// following BOS token (1), sentencepiece decoder strips any leading whitespace (see PR #89)
		char *token_str = (token == 1 && ti->vocab[next][0] == ' ') ? ti->vocab[next]+1 : ti->vocab[next];
		printf("%s", token_str);
		fflush(stdout);

		// advance forward
		token = next;
		pos++;
		// init our timer here because the first iteration is slow due to memmap
		if (start == 0) { start = time_in_ms(); }
	}
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

	char str[256];
	do{
		gets(str);
		llama_prompt(&model, &token, &tokenstate, str);
		llama_runmodel(&model, &modelstate, &token, &tokenstate);
		printf("\n");
	}while(1);

	//llama_exitprompt();
	//llama_exittokenizer();
	//llama_exitstate();
	//llama_exitmodel();
}
