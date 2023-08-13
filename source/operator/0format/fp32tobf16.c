#include <stdio.h>
#include <stdlib.h>


void fp32tobf16(int argc, char** argv)
{
	if(argc < 3){
		printf("./a.out fp32tobf16 in.bin out.bin\n");
		return;
	}

	FILE *fi = fopen(argv[1], "rb");
	FILE *fo = fopen(argv[2], "wb");
	unsigned short* ptr = malloc(0x1000000);		//16MB

	fread(ptr, 0x1c, 1, fi);
	fwrite(ptr,0x1c, 1, fo);

	int j;
	int ret = 0;
	while(1){
		ret = fread(ptr, 1, 0x100000, fi);
		printf("read: ret=%x\n",ret);
		if(ret <= 0)break;

		for(j=0;j<ret/4;j++){
			ptr[j] = ptr[j*2+1];
		}

		ret = fwrite(ptr, 1, ret/2, fo);
		printf("write:ret=%x\n",ret);
		if(ret <= 0)break;
	}

	fclose(fo);
	fclose(fi);
}