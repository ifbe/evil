#include <stdio.h>
#include <stdlib.h>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define DEBUG_MINMAX 0

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
static u64 g_pos = 0;


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
	if(DEBUG_MINMAX)printf("min=%f,max=%f\n", min, max);
}
void calc_1(FILE* fi, FILE* fo, u64 cx, u64 cy)
{
	u64 cntx = cx;
	u64 cnty = cy;
	while(1){
		if(cnty&1)break;
		if(cntx>=256*0x100000)break;		//256M*4B
		cntx <<= 1;
		cnty >>= 1;
	}

	u64 sz = cntx*sizeof(float);
	u8* ptr = malloc(sz);
	if(0 == ptr){
		printf("malloc fail!\n");
		exit(-1);
	}

	int kk;
	int ret = 0;
	u64 cnt = 0;
	while(1){
		ret = fread(ptr, 1, sz, fi);
		printf("read:at=%llx,sz=%llx,ret=%x\n", g_pos, sz, ret);
		if(sz != ret)break;
		g_pos += sz;

		for(kk=0;kk<cntx;kk+=cx){
			if(DEBUG_MINMAX)printf("convert %llx: ", 4 * (cntx * cnt + kk));
			fp32tobf16((void*)(ptr + 4*kk), cx, (void*)(ptr + 2*kk), 0);
		}

		ret = fwrite(ptr, 1, cntx*sizeof(short), fo);
		printf("write:ret=%x\n",ret);
		if(ret <= 0)break;

		printf("\n");
		cnt += 1;
		if(cnt >= cnty)break;
	}

	free(ptr);

	if(cnt < cnty){
		printf("unexpected break\n");
		exit(-1);
	}
}
void llama_quanti(int argc, char** argv)
{
	if(argc < 3){
		printf("./a.out fp32tobf16 in.bin out.bin\n");
		return;
	}

	FILE *fi = fopen(argv[1], "rb");

	modelinfo mi;
	fread(&mi, 0x1c, 1, fi);
	g_pos = 0x1c;

	int shared_weights = mi.vocab_size > 0 ? 1 : 0;
	int vocab_size = mi.vocab_size > 0 ? mi.vocab_size : -mi.vocab_size;

	printf("dim=%d\n", mi.dim);
	printf("hidden_dim=%d\n", mi.hidden_dim);
	printf("n_layers=%d\n", mi.n_layers);
	printf("n_heads=%d\n", mi.n_heads);
	printf("n_kv_heads=%d\n", mi.n_kv_heads);
	printf("vocab_size=%d\n", vocab_size);
	printf("seq_len=%d\n", mi.seq_len);
	printf("shared_weights=%d\n",shared_weights);
	printf("\n");

	FILE *fo = fopen(argv[2], "wb");
	fwrite(&mi,0x1c, 1, fo);

	//token_embedding_table
	calc_1(fi, fo, mi.dim, vocab_size);

	//rms_att_weight
	calc_1(fi, fo, mi.dim, mi.n_layers);

	//wq
	calc_1(fi, fo, mi.dim, mi.dim * mi.n_layers);

	int kvdim = (mi.dim * mi.n_kv_heads) / mi.n_heads;

	//wk
	calc_1(fi, fo, mi.dim, kvdim * mi.n_layers);

	//wv
	calc_1(fi, fo, mi.dim, kvdim * mi.n_layers);

	//wo
	calc_1(fi, fo, mi.dim, mi.dim * mi.n_layers);

	//rms_ffn_weight
	calc_1(fi, fo, mi.dim, mi.n_layers);

	//w1
	calc_1(fi, fo, mi.hidden_dim, mi.dim * mi.n_layers);

	//w2
	calc_1(fi, fo, mi.dim, mi.hidden_dim * mi.n_layers);

	//w3
	calc_1(fi, fo, mi.hidden_dim, mi.dim * mi.n_layers);

	//rms_final_weight
	calc_1(fi, fo, mi.dim, 1);

	//freq_cis_real
	int head_size = mi.dim / mi.n_heads;
	calc_1(fi, fo, head_size / 2, mi.seq_len);

	//freq_cis_imag
	calc_1(fi, fo, head_size / 2, mi.seq_len);

	if(!shared_weights){
		calc_1(fi, fo, mi.dim, vocab_size);
	}

	fclose(fo);
	fclose(fi);
}

void llama_info(int argc, char** argv)
{
	if(argc < 2){
		printf("./a.out fp32tobf16 in.bin\n");
		return;
	}

	modelinfo mi;
	FILE *fi = fopen(argv[1], "rb");
	fread(&mi, 0x1c, 1, fi);
#if defined _WIN32
	_fseeki64(fi, 0, SEEK_END);
	u64 filesize = _ftelli64(fi);
#else
	fseek(fi, 0, SEEK_END);
	u64 filesize = ftell(fi);
#endif
	printf("filesize=%lld(%llx)\n",filesize,filesize);
	fclose(fi);

	int shared_weights = mi.vocab_size > 0 ? 1 : 0;
	int vocab_size = mi.vocab_size > 0 ? mi.vocab_size : -mi.vocab_size;

	printf("dim=%d\n", mi.dim);
	printf("hidden_dim=%d\n", mi.hidden_dim);
	printf("n_layers=%d\n", mi.n_layers);
	printf("n_heads=%d\n", mi.n_heads);
	printf("n_kv_heads=%d\n", mi.n_kv_heads);
	printf("vocab_size=%d\n", vocab_size);
	printf("seq_len=%d\n", mi.seq_len);
	printf("shared_weights=%d\n",shared_weights);

	u64 tmp = 0;
	u64 pos = 0x1c;

	//token_embedding_table
	tmp = 4 * (u64)mi.dim * vocab_size;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//rms_att_weight
	tmp = 4 * (u64)mi.dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//wq
	tmp = 4 * (u64)mi.dim * mi.dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	int kvdim = (mi.dim * mi.n_kv_heads) / mi.n_heads;

	//wk
	tmp = 4 * (u64)mi.dim * kvdim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//wv
	tmp = 4 * (u64)mi.dim * kvdim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//wo
	tmp = 4 * (u64)mi.dim * mi.dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//rms_ffn_weight
	tmp = 4 * (u64)mi.dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//w1
	tmp = 4 * (u64)mi.hidden_dim * mi.dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//w2
	tmp = 4 * (u64)mi.dim * mi.hidden_dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//w3
	tmp = 4 * (u64)mi.hidden_dim * mi.dim * mi.n_layers;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//rms_final_weight
	tmp = 4 * (u64)mi.dim;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//freq_cis_real
	int head_size = mi.dim / mi.n_heads;
	tmp = 4 * (u64)(head_size / 2) * mi.seq_len;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	//freq_cis_imag
	tmp = 4 * (u64)(head_size / 2) * mi.seq_len;
	printf("[%llx,%llx]\n", pos, tmp);
	pos += tmp;

	if(!shared_weights){
		tmp = 4 * (u64)mi.dim * vocab_size;
		printf("[%llx,%llx]\n", pos, tmp);
		pos += tmp;
	}

	printf("size=%lld(%llx)\n", pos, pos);
}