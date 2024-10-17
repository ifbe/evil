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
int mnist_model_save(char* path);

/*
L = (result-real)*(result-real) = result*result - 2*result*real + real*real
result = w * x + b
Ltoresult = 2*result-2*real
resulttow = x
resulttob = 1
Ltow = Ltoresult * resulttow
Ltob = Ltoresult
*/
int ann_forward(u8* image, int label, float* weight, float* result);
int ann_backward(u8* image, int label, float* weight, float* result)
{
	int x,y;
	float real, L_to_R, R_to_W, R_to_B, L_to_W, L_to_B;
	float* bias = &weight[28*28*10];
	for(y=0;y<10;y++){
		real = (y==label) ? 1.0 : 0.0;
		L_to_R = result[y] - real;

		//update weight
		for(x=0;x<28*28;x++){
			L_to_W = L_to_R * (image[x]/255.0);
			weight[28*28*y + x] += -0.1 * L_to_W;
		}

		//update bias
		L_to_B = L_to_R;
		bias[y] += -0.1 * L_to_B;
	}
	return 0;
}


int countgood = 0;
int countwrong = 0;
int mnist_train_one(struct mnist_runstate* rs, int id)
{
	u8* image = mnist_image_get(id);
	if(0 == image)return -1;
	u8* label = mnist_label_get(id);

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
		countgood++;
	}
	else{
		printf("label=%d, maxidx=%d, wrong\n", label[0], maxidx);
		countwrong ++;
	}

	ann_backward(image, label[0], rs->modelmem, result);
	return 0;
}
int mnist_train(int argc, char** argv)
{
	struct mnist_runstate rs;
	mnist_init(argc,argv, &rs);

	int id = 0;
	int rettrain;
	while(1){
		printf("id=%d\n",id);

		rettrain = mnist_train_one(&rs, id);
		if(rettrain < 0)break;

		id++;
	}
	printf("countgood=%d,countwrong=%d\n", countgood, countwrong);

	mnist_model_save(rs.modelpath);

	mnist_exit(&rs);
	return 0;
}