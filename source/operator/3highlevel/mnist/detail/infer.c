#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef unsigned char u8;


struct mnist_runstate{
	char* modelpath;
	float* modelmem;

	char* imagepath;
	char* labelpath;
};
int mnist_init(int argc, char** argv, struct mnist_runstate* rs);
int mnist_exit(struct mnist_runstate* rs);
void* mnist_image_get(int id);
void* mnist_label_get(int id);



int ann_forward(u8* image, int label, float* weight, float* result)
{
	int x,y;
	float* bias = &weight[28*28*10];
	for(y=0;y<10;y++){
		//y = b
		result[y] = bias[y];
		//y += w*x
		for(x=0;x<28*28;x++){
			result[y] += weight[28*28*y + x] * image[x];
		}
		//normalize
		result[y] /= 28*28*255;
		//ReLU
		if(result[y] < 0.0)result[y] = 0.0;
	}

	return 0;
}


int mnist_infer_one(struct mnist_runstate* rs, int id)
{
	u8* image = mnist_image_get(id);
	if(0 == image)return -1;
	u8* label = mnist_label_get(id);

	//printf("%s: image=%p\n", __func__, image);
	//printf("%s: label=%p\n", __func__, label);
	int x,y;
	for(y=0;y<28;y++){
		for(x=0;x<28;x++){
			printf("%d%c", image[y*28+x]/26, (x+1==28) ? '\n' : ' ');
		}
	}
	printf("label=%d\n", label[0]);

	float result[10];
	ann_forward(image, label[0], rs->modelmem, result);

	int j;
	int maxidx = -1;
	float maxval = 1e-9;
	for(j=0;j<10;j++){
		printf("%f%c", result[j], (j<9) ? ' ' : '\n');
		if(maxval < result[j]){
			maxval = result[j];
			maxidx = j;
		}
	}
	if(label[0]==maxidx){
		printf("label=%d, maxidx=%d, good\n", label[0], maxidx);
	}
	else{
		printf("label=%d, maxidx=%d, wrong\n", label[0], maxidx);
	}

	return 0;
}


int mnist_infer(int argc, char** argv)
{
	struct mnist_runstate rs;
	mnist_init(argc,argv, &rs);

	int id = 0;
	int retkey;
	int rettrain;
	while(1){
		retkey = scanf("%d", &id);
		if(retkey <= 0)break;

		printf("id=%d\n",id);
		rettrain = mnist_infer_one(&rs, id);
		if(rettrain < 0)break;
	}

	mnist_exit(&rs);
}