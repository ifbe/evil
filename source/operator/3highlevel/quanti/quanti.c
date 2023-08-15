#include <stdio.h>
#include <stdlib.h>
#define MODELWEIGHT_HEADSIZE 0x1c
#define MODELWEIGHT_FLOATTYPE float


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
	for(j=0;j<cnt1;j++){
		bf16[j] = fp32[2*j+1];
	}
}
void quantization(int argc, char** argv)
{
	if(argc < 3){
		printf("./a.out fp32tobf16 in.bin out.bin\n");
		return;
	}

	FILE *fi = fopen(argv[1], "rb");
	FILE *fo = fopen(argv[2], "wb");
	void* ptr = malloc(0x1000000);		//16MB

	fread(ptr, 0x1c, 1, fi);
	fwrite(ptr,0x1c, 1, fo);

	int ret = 0;
	int wlen = 0;
	while(1){
		ret = fread(ptr, 1, 0x1000000, fi);
		printf("read: ret=%x\n",ret);
		if(ret <= 0)break;

		if(1){
			fp32tobf16(ptr, ret/4, ptr, 0);
			wlen = 0x1000000/2;
		}

		ret = fwrite(ptr, 1, wlen, fo);
		printf("write:ret=%x\n",ret);
		if(ret <= 0)break;
	}

	fclose(fo);
	fclose(fi);
}
