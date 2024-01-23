#include <stdio.h>
#include <stdlib.h>
#define MODELWEIGHT_HEADSIZE 0x1c
#define MODELWEIGHT_FLOATTYPE float

typedef struct{
	int dim; // transformer dimension
	int hidden_dim; // for ffn layers
	int n_layers; // number of layers
	int n_heads; // number of query heads
	int n_kv_heads; // number of key/value heads (can be < query heads because of multiquery)
	int vocab_size; // vocabulary size, usually 256 (byte-level)
	int seq_len;

}modelinfo;
/*
void fp32tofp16(float* fp32, int cnt1, _Bfloat16* fp16, int cnt2)
{
	int j;
	for(j=0;j<cnt1;j++){
		fp16[j] = fp32[j];
	}
}
*/
void fp32tobf16(unsigned short* fp32, int cnt1, unsigned short* bf16, int cnt2)
{
	int j;
	float min = 1000*1000*1000;
	float max =-1000*1000*1000;
	float* ptr = (void*)fp32;
	for(j=0;j<cnt1;j++){
		bf16[j] = fp32[2*j+1];
		if(min>ptr[j])min = ptr[j];
		if(max<ptr[j])max = ptr[j];
	}
	printf("min=%f,max=%f\n", min, max);
}
void calc_1(FILE* fi, FILE* fo, int cntone, int cntall)
{
	void* ptr = malloc(cntall*sizeof(float));		//16MB
	if(0 == ptr){
		printf("malloc fail!\n");
		exit(-1);
	}

	int ret = 0;
	int cnt = 0;
	while(1){
		ret = fread(ptr, 1, cntall*sizeof(float), fi);
		printf("read: ret=%x\n",ret);
		if(ret <= 0)break;

		cnt += cntall;
		if(cnt >= cntall)break;
	}

	for(cnt=0;cnt<cntall;cnt+=cntone){
		printf("%04x: ", cnt);
		fp32tobf16(ptr+cnt*sizeof(float), cntone, ptr+cnt*sizeof(short), 0);
	}

	cnt = 0;
	while(1){
		ret = fwrite(ptr, 1, cntall*sizeof(short), fo);
		printf("write:ret=%x\n",ret);
		if(ret <= 0)break;

		cnt += cntall;
		if(cnt >= cntall)break;
	}
	free(ptr);
}
void llama_quanti(int argc, char** argv)
{
	if(argc < 3){
		printf("./a.out fp32tobf16 in.bin out.bin\n");
		return;
	}

	FILE *fi = fopen(argv[1], "rb");
	FILE *fo = fopen(argv[2], "wb");

	modelinfo mi;
	fread(&mi, 0x1c, 1, fi);
	fwrite(&mi,0x1c, 1, fo);

	int shared_weights = mi.vocab_size > 0 ? 1 : 0;
	if(mi.vocab_size < 0)mi.vocab_size = -mi.vocab_size;

	printf("dim=%d\n", mi.dim);
	printf("hidden_dim=%d\n", mi.hidden_dim);
	printf("n_layers=%d\n", mi.n_layers);
	printf("n_heads=%d\n", mi.n_heads);
	printf("n_kv_heads=%d\n", mi.n_kv_heads);
	printf("vocab_size=%d\n", mi.vocab_size);
	printf("seq_len=%d\n", mi.seq_len);
	printf("shared_weights=%d\n",shared_weights);

	//token_embedding_table
	calc_1(fi, fo, mi.dim, mi.vocab_size * mi.dim);

	//rms_att_weight
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.dim);

	//wq
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.dim * mi.dim);

	//wk
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.dim * mi.dim);

	//wv
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.dim * mi.dim);

	//wo
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.dim * mi.dim);

	//rms_ffn_weight
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.dim);

	//w1
	calc_1(fi, fo, mi.hidden_dim, mi.n_layers * mi.dim * mi.hidden_dim);

	//w2
	calc_1(fi, fo, mi.dim, mi.n_layers * mi.hidden_dim * mi.dim);

	//w3
	calc_1(fi, fo, mi.hidden_dim, mi.n_layers * mi.dim * mi.hidden_dim);

	//rms_final_weight
	calc_1(fi, fo, mi.dim, mi.dim);

	//freq_cis_real
	int head_size = mi.dim / mi.n_heads;
	calc_1(fi, fo, head_size / 2, mi.seq_len * head_size / 2);

	//freq_cis_imag
	calc_1(fi, fo, head_size / 2, mi.seq_len * head_size / 2);

	if(!shared_weights){
		calc_1(fi, fo, mi.dim, mi.vocab_size * mi.dim);
	}

	fclose(fo);
	fclose(fi);
}
